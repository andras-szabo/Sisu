#pragma once
#include <cstddef>
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

		return 0;
	}

	//TODO: make sure we know when we're making copies
	GameObject() = default;
	GameObject(const GameObject& other) = default;

public:
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