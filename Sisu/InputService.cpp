#include "stdafx.h"
#include "InputService.h"

void InputService::OnKeyDown(WPARAM virtualKeyCode)
{
	_keyPressFrame[virtualKeyCode] = _gameTimer->FrameCount();
}

void InputService::OnKeyUp(WPARAM virtualKeyCode)
{
	_keyReleaseFrame[virtualKeyCode] = _gameTimer->FrameCount();
}

bool InputService::GetKey(WPARAM virtualKeyCode) const
{
	return _keyPressFrame[virtualKeyCode] > _keyReleaseFrame[virtualKeyCode];
}

bool InputService::GetKeyDown(WPARAM virtualKeyCode) const
{
	return _keyPressFrame[virtualKeyCode] == _gameTimer->FrameCount();
}

bool InputService::GetKeyUp(WPARAM virtualKeyCode) const
{
	return _keyReleaseFrame[virtualKeyCode] == _gameTimer->FrameCount();
}
