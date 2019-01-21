#pragma once
#include <cmath>
#include <algorithm>
#include <array>
#include "../HLSDK/Vec3.hpp"

namespace math
{
    typedef vec3_s vec3;

    struct bbox
    {
        vec3 bbmin;
        vec3 bbmax;
    };

    struct ray_result
    {
        bool hit;
        float distance;
    };

    class matrix3x4
    {
        private:
            using mtrx = float[3][4];

        public:
            std::array<std::array<float, 4>, 3> matrix;

            matrix3x4()
            {

            }

            matrix3x4(mtrx p)
            {
                for (size_t i = 0; i < 3; i++)
                {
                    for (size_t o = 0; o < 4; o++)
                    {
                        this->matrix[i][o] = p[i][o];
                    }
                }
            }

            std::array<float, 4>& operator[](int index)
            {
                return this->matrix[index];
            }

            vec3 transform_vec3(const vec3& vec) const
            {
                vec3 result = {};
                vec3 prod1 = vec3(this->matrix[0][0], this->matrix[0][1], this->matrix[0][2]);
                vec3 prod2 = vec3(this->matrix[1][0], this->matrix[1][1], this->matrix[1][2]);
                vec3 prod3 = vec3(this->matrix[2][0], this->matrix[2][1], this->matrix[2][2]);

                result.x = vec.dot_product(prod1) + this->matrix[0][3];
                result.y = vec.dot_product(prod2) + this->matrix[1][3];
                result.z = vec.dot_product(prod3) + this->matrix[2][3];

                return result;
            }

            vec3 rotate_vec3(const vec3& vec) const
            {
                vec3 result = {};
                vec3 prod1 = vec3(this->matrix[0][0], this->matrix[0][1], this->matrix[0][2]);
                vec3 prod2 = vec3(this->matrix[1][0], this->matrix[1][1], this->matrix[1][2]);
                vec3 prod3 = vec3(this->matrix[2][0], this->matrix[2][1], this->matrix[2][2]);

                result.x = vec.dot_product(prod1);
                result.y = vec.dot_product(prod2);
                result.z = vec.dot_product(prod3);

                return result;
            }

            vec3 inverse_transform_vec3(const vec3& vec) const
            {
                vec3 result = {};
                vec3 in1t = {};

                in1t.x = vec.x - this->matrix[0][3];
                in1t.y = vec.y - this->matrix[1][3];
                in1t.z = vec.z - this->matrix[2][3];

                result.x = (in1t.x * this->matrix[0][0]) + (in1t.y * this->matrix[1][0]) + (in1t.z * this->matrix[2][0]);
                result.y = (in1t.x * this->matrix[0][1]) + (in1t.y * this->matrix[1][1]) + (in1t.z * this->matrix[2][1]);
                result.z = (in1t.x * this->matrix[0][2]) + (in1t.y * this->matrix[1][2]) + (in1t.z * this->matrix[2][2]);

                return result;
            }

            vec3 inverse_rotate_vec3(const vec3& vec) const
            {
                vec3 result = {};

                result.x = (vec.x * this->matrix[0][0]) + (vec.y * this->matrix[1][0]) + (vec.z * this->matrix[2][0]);
                result.y = (vec.x * this->matrix[0][1]) + (vec.y * this->matrix[1][1]) + (vec.z * this->matrix[2][1]);
                result.z = (vec.x * this->matrix[0][2]) + (vec.y * this->matrix[1][2]) + (vec.z * this->matrix[2][2]);

                return result;
            }
    };

    // Checks whenether a ray hits a axis-aligned bounding box
    inline ray_result ray_hits_aabbox(const vec3& origin, const vec3& direction, const bbox& aabbox)
    {
        vec3 direction_inverse = { 1 / direction.x, 1 / direction.y, 1 / direction.z };

        // If line is parallel and outsite the box it is not possible to intersect 
        if (direction.x == 0.0f && (origin.x < std::min(aabbox.bbmin.x, aabbox.bbmax.x) || origin.x > std::max(aabbox.bbmin.x, aabbox.bbmax.x)))
            return { false, 0 };
    
        if (direction.y == 0.0f && (origin.y < std::min(aabbox.bbmin.y, aabbox.bbmax.y) || origin.y > std::max(aabbox.bbmin.y, aabbox.bbmax.y)))
            return { false, 0 };
    
        if (direction.z == 0.0f && (origin.z < std::min(aabbox.bbmin.z, aabbox.bbmax.z) || origin.z > std::max(aabbox.bbmin.z, aabbox.bbmax.z)))
            return { false, 0 };

        float t1 = (aabbox.bbmin.x - origin.x) * direction_inverse.x;
        float t2 = (aabbox.bbmax.x - origin.x) * direction_inverse.x;
        float t3 = (aabbox.bbmin.y - origin.y) * direction_inverse.y;
        float t4 = (aabbox.bbmax.y - origin.y) * direction_inverse.y;
        float t5 = (aabbox.bbmin.z - origin.z) * direction_inverse.z;
        float t6 = (aabbox.bbmax.z - origin.z) * direction_inverse.z;
    
        float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
        float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

        if (tmax < 0) {
            return { false, tmax };
        }
    
        if (tmin > tmax) {
            return { false, tmax };
        }
    
        return { true, tmin };
    }

    // Checks whenether a ray hits a rotated bounding box
    inline ray_result ray_hits_rbbox(const vec3& origin, const vec3& direction, const bbox& aabbox, const matrix3x4& matrix)
    {
        vec3 transformed_origin = matrix.inverse_transform_vec3(origin);
        vec3 rotated_direction = matrix.inverse_rotate_vec3(direction);
        return ray_hits_aabbox(transformed_origin, rotated_direction, aabbox);
    }

