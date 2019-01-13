#pragma once
class GameTimer;

struct IRenderer
{
	virtual bool IsSetup() const = 0;
	virtual bool Init() = 0;
	virtual void OnResize() = 0;
	virtual void PreDraw() = 0;
	virtual void Draw(GameTimer* gt) = 0;
};
