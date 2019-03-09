#include "stdafx.h"
#include "InputService.h"

void InputService::OnMouseMove(WPARAM buttonState, int x, int y)
{
	_currentMousePos.x = x;
	_currentMousePos.y = y;
}

void InputService::ResetMouseDelta()
{
	_lastMousePos.x = _currentMousePos.x;
	_lastMousePos.y = _currentMousePos.y;
}

IInputService::Point InputService::GetMouseDelta() const
{
	return Point(_currentMousePos.x - _lastMousePos.x,
				 _currentMousePos.y - _lastMousePos.y);
}

bool InputService::GetMouseButton(int btn) const
{
	return _mouseButtonPressFrame[btn] > _mouseButtonReleaseFrame[btn];
}

void InputService::OnKeyDown(WPARAM virtualKeyCode)
{
	_keyPressFrame[virtualKeyCode] = _gameTimer->FrameCount();
}

void InputService::OnKeyUp(WPARAM virtualKeyCode)
{
	_keyReleaseFrame[virtualKeyCode] = _gameTimer->FrameCount();
}

bool InputService::GetKey(KeyCode key) const
{
	auto virtualKeyCode = static_cast<WPARAM>(key);
	return _keyPressFrame[virtualKeyCode] > _keyReleaseFrame[virtualKeyCode];
}

bool InputService::GetKeyDown(KeyCode key) const
{
	auto virtualKeyCode = static_cast<WPARAM>(key);
	auto frameCount = _gameTimer->FrameCount();
	return _keyPressFrame[virtualKeyCode] == frameCount - 1;
}

bool InputService::GetKeyUp(WPARAM virtualKeyCode) const
{
	return _keyReleaseFrame[virtualKeyCode] == _gameTimer->FrameCount();
}

void InputService::OnMouseDown(WPARAM buttonState, int x, int y)
{
	_mouseButtonPressFrame[0] = _gameTimer->FrameCount();
}

void InputService::OnMouseUp(WPARAM buttonState, int x, int y)
{
	_mouseButtonReleaseFrame[0] = _gameTimer->FrameCount();
}
