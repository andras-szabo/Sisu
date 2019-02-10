#pragma once
#include "stdafx.h"
#include "GUIService.h"
#include "Camera.h"

void GUIService::OnResize()
{
	auto dimensions = _windowManager->Dimensions();
	_camera.OnResize(dimensions.first, dimensions.second);
}
