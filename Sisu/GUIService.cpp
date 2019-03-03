#pragma once
#include "stdafx.h"
#include "GUIService.h"
#include "Camera.h"

void GUIService::OnResize()
{
	auto dimensions = _windowManager->Dimensions();
	_camera.OnResize(dimensions.first, dimensions.second);
}

void GUIService::Update(const GameTimer& gt)
{
	static const Sisu::Vector3 inputAxes;
	static const Sisu::Vector3 inputEuler;

	_camera.Update(gt, inputAxes, inputEuler);
}
