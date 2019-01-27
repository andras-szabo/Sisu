#pragma once
#include "IInputService.h"
#include "GameTimer.h"
#include <vector>

class InputService : public IInputService
{
public:
	InputService(GameTimer* const gt)
		: _gameTimer(gt)
	{
		std::size_t itemCount = 255;
		unsigned long itemValue = 0;

		_keyPressFrame = std::vector<unsigned long>(itemCount, itemValue);
		_keyReleaseFrame = std::vector<unsigned long>(itemCount, itemValue);
	}

	virtual bool GetKeyDown(WPARAM key) const override;
	virtual bool GetKeyUp(WPARAM key) const override;
	virtual bool GetKey(WPARAM key) const override;

	virtual void OnMouseDown(WPARAM buttonState, int x, int y) override {}
	virtual void OnMouseUp(WPARAM buttonState, int x, int y) override {}
	virtual void OnMouseMove(WPARAM buttonState, int x, int y) override {}

	virtual void OnKeyDown(WPARAM virtualKeyCode) override;
	virtual void OnKeyUp(WPARAM virtualKeyCode) override;

private:
	GameTimer* const _gameTimer;
	std::vector<unsigned long> _keyPressFrame;
	std::vector<unsigned long> _keyReleaseFrame;
};
