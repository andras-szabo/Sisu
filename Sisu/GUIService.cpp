#pragma once
#include "stdafx.h"
#include "GUIService.h"
#include "Camera.h"
#include "UIElement.h"

std::size_t GUIService::CreateUIElement(Sisu::Vector3 position, Sisu::Vector3 localScale)
{
	UIElement newElement;
	newElement.position = position;
	newElement.scale = localScale;

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

	_renderer->AddUIRenderItem(_uiElements[index]);

	return index;
}

void GUIService::OnResize()
{
}

void GUIService::Update(const GameTimer& gt)
{
}
