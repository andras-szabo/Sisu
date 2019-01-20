#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Sisu/SisuUtilities.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(UtilityTests)
	{
	public:
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