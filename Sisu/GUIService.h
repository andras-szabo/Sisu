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
		_camera.SetCameraIndex(0);
		OnResize();
	}

	virtual const D3DCamera& GetCamera() const override { return _camera; }
	virtual void OnResize() override;

private:
	IInputService* const _inputService;
	WindowManager* const _windowManager;
	D3DCamera _camera;
};
