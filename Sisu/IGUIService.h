#pragma once
#include "SisuUtilities.h"
#include "UIElement.h"

class D3DCamera;
class GameTimer;

class IGUIService
{
public:
	virtual ~IGUIService() {}

	virtual void OnResize() = 0;
	virtual void Update(const GameTimer& gt) = 0;

	//For testing
	virtual std::size_t CreateUIElement(Sisu::Vector3 position, Sisu::Vector3 localScale) = 0;
	virtual std::size_t CreateLetter(char character, Sisu::Vector3 position, Sisu::Vector3 localScale) = 0;
};
