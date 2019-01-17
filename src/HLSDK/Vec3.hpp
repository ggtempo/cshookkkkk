#pragma once

#include <cmath>

const float pi = std::acos(-1.0);

typedef struct vec3_s
{
    public:
		// A vec3_s simply has three properties called x, y and z
		float x, y, z;

	public:
		// ------------ Constructors ------------

		// Default constructor
		vec3_s() : x(0), y(0), z(0) {}

		// Three parameter constructor
		vec3_s(float x, float y, float z) : x(x), y(y), z(z) {}

        // Copy constructor
        vec3_s(const vec3_s& other) : x(other.x), y(other.y), z(other.z) {}

        // Copy assignment
        vec3_s& operator=(const vec3_s& other)
        {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;

            return *this;
        }

		// Method to normalize a vector
		void normalize()
		{
			// Calculate the magnitude of our vector
			float magnitude = sqrt((x * x) + (y * y) + (z * z));

			// As long as the magnitude isn't zero, divide each element by the magnitude
			// to get the normalised value between -1 and +1
			if (magnitude != 0)
			{
				x /= magnitude;
				y /= magnitude;
				z /= magnitude;
			}
		}
        
		static float dot_product(const vec3_s &vec1, const vec3_s &vec2)
		{
			return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
		}

		float dot_product(const vec3_s &vec) const
		{
			return (x * vec.x) + (y * vec.y) + (z * vec.z);
		}

		static vec3_s cross_product(const vec3_s &vec1, const vec3_s &vec2)
		{
			return vec3_s(vec1.y * vec2.z - vec1.z * vec2.y, vec1.z * vec2.x - vec1.x * vec2.z, vec1.x * vec2.y - vec1.y * vec2.x);
		}

		// Easy adders
		void addX(float value) { x += value; }
		void addY(float value) { y += value; }
		void addZ(float value) { z += value; }

		static float get_distance(const vec3_s &v1, const vec3_s &v2)
		{
			float dx = v2.x - v1.x;
			float dy = v2.y - v1.y;
			float dz = v2.z - v1.z;

			return sqrt(dx * dx + dy * dy + dz * dz);
		}

		// ------------ Overloaded operators ------------

		// Overloaded addition operator to add vec3_ss together
		vec3_s operator+(const vec3_s &vector) const
		{
			return vec3_s(x + vector.x, y + vector.y, z + vector.z);
		}

		// Overloaded add and asssign operator to add vec3_ss together
		void operator+=(const vec3_s &vector)
		{
			x += vector.x;
			y += vector.y;
			z += vector.z;
		}

		// Overloaded subtraction operator to subtract a vec3_s from another vec3_s
		vec3_s operator-(const vec3_s &vector) const
		{
			return vec3_s(x - vector.x, y - vector.y, z - vector.z);
		}

		// Overloaded subtract and asssign operator to subtract a vec3_s from another vec3_s
		void operator-=(const vec3_s &vector)
		{
			x -= vector.x;
			y -= vector.y;
			z -= vector.z;
		}

		// Overloaded multiplication operator to multiply two vec3_ss together
		vec3_s operator*(const vec3_s &vector) const
		{
			return vec3_s(x * vector.x, y * vector.y, z * vector.z);
		}

		// Overloaded multiply operator to multiply a vector by a scalar
		vec3_s operator*(const float &value) const
		{
			return vec3_s(x * value, y * value, z * value);
		}

		// Overloaded multiply and assign operator to multiply a vector by a scalar
		void operator*=(const float &value)
		{
			x *= value;
			y *= value;
			z *= value;
		}

		// Overloaded multiply operator to multiply a vector by a scalar
		vec3_s operator/(const float &value) const
		{
			return vec3_s(x / value, y / value, z / value);
		}

		vec3_s operator/(const vec3_s &vector) const
		{
			return vec3_s(x / vector.x, y / vector.y, z / vector.z);
		}

		// Overloaded multiply and assign operator to multiply a vector by a scalar
		void operator/=(const float &value)
		{
			x /= value;
			y /= value;
			z /= value;
		}

		bool operator==(const vec3_s &b) const
		{
			return (x == b.x) && (y == b.y) && (z == b.z);
		}

		inline float length() const
		{
			return std::sqrt((x*x) + (y*y) + (z*z));
		}

        operator float*() { return &this->x; }
        operator const float* () const { return &this->x; }

        vec3_s to_angles() const
        {
            vec3_s angles = {};

            float tmp, yaw, pitch;
	
            if (this->y == 0 && this->x == 0)
            {
                yaw = 0;
                if (this->z > 0)
                    pitch = 90;
                else
                    pitch = 270;
            }
            else
            {
                yaw = (std::atan2(this->y, this->x) * 180 / pi);
                if (yaw < 0)
                    yaw += 360;

                tmp = std::sqrt (this->x * this->x + this->y * this->y);
                pitch = (std::atan2(this->z, tmp) * 180 / pi);
                if (pitch < 0)
                    pitch += 360;
            }
            
            angles.x = pitch;
            angles.y = yaw;
            angles.z = 0;

            return angles;
        }

        vec3_s multiply_add(float scale, const vec3_s& other) const
        {
            vec3_s result = {};

            result.x = this->x + (scale * other.x);
            result.y = this->y + (scale * other.y);
            result.z = this->z + (scale * other.z);

            return result;
        }
} vec3_t;