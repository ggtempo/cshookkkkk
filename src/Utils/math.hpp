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
}