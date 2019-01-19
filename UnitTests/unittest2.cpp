#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Sisu/Arena.h"
#include "../Sisu/GameObject.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(GameObjectsInArena)
	{
	public:
		TEST_METHOD(AddSimpleGO)
		{
			Arena<GameObject> a(10);
			GameObject::AddToArena(a, GameObject());
			Assert::IsTrue(a[0].index == 0);

			GameObject::AddToArena(a, GameObject());
			Assert::IsTrue(a[1].index == 1);
		}

		TEST_METHOD(AddChild)
		{
			Arena<GameObject> a;
			auto parentIndex = GameObject::AddToArena(a, GameObject());
			auto childIndex = GameObject::AddChild(a, parentIndex, GameObject());

			Assert::IsTrue(parentIndex == 0);
			Assert::IsTrue(childIndex == 1);

			const auto& parent = a[parentIndex];
			const auto& child = a[childIndex];

			Assert::IsTrue(parent.hasChildren);

			Assert::IsTrue(parent.hasChildren && parent.childrenStartIndex == childIndex && parent.childrenEndIndex == childIndex);
			Assert::IsTrue(child.parentIndex == parentIndex);

			auto otherChild = GameObject::AddChild(a, parentIndex, GameObject());
			Assert::IsTrue(otherChild == 2);
			Assert::IsTrue(a[parentIndex].childrenStartIndex == 1 && a[parentIndex].childrenEndIndex == 2);
			Assert::IsTrue(a[otherChild].parentIndex == parentIndex);

			// So now, in the arena, we have:
			// parent, child, child, empty, empty, empty ....
			// Now we add a new parent =>
			// parent, child, child, newParent, empty, empty ...
			// So when we add the next kid, we'll have to relocate the first two children:
			//
			// parent, ____, ____, newParent, child, child, child, ____

			auto newGOindex = GameObject::AddToArena(a, GameObject());
			Assert::IsTrue(newGOindex == 3);
			auto thirdChild = GameObject::AddChild(a, parentIndex, GameObject());
			Assert::IsTrue(a[parentIndex].childrenStartIndex == 4 && a[parentIndex].childrenEndIndex == 6);
			Assert::IsTrue(a[4].parentIndex == parentIndex && a[5].parentIndex == parentIndex && a[6].parentIndex == parentIndex);
			Assert::IsTrue(a.CanAddItemAt(1) && a.CanAddItemAt(2));

			// Now let's try to add new kid to newParent, it should go right before it!
			// parent, _____, freshKid, freshKidsParent, child, child, child, ____ ...
			auto freshChild = GameObject::AddChild(a, newGOindex, GameObject());
			Assert::IsTrue(freshChild == 2);
			Assert::IsTrue(a.CanAddItemAt(1));
			Assert::IsTrue(a[newGOindex].hasChildren && a[newGOindex].childrenStartIndex == 2 && a[newGOindex].childrenEndIndex == 2);
			Assert::IsTrue(a[freshChild].parentIndex == 3);
		}
		
		TEST_METHOD(AddAndRelocateChildren)
		{
			Arena<GameObject> a;
			auto parentIndex = GameObject::AddToArena(a, GameObject());
			Assert::IsTrue(parentIndex == 0);
			std::vector<GameObject> children;
			for (int i = 0; i < 4; ++i)
			{
				children.push_back(GameObject());
			}

			auto childCount = children.end() - children.begin();
			auto firstChildIndex = GameObject::AddChildren(a, parentIndex, children.begin(), children.end());
			Assert::IsTrue(firstChildIndex == 1);
			Assert::IsTrue(a[parentIndex].hasChildren);
			Assert::IsTrue(a[parentIndex].childrenStartIndex == firstChildIndex && a[parentIndex].childrenEndIndex == firstChildIndex + 3);

			auto newDudeIndex = GameObject::AddToArena(a, GameObject());
			Assert::IsTrue(newDudeIndex == 5);
			// parent, c1, c2, c3, c4, newDude

			children.clear();
			for (int i = 0; i < 2; ++i)
			{
				children.push_back(GameObject());
			}

			GameObject::AddChildren(a, parentIndex, children.begin(), children.end());
			// expected:
			// parent, _, _, _, _, newDude, c1, c2, c3, c4, c5, c6
			//	0		1 2  3  4  5        6   7   8 	9   10  11
			Assert::IsTrue(a[parentIndex].childrenStartIndex == 6);
			Assert::IsTrue(a[parentIndex].childrenEndIndex == 11);
		}

		//TODO: Tests w/ hierarchies

		TEST_METHOD(AddChildren)
		{
			Arena<GameObject> a;
			auto parentIndex = GameObject::AddToArena(a, GameObject());
			Assert::IsTrue(parentIndex == 0);

			std::vector<GameObject> children;
			for (int i = 0; i < 4; ++i)
			{
				children.push_back(GameObject());
			}

			auto childCount = (children.end() - children.begin());

			auto firstChildIndex = GameObject::AddChildren(a, parentIndex, children.begin(), children.end());
			Assert::IsTrue(firstChildIndex == 1);
			Assert::IsTrue(a[parentIndex].hasChildren); 
			Assert::IsTrue(a[parentIndex].childrenStartIndex == firstChildIndex && a[parentIndex].childrenEndIndex == firstChildIndex + 3);

			for (int i = 0; i < 4; ++i)
			{
				Assert::IsTrue(a[firstChildIndex + i].parentIndex == parentIndex);
			}

			std::vector<GameObject> newKids;
			for (int i = 0; i < 2; ++i)
			{
				newKids.push_back(GameObject());
			}

			childCount = newKids.end() - newKids.begin();
			firstChildIndex = GameObject::AddChildren(a, parentIndex, newKids.begin(), newKids.end());
			Assert::IsTrue(firstChildIndex == 1);
			Assert::IsTrue(a[parentIndex].hasChildren);
			Assert::IsTrue(a[parentIndex].childrenStartIndex == firstChildIndex && a[parentIndex].childrenEndIndex == firstChildIndex + 5);
		}
	};
}