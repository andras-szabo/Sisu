#pragma once
#include "Camera.h"
#include <vector>

class GameTimer;

class ICameraService
{
public:
	virtual D3DCamera* GetActiveCamera() = 0;
	virtual void Update(const GameTimer& gt) = 0;
	virtual void OnResize() = 0;
	virtual std::vector<D3DCamera> GetActiveCameras() const = 0;
	virtual void SetCameras(const std::vector<D3DCamera> cameras) = 0;
	virtual std::size_t MaxCameraCount() const = 0;
};