    inline float to_deg(float angle)
    {
        return angle * (180 / pi);
    }

    inline float to_rad(float angle)
    {
        return angle * (pi / 180);
    }

    inline vec3 correct_movement(const vec3& original_angles, const vec3& new_angles, const vec3& movement)
    {
        if (original_angles == new_angles)
            return movement;

        // Angles differ

        vec3 result = {};
        // Solve upmove first
        if ((original_angles.x > 0.0 && new_angles.x < 0.0) || (original_angles.x < 0.0 && new_angles.x > 0.0))
        {
            // If we swapped which half of the 'screen' we are looking at, swap up-move
            result.z = -movement.z;
        }
        else
        {
            // Otherwise just keep it same
            result.z = movement.z;
        }

        
        float angle_difference = (new_angles.y - original_angles.y);
        float cs = std::cos(to_rad(angle_difference));
        float sn = std::sin(to_rad(angle_difference));

        result.x = (movement.x * cs) - (movement.y * sn);
        result.y = (movement.x * sn) + (movement.y * cs);

        return result;
    }


    // Nospread code
    static unsigned int glSeed = 0; 

    static unsigned int seed_table[ 256 ] =
    {
        28985, 27138, 26457, 9451, 17764, 10909, 28790, 8716, 6361, 4853, 17798, 21977, 19643, 20662, 10834, 20103,
        27067, 28634, 18623, 25849, 8576, 26234, 23887, 18228, 32587, 4836, 3306, 1811, 3035, 24559, 18399, 315,
        26766, 907, 24102, 12370, 9674, 2972, 10472, 16492, 22683, 11529, 27968, 30406, 13213, 2319, 23620, 16823,
        10013, 23772, 21567, 1251, 19579, 20313, 18241, 30130, 8402, 20807, 27354, 7169, 21211, 17293, 5410, 19223,
        10255, 22480, 27388, 9946, 15628, 24389, 17308, 2370, 9530, 31683, 25927, 23567, 11694, 26397, 32602, 15031,
        18255, 17582, 1422, 28835, 23607, 12597, 20602, 10138, 5212, 1252, 10074, 23166, 19823, 31667, 5902, 24630,
        18948, 14330, 14950, 8939, 23540, 21311, 22428, 22391, 3583, 29004, 30498, 18714, 4278, 2437, 22430, 3439,
        28313, 23161, 25396, 13471, 19324, 15287, 2563, 18901, 13103, 16867, 9714, 14322, 15197, 26889, 19372, 26241,
        31925, 14640, 11497, 8941, 10056, 6451, 28656, 10737, 13874, 17356, 8281, 25937, 1661, 4850, 7448, 12744,
        21826, 5477, 10167, 16705, 26897, 8839, 30947, 27978, 27283, 24685, 32298, 3525, 12398, 28726, 9475, 10208,
        617, 13467, 22287, 2376, 6097, 26312, 2974, 9114, 21787, 28010, 4725, 15387, 3274, 10762, 31695, 17320,
        18324, 12441, 16801, 27376, 22464, 7500, 5666, 18144, 15314, 31914, 31627, 6495, 5226, 31203, 2331, 4668,
        12650, 18275, 351, 7268, 31319, 30119, 7600, 2905, 13826, 11343, 13053, 15583, 30055, 31093, 5067, 761,
        9685, 11070, 21369, 27155, 3663, 26542, 20169, 12161, 15411, 30401, 7580, 31784, 8985, 29367, 20989, 14203,
        29694, 21167, 10337, 1706, 28578, 887, 3373, 19477, 14382, 675, 7033, 15111, 26138, 12252, 30996, 21409,
        25678, 18555, 13256, 23316, 22407, 16727, 991, 9236, 5373, 29402, 6117, 15241, 27715, 19291, 19888, 19847
    };

    inline uint32_t seed_table_random() 
    { 
        glSeed *= 69069;
        glSeed += seed_table[ glSeed & 0xff ];
    
        return ( ++glSeed & 0x0fffffff ); 
    } 

    inline void seed_rand(uint32_t seed)
    {
        glSeed = seed_table[ seed & 0xff ];
    }

    inline int32_t shared_random_int(uint32_t seed, int32_t low, int32_t high)
    {
        uint32_t range;

        seed_rand((int32_t)seed + low + high);

        range = high - low + 1;
        if ( !(range - 1) )
        {
            return low;
        }
        else
        {
            int32_t offset;
            int32_t rnum;

            rnum = seed_table_random();

            offset = rnum % range;

            return (low + offset);
        }
    }

    inline float shared_random_float(uint32_t seed, float low, float high)
    {
        uint32_t range;

        seed_rand((int32_t)seed + *(int32_t *)&low + *(int32_t *)&high);

        seed_table_random();
        seed_table_random();

        range = (uint32_t)(high - low);
        if ( !range )
        {
            return low;
        }
        else
        {
            int32_t tensixrand;
            float offset;

            tensixrand = seed_table_random() & 65535;

            offset = (float)tensixrand / 65536.0f;

            return (low + offset * range);
        }
    }

    inline vec3 get_spread_vec(uint32_t seed, int32_t future, float weapon_spread)
    {
        vec3 result = {};
        float vecspread, speed;

        result.x = shared_random_float(seed + future, -0.5, 0.5) + shared_random_float(seed + 1 + future, -0.5, 0.5);
        result.y = shared_random_float(seed + 2 + future, -0.5, 0.5) + shared_random_float(seed + 3 + future, -0.5, 0.5);

        result.x *= weapon_spread;
        result.y *= weapon_spread;

        return result;
    }
}