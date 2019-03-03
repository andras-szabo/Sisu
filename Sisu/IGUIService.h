#pragma once

class D3DCamera;
class GameTimer;

class IGUIService
{
public:
	virtual const D3DCamera& GetCamera() const = 0;
	virtual void OnResize() = 0;
	virtual void Update(const GameTimer& gt) = 0;
};
