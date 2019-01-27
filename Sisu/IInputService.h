#pragma once

class IInputService
{
public:
	virtual bool GetKeyDown(WPARAM key) const = 0;
	virtual bool GetKeyUp(WPARAM key) const = 0;
	virtual bool GetKey(WPARAM key) const = 0;

	virtual void OnMouseDown(WPARAM buttonState, int x, int y) = 0;
	virtual void OnMouseUp(WPARAM buttonState, int x, int y) = 0;
	virtual void OnMouseMove(WPARAM buttonState, int x, int y) = 0;

	virtual void OnKeyDown(WPARAM virtualKeyCode) = 0;
	virtual void OnKeyUp(WPARAM virtualKeyCode) = 0;
};
