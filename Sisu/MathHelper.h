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

	static float Clamp(float value, float min, float max)
	{
		return value < min ? min : (value > max ? max : value);
	}

	static const float Pi;
};