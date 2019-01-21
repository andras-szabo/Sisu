#include "stdafx.h"
#include "SisuUtilities.h"

namespace Sisu
{
	bool Sisu::Approx(float a, float b, float e)
	{
		return (a > b) ? (a - b < e) : (b - a < e);
	}

	Sisu::Matrix4 Sisu::Matrix4::FromQuat(Quat q)
	{
		float r0x = 1.0f - 2.0f * pow(q.y, 2) - 2.0f * pow(q.z, 2);
		float r0y = 2.0f * q.x * q.y + 2.0f * q.w * q.z;
		float r0z = 2.0f * q.x * q.z - 2.0f * q.w * q.y;
		
		float r1x = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
		float r1y = 1.0f - 2.0f * pow(q.x, 2) - 2.0f * pow(q.z, 2);
		float r1z = 2.0f * q.y * q.z + 2.0f * q.w * q.x;

		float r2x = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
		float r2y = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
		float r2z = 1.0f - 2.0f * pow(q.x, 2) - 2.0f * pow(q.y, 2);

		return Matrix4(Vector4(r0x, r0y, r0z, 0.0f),
					   Vector4(r1x, r1y, r1z, 0.0f),
					   Vector4(r2x, r2y, r2z, 0.0f),
					   Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	Sisu::Vector3 operator*(const Sisu::Vector3& vec, float s)
	{
		return Sisu::Vector3(vec.x * s, vec.y * s, vec.z * s);
	}

	Sisu::Matrix4 operator*(const Sisu::Matrix4& a, const Sisu::Matrix4& b)
	{
		Sisu::Vector4 r0, r1, r2, r3;

		r0.x = (a.r0.x * b.r0.x) + (a.r0.y * b.r1.x) + (a.r0.z * b.r2.x) + (a.r0.w * b.r3.x);
		r0.y = (a.r0.x * b.r0.y) + (a.r0.y * b.r1.y) + (a.r0.z * b.r2.y) + (a.r0.w * b.r3.y);
		r0.z = (a.r0.x * b.r0.z) + (a.r0.y * b.r1.z) + (a.r0.z * b.r2.z) + (a.r0.w * b.r3.z);
		r0.w = (a.r0.x * b.r0.w) + (a.r0.y * b.r1.w) + (a.r0.z * b.r2.w) + (a.r0.w * b.r3.w);

		r1.x = (a.r1.x * b.r0.x) + (a.r1.y * b.r1.x) + (a.r1.z * b.r2.x) + (a.r1.w * b.r3.x);
		r1.y = (a.r1.x * b.r0.y) + (a.r1.y * b.r1.y) + (a.r1.z * b.r2.y) + (a.r1.w * b.r3.y);
		r1.z = (a.r1.x * b.r0.z) + (a.r1.y * b.r1.z) + (a.r1.z * b.r2.z) + (a.r1.w * b.r3.z);
		r1.w = (a.r1.x * b.r0.w) + (a.r1.y * b.r1.w) + (a.r1.z * b.r2.w) + (a.r1.w * b.r3.w);

		r2.x = (a.r2.x * b.r0.x) + (a.r2.y * b.r1.x) + (a.r2.z * b.r2.x) + (a.r2.w * b.r3.x);
		r2.y = (a.r2.x * b.r0.y) + (a.r2.y * b.r1.y) + (a.r2.z * b.r2.y) + (a.r2.w * b.r3.y);
		r2.z = (a.r2.x * b.r0.z) + (a.r2.y * b.r1.z) + (a.r2.z * b.r2.z) + (a.r2.w * b.r3.z);
		r2.w = (a.r2.x * b.r0.w) + (a.r2.y * b.r1.w) + (a.r2.z * b.r2.w) + (a.r2.w * b.r3.w);

		r3.x = (a.r3.x * b.r0.x) + (a.r3.y * b.r1.x) + (a.r3.z * b.r2.x) + (a.r3.w * b.r3.x);
		r3.y = (a.r3.x * b.r0.y) + (a.r3.y * b.r1.y) + (a.r3.z * b.r2.y) + (a.r3.w * b.r3.y);
		r3.z = (a.r3.x * b.r0.z) + (a.r3.y * b.r1.z) + (a.r3.z * b.r2.z) + (a.r3.w * b.r3.z);
		r3.w = (a.r3.x * b.r0.w) + (a.r3.y * b.r1.w) + (a.r3.z * b.r2.w) + (a.r3.w * b.r3.w);

		return Sisu::Matrix4(r0, r1, r2, r3);
	}

	Sisu::Quat operator*(const Sisu::Quat& a, const Sisu::Quat& b)
	{
		float w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;

		float x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
		float y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
		float z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;

		return Quat(x, y, z, w);
	}

	bool operator==(const Sisu::Quat& a, const Sisu::Quat& b)
	{
		return Approx(a.x, b.x) &&
			   Approx(a.y, b.y) &&
			   Approx(a.z, b.z) &&
			   Approx(a.w, b.w);
	}

}
