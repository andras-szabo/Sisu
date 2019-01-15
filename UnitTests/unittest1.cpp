#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Sisu/Arena.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{		
	struct TestData
	{
		TestData(float px, float py) : x(px), y(py) {}
		float x, y;
	};

	TEST_CLASS(ArenaTests)
	{
	public:
		TEST_METHOD(AnotherGapTest)
		{
			Arena<int> a(50);
			for (int i = 0; i < 50; ++i) { a.AddAnywhere(i); }
			for (int i = 0; i < 10; ++i) { a.RemoveAt(i); a.RemoveAt(49 - i); }
			Assert::IsTrue(a.GetStartIndexForGap(5, 20) == 5);
			Assert::IsTrue(a.GetStartIndexForGap(5, 42) == 40);
		}

		TEST_METHOD(OneMoreGapTest)
		{
			Arena<int> a(10);
			for (int i = 0; i < 10; ++i) { a.AddAnywhere(i); }
			a.RemoveAt(0, 2);	
			a.RemoveAt(8, 1);	// _, _, 2, 3, 4, 5, 6, 7, _, 9

			// AddAnywhere a single item, and it should go to 8, because it goes for best fit.
			// But with the constraint of "as close to 3 as possible", it should add it to 1

			Assert::IsTrue(a.GetStartIndexForGap(1, 3) == 1);
			Assert::IsTrue(a.AddAnywhere(99) == 8);
		}

		TEST_METHOD(PushBackAt)
		{
			Arena<TestData> a(10);
			for (int i = 0; i < 10; ++i) { a.AddAnywhere(TestData((float) i, (float) i)); }

			a.RemoveAt(1, 2);
			a.RemoveAt(8, 2);	// 0, _, _, 3, 4, 5, 6, 7, _, _

			auto index = a.GetStartIndexForGap(1, 3);
			Assert::IsTrue(index == 2);

			a.AddAt(index, TestData(99.0f, 99.0f));
			Assert::IsTrue(a[2].x > 90.0f);	// 0, _, 99, 3, 4, 5, 6, 7, _, _

			Assert::IsTrue(a.GetStartIndexForGap(3, 4) == 8);

			for (std::size_t i = 8; i < 11; ++i)
			{
				a.AddAt(i, TestData(123.0f, 123.0f));
			}

			Assert::IsTrue(a.ItemCount() == 10);
		}

		TEST_METHOD(GetSuitableRange2)
		{
			Arena<int> a(10);
			for (int i = 0; i < 10; ++i) { a.AddAnywhere(i); }
			
			a.RemoveAt(1, 2);
			a.RemoveAt(8, 2);	// 0, _, _, 3, 4, 5, 6, 7, _, _
			a.RemoveAt(3, 2);	// 0, _, _, _, _, 5, 6, 7, _, _

			Assert::IsTrue(a.GetStartIndexForGap(4, 0) == 1);
			Assert::IsTrue(a.GetStartIndexForGap(3, 5) == 2);
			Assert::IsTrue(a.GetStartIndexForGap(2, 5) == 3);
			Assert::IsTrue(a.GetStartIndexForGap(1, 5) == 4);
		}

		TEST_METHOD(GetSuitableRange)
		{
			Arena<int> a(10);
			Assert::IsTrue(a.GetStartIndexForGap(5, 0) == 0);

			for (int i = 0; i < 10; ++i) { a.AddAnywhere(i); }
			Assert::IsTrue(a.GetStartIndexForGap(2, 5) == 10);

			a.RemoveAt(1, 2);
			a.RemoveAt(8, 2);	// 0, _, _, 3, 4, 5, 6, 7, _, _

			Assert::IsTrue(a.GetStartIndexForGap(2, 0) == 1, L"first");
			Assert::IsTrue(a.GetStartIndexForGap(2, 6) == 8, L"second");
			Assert::IsTrue(a.GetStartIndexForGap(3, 3) == 8, L"third");
			Assert::IsTrue(a.GetStartIndexForGap(1, 3) == 2, L"fourth");
			Assert::IsTrue(a.GetStartIndexForGap(10, 5) == 8, L"fifth");
		}
		
		TEST_METHOD(CreateArena)
		{
			Arena<int> a;
			a.AddAnywhere(1);
			Assert::IsTrue(a.OccupiedSize() == 1 && a.ItemCount() == 1);

			auto second = a.AddAnywhere(1);
			Assert::IsTrue(a.OccupiedSize() == 2 && second == 1 && a.ItemCount() == 2);

			auto third = a.AddAnywhere(1);
			Assert::IsTrue(a.OccupiedSize() == 3 && third == 2);

			a.RemoveAt(second);
			Assert::IsTrue(a.OccupiedSize() == 3 && a.ItemCount() == 2);

			auto fourth = a.AddAnywhere(1);

			Assert::IsTrue(a.OccupiedSize() == 3 && a.ItemCount() == 3);
			Assert::IsTrue(fourth == second);
		}

		TEST_METHOD(Gaps)
		{
			Arena<int> a;
			for (int i = 0; i < 10; ++i) { a.AddAnywhere(i); }
			Assert::IsTrue(a.ItemCount() == 10 && a.OccupiedSize() == 10);

			for (int i = 0; i < 10; i += 2) { a.RemoveAt(i); }
			Assert::IsTrue(a.ItemCount() == 5 && a.OccupiedSize() == 10);

			auto newItemIndex = a.AddAnywhere(11);
			Assert::IsTrue(a.ItemCount() == 6 && a.OccupiedSize() == 10);
			Assert::IsTrue(newItemIndex == 8);

			auto newerItemIndex = a.AddAnywhere(123);
			Assert::IsTrue(a.ItemCount() == 7 && a.OccupiedSize() == 10);
			Assert::IsTrue(newerItemIndex == 6);
		}

		TEST_METHOD(BigGaps)
		{
			auto success = true;

			Arena<int> a;
			for (int i = 0; i < 10; ++i) { a.AddAnywhere(i); }

			std::vector<int> vec{ 99, 99, 99 };
			auto newIndex = a.AddAnywhere(std::begin(vec), std::end(vec));
			Assert::IsTrue(newIndex == 10);
			Assert::IsTrue(a.ItemCount() == 13 && a.OccupiedSize() == 13);

			for (int i = 2; i < 5; ++i) { a.RemoveAt(i); }
			Assert::IsTrue(a.ItemCount() == 10 && a.OccupiedSize() == 13);

			newIndex = a.AddAnywhere(std::begin(vec), std::end(vec));
			Assert::IsTrue(a.ItemCount() == 13 && a.OccupiedSize() == 13);
			Assert::IsTrue(newIndex == 2);

			a.RemoveAt(7);
			a.RemoveAt(8);

			newIndex = a.AddAnywhere(std::begin(vec), std::end(vec));
			Assert::IsTrue(a.ItemCount() == 14 && a.OccupiedSize() == 16);
			Assert::IsTrue(newIndex == 13);

			std::vector<int> smallVec{ 123, 123 };
			newIndex = a.AddAnywhere(std::begin(smallVec), std::end(smallVec));
			Assert::IsTrue(newIndex == 7);
			Assert::IsTrue(a.ItemCount() == 16 && a.OccupiedSize() == 16);

			a.RemoveAt(0, 5);
			Assert::IsTrue(a.ItemCount() == 11 && a.OccupiedSize() == 16);
		}

		TEST_METHOD(MoreGaps)
		{
			auto success = true;

			Arena<int> a(50);
			for (int i = 0; i < 50; ++i) { a.AddAnywhere(i); }
			Assert::IsTrue(a.ItemCount() == 50 && a.OccupiedSize() == 50);

			for (int i = 0; i < 50; i += 8) { a.RemoveAt(i, 4); }

			// this is how the end should look like
			// 39, __, __, __, __, 44, 45, 46, 47, __, __

			std::vector<int> vec{ 99, 99, 99 };
			auto newIndex = a.AddAnywhere(std::begin(vec), std::end(vec));
			Assert::IsTrue(a.OccupiedSize() == 50 && newIndex == 40);

			// now:
			// 39, 99, 99, 99, __, 44, 45, 46, 47, __, __

			newIndex = a.AddAnywhere(123);
			Assert::IsTrue(a.OccupiedSize() == 50 && newIndex == 43);

			std::vector<int> pair{ 123, 123 };
			newIndex = a.AddAnywhere(std::begin(pair), std::end(pair));
			Assert::IsTrue(a.OccupiedSize() == 50 && newIndex == 48);
		}

		TEST_METHOD(Clear)
		{
			Arena<int> a(50);
			Assert::IsTrue(a.begin() == a.end());

			for (int i = 0; i < 50; ++i) { a.AddAnywhere(i); }
			Assert::IsTrue(a.ItemCount() == 50 && a.OccupiedSize() == 50);
			Assert::IsTrue(a.begin() != a.end());

			a.Clear();
			Assert::IsTrue(a.ItemCount() == 0 && a.OccupiedSize() == 0);
			Assert::IsTrue(a.begin() == a.end());

			std::vector<int> vec{ 99, 99, 99 };
			auto newIndex = a.AddAnywhere(std::begin(vec), std::end(vec));
			Assert::IsTrue(a.OccupiedSize() == 3 && a.ItemCount() == 3 && newIndex == 0);
			Assert::IsTrue(a.begin() != a.end());
		}

		TEST_METHOD(Indices)
		{
			Arena<int> a(50);
			for (int i = 0; i < 50; ++i) { a.AddAnywhere(i); }

			for (int i = 0; i < 50; i += 9)
			{
				Assert::IsTrue(a[i] == i);
				a[i] = 123;
			}

			Assert::IsTrue(a[0] == 123 && a[9] == 123);
		}

		TEST_METHOD(RangeFor)
		{
			Arena<int> a(10);
			for (int i = 0; i < 10; ++i) { a.AddAnywhere(i); }

			auto index = 0;
			for (auto& item : a)
			{
				Assert::IsTrue(item == index++);
			}

			for (int i = 0; i < 10; i += 2)
			{
				a.RemoveAt(i);
			}

			// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
			// _, 1, _, 3, _, 5, _ 7, _, 9
			index = 0;
			for (auto& item : a)
			{
				Assert::IsTrue(item == 1 + index * 2);
				index++;
			}

			Arena<int> b(10);
			for (int i = 0; i < 10; ++i) { b.AddAnywhere(i); }
			b.RemoveAt(0, 2);
			b.RemoveAt(9);
			
			auto i = 2;
			for (auto& item : b)
			{
				Assert::IsTrue(item == i++);
			}

			Assert::IsTrue(i == 9);

			Arena<int> empty(123);
			for (auto& item : empty)
			{
				// Because this is an empty Arena, the for loop should not
				// execute even once.
				Assert::IsTrue(false);
			}

			empty.AddAnywhere(123);
			for (auto& item : empty)
			{
				Assert::IsTrue(item == 123);
			}

			empty.RemoveAt(0);
			for (auto& item : empty)
			{
				// See above; this should never run
				Assert::IsTrue(false);
			}
		}

		TEST_METHOD(RangeForAndVectorAdd)
		{
			Arena<int> a;
			std::vector<int> vec{ 1, 2, 3 };
			std::vector<int> otherVec{ 99, 99, 99, 99 };

			a.AddAnywhere(0);
			a.AddAnywhere(vec.begin(), vec.end());

			auto index = 0;
			for (auto& item : a) { Assert::IsTrue(item == index++); }
			Assert::IsTrue(index == 4);

			a.AddAnywhere(4);				// 0, 1, 2, 3, 4
			a.RemoveAt(1, 3);		// 0, _, _, _, 4
			std::vector<int> check{ 0, 4 };
			Assert::IsTrue(a.ItemCount() == 2);
			index = 0;
			for (auto& item : a) { Assert::IsTrue(item == check[index++]); }

			a.AddAnywhere(otherVec.begin(), otherVec.end());	// 0, _, _, _, 4, 99, 99, 99, 99
			Assert::IsTrue(a.ItemCount() == 6);
			check = std::vector<int>{ 0, 4, 99, 99, 99, 99 };
			index = 0;
			for (auto& item : a) { Assert::IsTrue(item == check[index++]); }

			a.AddAnywhere(vec.begin(), vec.end());				// 0, 1, 2, 3, 4, 99, 99, 99, 99
			Assert::IsTrue(a.ItemCount() == 9);
			check = std::vector<int>{ 0, 1, 2, 3, 4, 99, 99, 99, 99 };
			index = 0;
			for (auto& item : a) { Assert::IsTrue(item == check[index++]); }
		}
	};
}