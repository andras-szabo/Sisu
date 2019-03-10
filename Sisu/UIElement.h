#pragma once
#include "SisuUtilities.h"

struct UIElement
{
	UIElement(const Sisu::Vector3& pos, const Sisu::Vector3& sca, const std::pair<int, int>& windowDimensions):
		position(pos), scale(sca)
	{
		auto windowWidth = static_cast<float>(windowDimensions.first);
		auto windowHeight = static_cast<float>(windowDimensions.second);

		_pixelPosition.x = windowWidth * position.x;
		_pixelPosition.y = windowHeight * position.y;

		_pixelDimensions.x = windowWidth * scale.x;
		_pixelDimensions.y = windowHeight * scale.y;
	}

	void OnResize(float windowWidth, float windowHeight)
	{
		position.x = _pixelPosition.x / windowWidth;
		position.y = _pixelPosition.y / windowHeight;

		scale.x = _pixelDimensions.x / windowWidth;
		scale.y = _pixelDimensions.y / windowHeight;
	}

	std::size_t renderItemIndex = 0;
	Sisu::Vector3 position;
	Sisu::Vector3 scale;
	Sisu::Vector4 uvData;

private:
	Sisu::Vector3 _pixelPosition;
	Sisu::Vector3 _pixelDimensions;
};
