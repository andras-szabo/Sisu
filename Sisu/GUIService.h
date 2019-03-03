#pragma once
#include "IGUIService.h"
#include "Camera.h"

class GUIService : public IGUIService
{
public:
	GUIService(IInputService* const inputService, WindowManager* const windowManager):
		_inputService(inputService), _windowManager(windowManager)
	{
		_camera.isPerspective = false;

		auto fullscreen = _windowManager->Dimensions();
		_camera.SetViewport(Sisu::Vector4(0.0f, 0.0f, 1.0f, 1.0f),
			static_cast<float>(fullscreen.first),
			static_cast<float>(fullscreen.second),
			0.0f, 1.0f);

		_camera.clearDepthOnly = true;
		_camera.SetCameraIndex(0);

		OnResize();
	}

	virtual const D3DCamera& GetCamera() const override { return _camera; }
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;

private:
	IInputService* const _inputService;
	WindowManager* const _windowManager;
	D3DCamera _camera;
};
