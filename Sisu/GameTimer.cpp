#include "stdafx.h"
#include "GameTimer.h"

unsigned long GameTimer::FrameCount() const
{
	return _frameCount;
}

float GameTimer::DeltaTimeSeconds() const
{
	return _deltaTimeSeconds;
}

float GameTimer::SecondsSinceReset() const
{
	return _secondsSinceReset;
}

bool GameTimer::IsPaused() const
{
	return _isPaused;
}

void GameTimer::Reset()
{
	TimePoint now = std::chrono::steady_clock::now();
	_previousTimeStamp = now;
	_secondsSinceReset = 0.0f;
	_frameCount = 0;
	_isPaused = false;
}

void GameTimer::Pause()
{
	if (!_isPaused)
	{
		_isPaused = true;
	}
}

void GameTimer::Unpause()
{
	if (_isPaused)
	{
		_isPaused = false;
	}
}

void GameTimer::Tick()
{
	if (_isPaused)
	{
		_deltaTimeSeconds = 0.0f;
		return;
	}

	TimePoint now = std::chrono::steady_clock::now();
	DurationInSeconds elapsed = now - _previousTimeStamp;

	_deltaTimeSeconds = elapsed.count();
	_previousTimeStamp = now;
	_secondsSinceReset += _deltaTimeSeconds;
	_frameCount++;
}