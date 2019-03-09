#pragma once

enum class KeyCode : WPARAM
{
	A = 0x41,
	D = 0x44,
	E = 0x45,
	F = 0x46,
	Q = 0x51,
	R = 0x52,
	S = 0x53,
	U = 0x55,
	W = 0x57,

	One = 0x31,
	
	Shift = 0x10
};

class IInputService
{
public:
	struct Point
	{
		Point() : x(0.0f), y(0.0f) {}
		Point(float a, float b) : x(a), y(b) {}
		float x, y;
	};

	virtual bool GetKeyDown(KeyCode key) const = 0;
	virtual bool GetKeyUp(WPARAM key) const = 0;

	virtual bool GetKey(KeyCode key) const = 0;
	virtual bool GetMouseButton(int btn) const = 0;
	virtual Point GetMouseDelta() const = 0;

	virtual void OnMouseDown(WPARAM buttonState, int x, int y) = 0;
	virtual void OnMouseUp(WPARAM buttonState, int x, int y) = 0;
	virtual void OnMouseMove(WPARAM buttonState, int x, int y) = 0;

	virtual void OnKeyDown(WPARAM virtualKeyCode) = 0;
	virtual void OnKeyUp(WPARAM virtualKeyCode) = 0;

	virtual void PostDraw() = 0;
};
