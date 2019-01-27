#pragma once
#include "Camera.h"

class GameTimer;

class ICameraService
{
public:
	virtual D3DCamera* GetActiveCamera() = 0;
	virtual void Update(const GameTimer& gt) = 0;
	virtual void OnResize(float aspectRatio) = 0;
};
