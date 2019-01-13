#pragma once

#include "resource.h"
#include <string>
#include "WindowManager.h"
#include "GameTimer.h"
#include "IRenderer.h"

class SisuApp
{
	friend class WindowManager;
	friend class std::unique_ptr<SisuApp>;

public:
	SisuApp(SisuApp&& other) = default;
	SisuApp& operator=(SisuApp&& other) = default;

	virtual ~SisuApp() {}

protected:
	SisuApp(HINSTANCE hInstance) : _hAppInstance(hInstance) {}
	SisuApp(const SisuApp&) = delete;				// copy ctor, copy assignment: not allowed
	SisuApp& operator=(const SisuApp&) = delete;

public:
	HINSTANCE GetAppInstanceHandle() const { return _hAppInstance; }
	HWND MainWindowHandle() const { return _windowManager->MainWindowHandle(); }
	float AspectRatio() const { return _windowManager->AspectRatio(); }
	bool IsRendererSetup() const { return _renderer->IsSetup(); }

	virtual bool Init(int width, int height, const std::wstring& title);
	virtual void Pause(bool newState);

protected:
	virtual void OnResize();
	virtual void Update(const GameTimer& gameTimer) = 0;
	virtual void Draw(const GameTimer& gameTimer) = 0;

	virtual void OnMouseDown(WPARAM buttonState, int x, int y) {}
	virtual void OnMouseUp(WPARAM buttonState, int x, int y) {}
	virtual void OnMouseMove(WPARAM buttonState, int x, int y) {}

	virtual void OnKeyDown(WPARAM virtualKeyCode) {}
	virtual void OnKeyUp(WPARAM virtualKeyCode) {}

	bool InitGameTimer();
	bool InitWindowManager(int width, int height, const std::wstring& title);
	bool InitRenderer();

protected:
	HINSTANCE _hAppInstance = nullptr;

	std::unique_ptr<GameTimer> _gameTimer;
	std::unique_ptr<WindowManager> _windowManager;
	std::unique_ptr<IRenderer> _renderer;

	bool _isRendererSetup;
};
