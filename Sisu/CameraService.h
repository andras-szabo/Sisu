#pragma once
#include "ICameraService.h"

class GameTimer;
class IInputService;

class CameraService : public ICameraService
{
public:
	CameraService(IInputService* const inputService, WindowManager* const windowManager) :
		_inputService(inputService),
		_windowManager(windowManager)
	{
		D3DCamera defaultCamera;
		auto fullscreen = _windowManager->Dimensions();
		defaultCamera.SetViewport(Sisu::Vector4(0.0f, 0.0f, 1.0f, 1.0f),
								  static_cast<float>(fullscreen.first),
								  static_cast<float>(fullscreen.second),
								  0.0f, 1.0f);

		_cameras.push_back(defaultCamera);
	}

	D3DCamera* GetActiveCamera() override { return &_cameras[0]; }
	std::vector<D3DCamera> GetActiveCameras() const override { return _cameras; }

	virtual void Update(const GameTimer& gt) override;
	virtual void OnResize() override;
	virtual void SetCameras(const std::vector<D3DCamera> cameras) override;

private:
	IInputService* const _inputService;
	WindowManager* const _windowManager;
	std::vector<D3DCamera> _cameras;
};
