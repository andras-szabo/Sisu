#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Sisu/SisuUtilities.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(UtilityTests)
	{
	public:
		TEST_METHOD(QuatCreation)
		{
			auto qi = Sisu::Quat::Identity();
			Assert::IsTrue(qi.w == 1.0 && qi.x == 0 && qi.y == 0 && qi.z == 0);

			auto qe = Sisu::Quat::Euler(10.0f, 20.0f, 30.0f);
			Assert::IsTrue(Sisu::Approx(qe.Magnitude(), 1.0f));
		}

		TEST_METHOD(MatrixCreation)
		{
			auto qe = Sisu::Quat::Euler(0.0f, 90.0f, 0.0f);
			auto m = Sisu::Matrix4::FromQuat(qe);
		}

		TEST_METHOD(QuatMul)
		{
			auto qa = Sisu::Quat::Euler(0.0f, 90.0f, 0.0f);
			auto qb = Sisu::Quat::Euler(0.0f, -90.0f, 0.0f);
			auto prod = qa * qb;
			auto m = Sisu::Matrix4::FromQuat(prod);

			Assert::IsTrue(Sisu::Approx(m.r0.x, 1.0f) && Sisu::Approx(m.r1.y, 1.0f) && 
						   Sisu::Approx(m.r2.z, 1.0f) && Sisu::Approx(m.r3.w, 1.0f));

		}

		TEST_METHOD(QuatMul2)
		{
			auto qa = Sisu::Quat::Euler(10.0f, 0.0f, 0.0f);
			auto qb = Sisu::Quat::Euler(10.0f, 0.0f, 0.0f);

			auto prod = qa * qb;
			auto prod2 = qb * qa;

			auto expected = Sisu::Quat::Euler(20.0f, 0.0f, 0.0f);

			Assert::IsTrue(prod == prod2);
			Assert::IsTrue(prod == expected);
		}

		TEST_METHOD(Matrix4x4Mul)
		{
			auto a = Sisu::Matrix4::Identity();
			auto b = Sisu::Matrix4::Identity();

			a.r0.x = 2.0;
			a.r1.y = 2.0;

			auto res = a * b;
			Assert::IsTrue(res.r0.x == 2.0 && res.r1.y == 2.0);
		}
	};
}