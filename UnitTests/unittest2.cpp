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
			Arena<GameObject> a(10);
			auto parentIndex = GameObject::AddToArena(a, GameObject());
			auto childIndex = GameObject::AddChild(a, parentIndex, GameObject());

			Assert::IsTrue(parentIndex == 0);
			Assert::IsTrue(childIndex == 1);

			const auto& parent = a[parentIndex];
			const auto& child = a[childIndex];

			Assert::IsTrue(parent.hasChildren && parent.childrenStartIndex == childIndex && parent.childrenEndIndex == childIndex);
			Assert::IsTrue(child.parentIndex == parentIndex);

			auto otherChild = GameObject::AddChild(a, parentIndex, GameObject());
			Assert::IsTrue(otherChild == 2);
			Assert::IsTrue(parent.childrenStartIndex == 1 && parent.childrenEndIndex == 2);
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
			Assert::IsTrue(parent.childrenStartIndex == 4 && parent.childrenEndIndex == 6);
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

		TEST_METHOD(AddChildren)
		{

		}
	};
}