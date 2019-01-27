#pragma once
#include "IInputService.h"
#include "GameTimer.h"

class InputService : public IInputService
{
public:
	InputService(GameTimer* const gt)
		: _gameTimer(gt)
	{
	}

	virtual void OnMouseDown(WPARAM buttonState, int x, int y) override {}
	virtual void OnMouseUp(WPARAM buttonState, int x, int y) override {}
	virtual void OnMouseMove(WPARAM buttonState, int x, int y) override {}

	virtual void OnKeyDown(WPARAM virtualKeyCode) override {};
	virtual void OnKeyUp(WPARAM virtualKeyCode) override {};

private:
	GameTimer* const _gameTimer;
};
