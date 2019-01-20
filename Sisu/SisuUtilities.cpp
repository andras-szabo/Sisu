#include "stdafx.h"
#include "SisuUtilities.h"

namespace Sisu
{
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
}
