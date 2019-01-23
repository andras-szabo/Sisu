#pragma once
#include <cmath>
#define PI 3.14159265

namespace Sisu
{
	struct Quat;

	struct Vector3
	{
		static Vector3 Zero() { return Vector3(0.0f, 0.0f, 0.0f); }

		Vector3() = default;
		Vector3(float px, float py, float pz) : x(px), y(py), z(pz) {}

		float x, y, z;

		Vector3& operator+=(const Vector3& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;

			return *this;
		}
	};

	struct Color
	{
		static Color Blue() { return Color(0.0f, 0.0f, 1.0f, 1.0f); }
		static Color Red()  { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
		static Color Black() { return Color(0.0f, 0.0f, 0.0f, 1.0f); }

		Color() = default;
		Color(float pr, float pg, float pb, float pa) : r(pr), g(pg), b(pb), a(pa) {}

		float r, g, b, a;
	};

	struct Vector4
	{
		Vector4() = default;
		Vector4(float px, float py, float pz, float pw) :
			x(px), y(py), z(pz), w(pw) {}

		float x, y, z, w;
	};

	struct Matrix4
	{
		static Matrix4 Identity()
		{
			return Matrix4(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
						   Vector4(0.0f, 1.0f, 0.0f, 0.0f),
						   Vector4(0.0f, 0.0f, 1.0f, 0.0f),
						   Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		static Matrix4 FromQuat(Quat q);

		Matrix4() = default;
		Matrix4(Vector4 pr0, Vector4 pr1, Vector4 pr2, Vector4 pr3) :
			r0(pr0), r1(pr1), r2(pr2), r3(pr3) {}

		Vector4 r0, r1, r2, r3;
	};

	struct Quat
	{
		static Quat Identity()
		{
			return Quat();
		}

		// about upright X: pitch, about upright Y: heading, about upright Z: bank
		static Quat Euler(float pitch, float heading, float bank)
		{
			auto p = pitch * (PI / 180.0);
			auto h = heading * (PI / 180.0);
			auto b = bank * (PI / 180.0);

			auto cosp = cos(p / 2.0);
			auto cosh = cos(h / 2.0);
			auto cosb = cos(b / 2.0);

			auto sinp = sin(p / 2.0);
			auto sinh = sin(h / 2.0);
			auto sinb = sin(b / 2.0);

			Quat euler;

			euler.x = cosh * sinp * cosb + sinh * cosp * sinb;
			euler.y = sinh * cosp * cosb - cosh * sinp * sinb;
			euler.z = cosh * cosp * sinb - sinh * sinp * cosb;

			euler.w = cosh * cosp * cosb + sinh * sinp * sinb;

			return euler;
		}

		static Quat Euler(Sisu::Vector3 v)
		{
			return Euler(v.x, v.y, v.z);
		}

		Quat(): x(0.0), y(0.0), z(0.0), w(1.0) {}
		
		Quat(float px, float py, float pz, float pw) :
			x(px), y(py), z(pz), w(pw) {}

		Quat& operator=(const Quat& other)
		{
			if (&other != this)
			{
				x = other.x;
				y = other.y;
				z = other.z;
				w = other.w;
			}

			return *this;
		}

		float Magnitude() const
		{
			return sqrtf(pow(x, 2) + pow(y, 2) + pow(z, 2) + pow(w, 2));
		}

		float x, y, z, w;
	};

	// Arithmetics & stuff
	bool Approx(float a, float b, float e = 0.000001);
	Sisu::Vector3 operator*(const Sisu::Vector3& vec, float s);
	Sisu::Matrix4 operator*(const Sisu::Matrix4& a, const Sisu::Matrix4& b);
	Sisu::Quat operator*(const Sisu::Quat& a, const Sisu::Quat& b);
	bool operator==(const Sisu::Quat& a, const Sisu::Quat& b);
}
