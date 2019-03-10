#include "stdafx.h"
#include "Sisu.h"
#include "BrickRenderer.h"
#include "InputService.h"
#include "CameraService.h"
#include "GUIService.h"

bool SisuApp::Init(int width, int height, const std::wstring& title)
{
	auto success = true;

	success &= InitArenas();

	success &= InitGameTimer();
	success &= InitInputService(_gameTimer.get());
	success &= InitWindowManager(_inputService.get(), width, height, title);
	success &= InitCameraService(_inputService.get(), _windowManager.get());
	success &= InitRenderer(_windowManager.get(), _gameTimer.get(), _gameObjects.get(), _cameraService.get());
	success &= InitGUIService(_inputService.get(), _windowManager.get(), _cameraService.get(), _renderer.get());
	success &= InitTransformUpdateSystem();

	// TODO - also, proper setup
	auto w = static_cast<float>(width);
	auto h = static_cast<float>(height);

	D3DCamera topLeft;		topLeft.SetViewport(Sisu::Vector4(0.0f, 0.0f, 0.5f, 0.5f), w, h, 0.0f, 1.0f);
	topLeft.isPerspective = true;

	D3DCamera topRight;		topRight.SetViewport(Sisu::Vector4(0.5f, 0.0f, 0.5f, 0.5f), w, h, 0.0f, 1.0f);
	topRight.SetPosition(Sisu::Vector3(0.0f, 12.0f, 0.0f));
	topRight.SetRotation(Sisu::Vector3(90.0f, 0.0f, 0.0f));
	topRight.SetClearColor(DirectX::Colors::CornflowerBlue);

	D3DCamera bottomRight;	bottomRight.SetViewport(Sisu::Vector4(0.5f, 0.5f, 0.5f, 0.5f), w, h, 0.0f, 1.0f);
	bottomRight.SetPosition(Sisu::Vector3(0.0f, 0.0f, -12.0f));
	bottomRight.SetClearColor(DirectX::Colors::Azure);

	D3DCamera bottomLeft;	bottomLeft.SetViewport(Sisu::Vector4(0.0f, 0.5f, 0.5f, 0.5f), w, h, 0.0f, 1.0f);
	bottomLeft.SetPosition(Sisu::Vector3(-12.0f, 0.0f, 0.0f));
	bottomLeft.SetRotation(Sisu::Vector3(0.0f, 90.0f, 0.0f));
	bottomLeft.SetClearColor(DirectX::Colors::AliceBlue);

	std::vector<D3DCamera> cameras{ topLeft, topRight, bottomLeft, bottomRight };
	_cameraService->SetCameras(cameras);

	//TODO proper setup
	auto yetAnotherCubeIndex = GameObject::AddToArena(*_gameObjects, GameObject());
	auto& yac = (*_gameObjects)[yetAnotherCubeIndex];

	yac.isVisible = true;
	yac.eulerRotPerSec = Sisu::Vector3(0.5, 0.5, 0.5);
	yac.localScale = Sisu::Vector3(10.0f, 10.0f, 2.0f);
	yac.localPosition = Sisu::Vector3(5.0f, 0.0f, 0.0f);
	yac.color = Sisu::Color::Green();
	yac.borderColor = Sisu::Color::Blue();

	auto parentIndex = GameObject::AddToArena(*_gameObjects, GameObject());
	auto& testObject = (*_gameObjects)[parentIndex];

	testObject.isVisible = true;
	testObject.velocityPerSec = Sisu::Vector3(0.0, 0.0, 0.0);
	testObject.eulerRotPerSec = Sisu::Vector3(0.0, 0.0, 90.0);
	testObject.localScale = Sisu::Vector3(1.0, 1.0, 1.0);
	testObject.borderColor = Sisu::Color::White();

	auto childIndex = GameObject::AddChild(*_gameObjects, parentIndex, GameObject());
	auto& child = (*_gameObjects)[childIndex];

	child.isVisible = true;
	child.localPosition = Sisu::Vector3(-2.0, 0.0, 0.0);
	child.eulerRotPerSec = Sisu::Vector3(0.0, 45.0, 0.0);
	child.localScale = Sisu::Vector3(1.0, 0.5, 1.0);
	child.color = Sisu::Color::Red();
	child.borderColor = Sisu::Color::Black();

	auto grandKidIndex = GameObject::AddChild(*_gameObjects, childIndex, GameObject());
	auto& grandKid = (*_gameObjects)[grandKidIndex];

	grandKid.isVisible = true;
	grandKid.localPosition = Sisu::Vector3(-2.0, 0.0, 0.0);
	grandKid.eulerRotPerSec = Sisu::Vector3(0.0, 22.5, 22.5);
	grandKid.localScale = Sisu::Vector3(0.5, 2.5, 0.5);
	grandKid.color = Sisu::Color::Black();
	grandKid.borderColor = Sisu::Color::Yellow();

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

bool SisuApp::InitInputService(GameTimer* const gt)
{
	_inputService = std::make_unique<InputService>(gt);
	return _inputService != nullptr;
}

bool SisuApp::InitCameraService(IInputService* const inputService, WindowManager* const windowManager)
{
	_cameraService = std::make_unique<CameraService>(inputService, windowManager);
	return _cameraService != nullptr;
}

bool SisuApp::InitRenderer(WindowManager* const windowManager, GameTimer* const gt,
	Arena<GameObject>* const arena, ICameraService* const camService)
{
	_renderer = std::make_unique<BrickRenderer>(windowManager, gt, arena, camService);
	return _renderer->Init();
}

bool SisuApp::InitGUIService(IInputService * const inputService, WindowManager * const windowManager, 
							 ICameraService* const camService, IRenderer* const renderer)
{
	_gui = std::make_unique<GUIService>(inputService, windowManager, camService, renderer);
	return _gui != nullptr;
}

bool SisuApp::InitWindowManager(IInputService* const inputService, int width, int height, const std::wstring& title)
{
	_windowManager = std::make_unique<WindowManager>(*this, inputService, width, height, title);
	return _windowManager->IsSetup();
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
				PostDraw();	//TODO better name
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
	_renderer->SetWireframe(_inputService->GetKey(KeyCode::One));

	//TODO - move to its proper lace
	if (_inputService->GetKeyDown(KeyCode::U))
	{
		auto x = rand() % 50 * 0.01f;
		auto y = rand() % 50 * 0.01f;
		auto w = rand() % 50 * 0.01f; if (w <= 0.0f) { w = 0.1f; }
		auto h = rand() % 50 * 0.01f; if (h <= 0.0f) { h = 0.1f; }

		_gui->CreateUIElement(Sisu::Vector3(x, y, 0.0f), Sisu::Vector3(w, h, 1.0f));
	}
}

void SisuApp::Draw()
{
	_renderer->Draw(*_gameTimer.get());
}

void SisuApp::PostDraw()
{
	_inputService->PostDraw();
}

void SisuApp::Pause(bool newState)
{
	if (newState) { _gameTimer->Pause(); }
	else { _gameTimer->Unpause(); }
}

void SisuApp::OnResize()
{
	if (_gui != nullptr)
	{
		_gui->OnResize();
	}

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