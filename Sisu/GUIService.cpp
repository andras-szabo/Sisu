#pragma once
#include "stdafx.h"
#include "GUIService.h"
#include "Camera.h"
#include "UIElement.h"

std::size_t GUIService::CreateLetter(char character, Sisu::Vector3 position, Sisu::Vector3 localScale)
{
	static const std::string charTable =
		"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-()[]?!/*,_;:\"%&\\{}~<>|@^$.";

	auto dimensions = _windowManager->Dimensions();
	UIElement newElement(position, localScale, dimensions);

	//TODO into sep. function
	//How does this work? It assumes the character texture map to be 10x10,
	//containing the characters in the order laid out in charTable
	Sisu::Vector4 uvData(0.0f, 0.0f, 0.0f, 0.0f);

	if (!isspace(character))
	{
		auto charIndex = 0;
		auto found = false;
		for (; !found && charIndex < charTable.size(); ++charIndex)
		{
			found = charTable[charIndex] == character;
		}

		charIndex--;
		uvData.x = 0.1f * (charIndex % 10);
		uvData.y = 0.1f * (charIndex / 10);
		uvData.z = 0.1f;
		uvData.w = 0.1f;
	}

	newElement.uvData = uvData;

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
