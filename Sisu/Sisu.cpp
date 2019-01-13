#include "stdafx.h"
#include "Sisu.h"
#include "D3DRenderer.h"

bool SisuApp::Init(int width, int height, const std::wstring& title)
{
	auto success = true;

	success &= InitGameTimer();
	success &= InitWindowManager(width, height, title);
	success &= InitRenderer();

	return success;
}

bool SisuApp::InitGameTimer()
{
	_gameTimer = std::make_unique<GameTimer>();
	return _gameTimer != nullptr;
}

bool SisuApp::InitWindowManager(int width, int height, const std::wstring& title)
{
	_windowManager = std::make_unique<WindowManager>(*this, width, height, title);
	return _windowManager->IsSetup();
}

bool SisuApp::InitRenderer()
{
	_renderer = std::make_unique<D3DRenderer>(_windowManager.get(), _gameTimer.get());
	return _renderer->Init();
}