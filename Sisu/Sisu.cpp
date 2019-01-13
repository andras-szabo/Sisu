#include "stdafx.h"
#include "Sisu.h"
#include "BrickRenderer.h"

bool SisuApp::Init(int width, int height, const std::wstring& title)
{
	auto success = true;

	success &= InitArenas();
	success &= InitGameTimer();
	success &= InitWindowManager(width, height, title);
	success &= InitRenderer();

	return success;
}

bool SisuApp::InitArenas()
{
	_gameObjects = std::make_unique<Arena<GameObject>>(MaxGameObjectCount);
	_bricks = std::make_unique<Arena<Brick>>(MaxBrickCount);

	return _gameObjects != nullptr && _bricks != nullptr;
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
	_renderer = std::make_unique<BrickRenderer>(_windowManager.get(), 
												_gameTimer.get(),
												_gameObjects.get(),
												_bricks.get());
	return _renderer->Init();
}

int SisuApp::Run()
{
	MSG msg = { 0 };
	_gameTimer->Reset();

	std::clog << "Starting game loop.\n";

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			_gameTimer->Tick();
			if (!_gameTimer->IsPaused())
			{
				CalculateFrameStats();
				Update();
				Draw();
			}
			else
			{
				Sleep(100);		// sleep for 100 ms
			}
		}
	}

	return (int)msg.wParam;
}

void SisuApp::Update()
{
}

void SisuApp::Draw()
{
	_renderer->Draw(_gameTimer.get());
}

void SisuApp::Pause(bool newState)
{
	if (newState) { _gameTimer->Pause(); }
	else { _gameTimer->Unpause(); }
}

void SisuApp::OnResize()
{
	if (_renderer != nullptr)
	{
		_renderer->OnResize();
	}
}

void SisuApp::CalculateFrameStats()
{
	static int frameCount = 0;
	static float elapsedTime = 0.0f;

	frameCount++;
	if (_gameTimer->SecondsSinceReset() - elapsedTime >= 1.0f)
	{
		auto fps = (float)frameCount;
		auto msPerFrame = 1000.0f / fps;
		auto fpsAsString = std::to_wstring(fps);
		auto msPerFrameAsString = std::to_wstring(msPerFrame);
		_windowManager->SetText(L"      fps: " + fpsAsString + L"    ms/frame: " + msPerFrameAsString);

		frameCount = 0;
		elapsedTime += 1.0f;
	}
}