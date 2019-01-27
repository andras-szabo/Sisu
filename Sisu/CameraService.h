#pragma once
#include "ICameraService.h"

class GameTimer;
class IInputService;

class CameraService : public ICameraService
{
public:
	CameraService(IInputService* const inputService):
		_inputService(inputService) 
	{
		_cameras.push_back(D3DCamera());
	}

	D3DCamera* GetActiveCamera() override { return &_cameras[0]; }
	virtual void Update(const GameTimer& gt) override;
	virtual void OnResize(float aspectRatio) override;

private:
	IInputService* const _inputService;
	std::vector<D3DCamera> _cameras;
};
