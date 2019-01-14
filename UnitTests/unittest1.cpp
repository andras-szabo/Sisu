#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Sisu/Arena.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{		
	TEST_CLASS(ArenaTests)
	{
	public:
		
		TEST_METHOD(CreateArena)
		{
			Arena<int> a;
			a.Add(1);
			Assert::IsTrue(a.OccupiedSize() == 1 && a.ActualSize() == 1);

			auto second = a.Add(1);
			Assert::IsTrue(a.OccupiedSize() == 2 && second == 1 && a.ActualSize() == 2);

			auto third = a.Add(1);
			Assert::IsTrue(a.OccupiedSize() == 3 && third == 2);

			a.RemoveAt(second);
			Assert::IsTrue(a.OccupiedSize() == 3 && a.ActualSize() == 2);

			auto fourth = a.Add(1);

			Assert::IsTrue(a.OccupiedSize() == 3 && a.ActualSize() == 3);
			Assert::IsTrue(fourth == second);
		}

		TEST_METHOD(Gaps)
		{
			Arena<int> a;
			for (int i = 0; i < 10; ++i) { a.Add(i); }
			Assert::IsTrue(a.ActualSize() == 10 && a.OccupiedSize() == 10);

			for (int i = 0; i < 10; i += 2) { a.RemoveAt(i); }
			Assert::IsTrue(a.ActualSize() == 5 && a.OccupiedSize() == 10);

			auto newItemIndex = a.Add(11);
			Assert::IsTrue(a.ActualSize() == 6 && a.OccupiedSize() == 10);
			Assert::IsTrue(newItemIndex == 8);

			auto newerItemIndex = a.Add(123);
			Assert::IsTrue(a.ActualSize() == 7 && a.OccupiedSize() == 10);
			Assert::IsTrue(newerItemIndex == 6);
		}

		TEST_METHOD(BigGaps)
		{
			auto success = true;

			Arena<int> a;
			for (int i = 0; i < 10; ++i) { a.Add(i); }

			std::vector<int> vec{ 99, 99, 99 };
			auto newIndex = a.Add(std::begin(vec), std::end(vec));
			Assert::IsTrue(newIndex == 10);
			Assert::IsTrue(a.ActualSize() == 13 && a.OccupiedSize() == 13);

			for (int i = 2; i < 5; ++i) { a.RemoveAt(i); }
			Assert::IsTrue(a.ActualSize() == 10 && a.OccupiedSize() == 13);

			newIndex = a.Add(std::begin(vec), std::end(vec));
			Assert::IsTrue(a.ActualSize() == 13 && a.OccupiedSize() == 13);
			Assert::IsTrue(newIndex == 2);

			a.RemoveAt(7);
			a.RemoveAt(8);

			newIndex = a.Add(std::begin(vec), std::end(vec));
			Assert::IsTrue(a.ActualSize() == 14 && a.OccupiedSize() == 16);
			Assert::IsTrue(newIndex == 13);

			std::vector<int> smallVec{ 123, 123 };
			newIndex = a.Add(std::begin(smallVec), std::end(smallVec));
			Assert::IsTrue(newIndex == 7);
			Assert::IsTrue(a.ActualSize() == 16 && a.OccupiedSize() == 16);

			a.RemoveAt(0, 5);
			Assert::IsTrue(a.ActualSize() == 11 && a.OccupiedSize() == 16);
		}

		TEST_METHOD(MoreGaps)
		{
			auto success = true;

			Arena<int> a(50);
			for (int i = 0; i < 50; ++i) { a.Add(i); }
			Assert::IsTrue(a.ActualSize() == 50 && a.OccupiedSize() == 50);

			for (int i = 0; i < 50; i += 8) { a.RemoveAt(i, 4); }

			// this is how the end should look like
			// 39, __, __, __, __, 44, 45, 46, 47, __, __

			std::vector<int> vec{ 99, 99, 99 };
			auto newIndex = a.Add(std::begin(vec), std::end(vec));
			Assert::IsTrue(a.OccupiedSize() == 50 && newIndex == 40);

			// now:
			// 39, 99, 99, 99, __, 44, 45, 46, 47, __, __

			newIndex = a.Add(123);
			Assert::IsTrue(a.OccupiedSize() == 50 && newIndex == 43);

			std::vector<int> pair{ 123, 123 };
			newIndex = a.Add(std::begin(pair), std::end(pair));
			Assert::IsTrue(a.OccupiedSize() == 50 && newIndex == 48);
		}

		TEST_METHOD(Clear)
		{
			Arena<int> a(50);
			Assert::IsTrue(a.begin() == a.end());

			for (int i = 0; i < 50; ++i) { a.Add(i); }
			Assert::IsTrue(a.ActualSize() == 50 && a.OccupiedSize() == 50);
			Assert::IsTrue(a.begin() != a.end());

			a.Clear();
			Assert::IsTrue(a.ActualSize() == 0 && a.OccupiedSize() == 0);
			Assert::IsTrue(a.begin() == a.end());

			std::vector<int> vec{ 99, 99, 99 };
			auto newIndex = a.Add(std::begin(vec), std::end(vec));
			Assert::IsTrue(a.OccupiedSize() == 3 && a.ActualSize() == 3 && newIndex == 0);
			Assert::IsTrue(a.begin() != a.end());
		}

		TEST_METHOD(Indices)
		{
			Arena<int> a(50);
			for (int i = 0; i < 50; ++i) { a.Add(i); }

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
			for (int i = 0; i < 10; ++i) { a.Add(i); }

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
			for (int i = 0; i < 10; ++i) { b.Add(i); }
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

			empty.Add(123);
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

			a.Add(0);
			a.Add(vec.begin(), vec.end());

			auto index = 0;
			for (auto& item : a) { Assert::IsTrue(item == index++); }
			Assert::IsTrue(index == 4);

			a.Add(4);				// 0, 1, 2, 3, 4
			a.RemoveAt(1, 3);		// 0, _, _, _, 4
			std::vector<int> check{ 0, 4 };
			Assert::IsTrue(a.ActualSize() == 2);
			index = 0;
			for (auto& item : a) { Assert::IsTrue(item == check[index++]); }

			a.Add(otherVec.begin(), otherVec.end());	// 0, _, _, _, 4, 99, 99, 99, 99
			Assert::IsTrue(a.ActualSize() == 6);
			check = std::vector<int>{ 0, 4, 99, 99, 99, 99 };
			index = 0;
			for (auto& item : a) { Assert::IsTrue(item == check[index++]); }

			a.Add(vec.begin(), vec.end());				// 0, 1, 2, 3, 4, 99, 99, 99, 99
			Assert::IsTrue(a.ActualSize() == 9);
			check = std::vector<int>{ 0, 1, 2, 3, 4, 99, 99, 99, 99 };
			index = 0;
			for (auto& item : a) { Assert::IsTrue(item == check[index++]); }
		}
	};
}