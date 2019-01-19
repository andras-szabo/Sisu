#pragma once
#include <cstddef>
#include <vector>
#include "SisuUtilities.h"
#include "Arena.h"

class GameObject
{
public:
	static std::size_t AddToArena(Arena<GameObject>& arena, GameObject go)
	{
		auto index = arena.AddAnywhere(go);
		arena[index].index = index;
		return index;
	}

	static std::size_t AddChildren(Arena<GameObject>& arena, std::size_t parentIndex,
		std::vector<GameObject>::iterator begin,
		std::vector<GameObject>::iterator end)
	{
		std::size_t newChildrenCount = end - begin;
		if (!arena[parentIndex].hasChildren)
		{
			auto firstChildIndex = arena.GetStartIndexForGap(newChildrenCount, parentIndex);
			arena.AddAt(firstChildIndex, begin, end);

			arena[parentIndex].hasChildren = true;
			arena[parentIndex].childrenStartIndex = firstChildIndex;
			arena[parentIndex].childrenEndIndex = firstChildIndex + newChildrenCount - 1;
			
			for (auto i = 0; i < newChildrenCount; ++i)
			{
				arena[firstChildIndex + i].index = firstChildIndex + i;
				arena[firstChildIndex + i].parentIndex = parentIndex;
			}

			return firstChildIndex;
		}

		// Check the slot after the first kids; if OK, put them there.
		if (arena.CanAddItemsAt(arena[parentIndex].childrenEndIndex + 1, newChildrenCount))
		{
			auto firstChildIndex = arena[parentIndex].childrenEndIndex + 1;
			arena.AddAt(firstChildIndex, begin, end);

			arena[parentIndex].hasChildren = true;
			arena[parentIndex].childrenEndIndex = firstChildIndex + newChildrenCount - 1;

			for (auto i = 0; i < newChildrenCount; ++i)
			{
				arena[firstChildIndex + i].index = firstChildIndex + i;
				arena[firstChildIndex + i].parentIndex = parentIndex;
			}

			return arena[parentIndex].childrenStartIndex;
		}

		// So now we need to relocate.
		// TODO: DRY
		auto& parent = arena[parentIndex];
		auto existingKidCount = parent.childrenEndIndex - parent.childrenStartIndex + 1;
		auto childrenCount = existingKidCount + newChildrenCount;
		auto gapStartIndex = arena.GetStartIndexForGap(childrenCount, parentIndex);

		// First copy the existing kids and update their indices
		auto fromIndex = parent.childrenStartIndex;
		for (std::size_t i = 0; i < existingKidCount; ++i)
		{
			auto newIndex = gapStartIndex + i;
			arena.AddAt(newIndex, arena[fromIndex + i]);
			arena[newIndex].index = newIndex;
			if (arena[newIndex].hasChildren)
			{
				for (std::size_t j = arena[newIndex].childrenStartIndex; j <= arena[newIndex].childrenEndIndex; ++j)
				{
					arena[j].parentIndex = newIndex;
				}
			}
		}

		arena.RemoveAt(fromIndex, existingKidCount);

		// Then add the new kids
		auto firstChildIndex = gapStartIndex + existingKidCount;
		arena.AddAt(firstChildIndex, begin, end);	
		
		for (auto i = 0; i < newChildrenCount; ++i)
		{
			arena[firstChildIndex + i].index = firstChildIndex + i;
			arena[firstChildIndex + i].parentIndex = parentIndex;
		}
	
		arena[parentIndex].childrenStartIndex = gapStartIndex;
		arena[parentIndex].childrenEndIndex = gapStartIndex + childrenCount - 1;

		return arena[parentIndex].childrenStartIndex;
	}

	static std::size_t AddChild(Arena<GameObject>& arena, std::size_t parentIndex, GameObject child)
	{
		if (!arena[parentIndex].hasChildren)
		{
			auto childIndex = arena.GetStartIndexForGap(1, parentIndex);
			arena.AddAt(childIndex, child);
			arena[childIndex].index = childIndex;

			auto& parent = arena[parentIndex];

			parent.hasChildren = true;
			parent.childrenStartIndex = childIndex;
			parent.childrenEndIndex = childIndex;

			arena[childIndex].parentIndex = parentIndex;

			return childIndex;
		}

		// Check the slot after last child of parent. If empty,
		// push the thing there.
		if (arena.CanAddItemAt(arena[parentIndex].childrenEndIndex + 1))
		{
			auto childIndex = arena[parentIndex].childrenEndIndex + 1;
			arena.AddAt(childIndex, child);
			arena[childIndex].index = childIndex;
			arena[parentIndex].childrenEndIndex = childIndex;
			arena[childIndex].parentIndex = parentIndex;
			return childIndex;
		}

		// We have to relocate all kids, including the new one
		auto& parent = arena[parentIndex];
		auto existingKidCount = parent.childrenEndIndex - parent.childrenStartIndex + 1;
		auto childrenCount = existingKidCount + 1;
		auto gapStartIndex = arena.GetStartIndexForGap(childrenCount, parentIndex);

		// First copy the existing kids and update their indices
		auto fromIndex = parent.childrenStartIndex;
		for (std::size_t i = 0; i < existingKidCount; ++i)
		{
			auto newIndex = gapStartIndex + i;
			arena.AddAt(newIndex, arena[fromIndex + i]);
			arena[newIndex].index = newIndex;
			if (arena[newIndex].hasChildren)
			{
				for (std::size_t j = arena[newIndex].childrenStartIndex; j <= arena[newIndex].childrenEndIndex; ++j)
				{
					arena[j].parentIndex = newIndex;
				}
			}
		}

		arena.RemoveAt(fromIndex, existingKidCount);

		// Then add the new kid
		auto newKidsIndex = gapStartIndex + existingKidCount;
		arena.AddAt(newKidsIndex, child);
		arena[newKidsIndex].index = newKidsIndex;
		arena[newKidsIndex].parentIndex = parentIndex;

		// Then set up the parent
		arena[parentIndex].childrenStartIndex = gapStartIndex;
		arena[parentIndex].childrenEndIndex = newKidsIndex;

		return newKidsIndex;
	}

	//TODO: make sure we know when we're making copies
	GameObject() = default;
	GameObject(const GameObject& other) = default;

public:
	//TODO - do we actually need index here? why?
	std::size_t index;
	std::size_t childrenStartIndex, childrenEndIndex;
	std::size_t parentIndex;

	bool isRoot;
	bool hasChildren;
	bool isVisible;

	Sisu::Vector3 localPosition;
	Sisu::Vector3 localRotation;
	Sisu::Vector3 localScale;

	Sisu::Color color;
	Sisu::Matrix4 transform;
};