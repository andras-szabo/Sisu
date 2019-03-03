#pragma once
#include <DirectXMath.h>

class MathHelper
{
public:
	static DirectX::XMFLOAT4X4 Identity4x4()
	{
		static DirectX::XMFLOAT4X4 I{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		return I;
	}

	// Creates a projection matrix such that
	// you can define UI in the (0,1) interval
	// coordinates, where (0,0) is the top left
	// of the screen, and (1,1) is bottom right.
	static DirectX::XMFLOAT4X4 ScreenSpaceUIProj()
	{
		static DirectX::XMFLOAT4X4 P{
			2.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -2.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 1.0f
		};

		return P;
	}

	static float Clamp(float value, float min, float max)
	{
		return value < min ? min : (value > max ? max : value);
	}

	static const float Pi;
};