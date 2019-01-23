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
	success &= InitTransformUpdateSystem();

	//TODO proper setup
	auto parentIndex = GameObject::AddToArena(*_gameObjects, GameObject());
	auto& testObject = (*_gameObjects)[parentIndex];

	testObject.isVisible = true;
	testObject.velocityPerSec = Sisu::Vector3(0.05, 0.0, 0.0);
	testObject.eulerRotPerSec = Sisu::Vector3(0.0, 0.0, 90.0);
	testObject.localScale = Sisu::Vector3(1.0, 1.0, 1.0);

	auto childIndex = GameObject::AddChild(*_gameObjects, parentIndex, GameObject());
	auto& child = (*_gameObjects)[childIndex];

	child.isVisible = true;
	child.localPosition = Sisu::Vector3(-2.0, 0.0, 0.0);
	child.eulerRotPerSec = Sisu::Vector3(0.0, 45.0, 0.0);
	child.localScale = Sisu::Vector3(0.5, 0.5, 0.5);
	child.color = Sisu::Color::Red();

	auto grandKidIndex = GameObject::AddChild(*_gameObjects, childIndex, GameObject());
	auto& grandKid = (*_gameObjects)[grandKidIndex];

	grandKid.isVisible = true;
	grandKid.localPosition = Sisu::Vector3(-2.0, 0.0, 0.0);
	grandKid.eulerRotPerSec = Sisu::Vector3(0.0, 22.5, 22.5);
	grandKid.localScale = Sisu::Vector3(0.5, 2.5, 0.5);
	grandKid.color = Sisu::Color::Black();

	return success;
}

bool SisuApp::InitArenas()
{
	_gameObjects = std::make_unique<Arena<GameObject>>(MaxGameObjectCount);
	return _gameObjects != nullptr;
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
												_gameObjects.get());
	return _renderer->Init();
}

bool SisuApp::InitTransformUpdateSystem()
{
	_transformUpdateSystem = std::make_unique<TransformUpdateSystem>();
	return _transformUpdateSystem != nullptr;
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
	const auto& gt = *_gameTimer;
	auto hasSomethingChanged = _transformUpdateSystem->Update(gt, *_gameObjects);

	if (hasSomethingChanged)
	{
		_renderer->SetDirty();
	}

	_renderer->Update(gt);
}

void SisuApp::Draw()
{
	_renderer->Draw(*_gameTimer.get());
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