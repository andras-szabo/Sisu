#pragma once
#include <stack>
#include "IGUIService.h"
#include "Camera.h"
#include "ICameraService.h"

class GUIService : public IGUIService
{
public:
	GUIService(IInputService* const inputService, WindowManager* const windowManager, 
			   ICameraService* const camService,
			   IRenderer* const renderer):
		_inputService(inputService), 
		_windowManager(windowManager),
		_cameraService(camService),
		_renderer(renderer)
	{
		D3DCamera camera(0.0f, 0.0f, 0.0f);

		camera.isPerspective = false;
		camera.isScreenSpaceUI = true;

		auto fullscreen = _windowManager->Dimensions();
		camera.SetViewport(Sisu::Vector4(0.0f, 0.0f, 1.0f, 1.0f),
			static_cast<float>(fullscreen.first),
			static_cast<float>(fullscreen.second),
			0.0f, 1.0f);

		camera.clearDepthOnly = true;
		camera.SetCameraIndex(0);

		_cameraService->CreateGUICamera(camera);
	}

	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual std::size_t CreateUIElement(Sisu::Vector3 position, Sisu::Vector3 localScale) override;

private:
	IInputService* const _inputService;
	ICameraService* const _cameraService;
	WindowManager* const _windowManager;
	IRenderer* const _renderer;

	std::vector<UIElement> _uiElements;
	std::stack<std::size_t> _freeUIElementPositions;
};
