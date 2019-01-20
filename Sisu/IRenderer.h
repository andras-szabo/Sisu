#pragma once
class GameTimer;

struct IRenderer
{
	virtual bool IsSetup() const = 0;
	virtual bool Init() = 0;
	virtual void OnResize() = 0;
	virtual void Update(const GameTimer& gt) = 0;
	virtual void Draw(const GameTimer& gt) = 0;
};
