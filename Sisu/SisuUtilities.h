#pragma once
namespace Sisu
{
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
		static Color Blue() { return Color(0.0, 0.0, 1.0, 1.0); }

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

		Matrix4() = default;
		Matrix4(Vector4 pr0, Vector4 pr1, Vector4 pr2, Vector4 pr3) :
			r0(pr0), r1(pr1), r2(pr2), r3(pr3) {}

		Vector4 r0, r1, r2, r3;
	};

	// Arithmetics
	Sisu::Vector3 operator*(const Sisu::Vector3& vec, float s);
	Sisu::Matrix4 operator*(const Sisu::Matrix4& a, const Sisu::Matrix4& b);
}
