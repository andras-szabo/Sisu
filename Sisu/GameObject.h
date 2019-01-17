#pragma once
#include <cstddef>
#include "SisuUtilities.h"

class GameObject
{
public:
	std::size_t brickIndex;
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