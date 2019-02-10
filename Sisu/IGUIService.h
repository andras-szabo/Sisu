#pragma once

class D3DCamera;

class IGUIService
{
public:
	virtual const D3DCamera& GetCamera() const = 0;
	virtual void OnResize() = 0;
};
