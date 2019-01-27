#include "stdafx.h"
#include "CameraService.h"
#include "IInputService.h"

void CameraService::OnResize(float aspectRatio)
{
	for (auto& camera : _cameras)
	{
		camera.OnResize(aspectRatio);
	}
}

void CameraService::Update(const GameTimer& gt)
{
	static Sisu::Vector3 inputAxes;
	static Sisu::Vector3 inputEuler;

	auto a = _inputService->GetKey(KeyCode::A);
	auto d = _inputService->GetKey(KeyCode::D);
	auto s = _inputService->GetKey(KeyCode::S);
	auto w = _inputService->GetKey(KeyCode::W);
	auto q = _inputService->GetKey(KeyCode::Q);
	auto e = _inputService->GetKey(KeyCode::E);
	auto r = _inputService->GetKey(KeyCode::R);
	auto f = _inputService->GetKey(KeyCode::F);

	inputAxes.x = (a ? -1 : 0) + (d ? 1 : 0);
	inputAxes.z = (s ? -1 : 0) + (w ? 1 : 0);
	inputAxes.y = (f ? -1 : 0) + (r ? 1 : 0);

	inputEuler.z = (e ? -1.0 : 0) + (q ? 1.0 : 0);

	if (_inputService->GetMouseButton(0))
	{
		auto mouseDelta = _inputService->GetMouseDelta();
		inputEuler.y = mouseDelta.x;
		inputEuler.x = mouseDelta.y;
	}
	else
	{
		inputEuler.x = 0.0f;
		inputEuler.y = 0.0f;
	}

	_cameras[0].Update(gt, inputAxes, inputEuler);
}