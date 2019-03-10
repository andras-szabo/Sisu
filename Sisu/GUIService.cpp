#pragma once
#include "stdafx.h"
#include "GUIService.h"
#include "Camera.h"
#include "UIElement.h"

std::size_t GUIService::CreateUIElement(Sisu::Vector3 position, Sisu::Vector3 localScale)
{
	auto dimensions = _windowManager->Dimensions();
	UIElement newElement(position, localScale, dimensions);

	std::size_t index = 0;
	if (_freeUIElementPositions.empty())
	{
		index = _uiElements.size();
		_uiElements.emplace_back(newElement);
	}
	else
	{
		index = _freeUIElementPositions.top();
		_freeUIElementPositions.pop();
		_uiElements[index] = newElement;
	}

	auto renderItemIndex = _renderer->AddUIRenderItem(_uiElements[index]);
	_uiElements[index].renderItemIndex = renderItemIndex;

	return index;
}

void GUIService::OnResize()
{
	auto dimensions = _windowManager->Dimensions();
	auto width = static_cast<float>(dimensions.first);
	auto height = static_cast<float>(dimensions.second);

	for (auto& uiElement : _uiElements)
	{
		uiElement.OnResize(width, height);
		_renderer->RefreshUIItem(uiElement);
	}
}

void GUIService::Update(const GameTimer& gt)
{
}
