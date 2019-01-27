#pragma once
#include <chrono>

using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
using DurationInSeconds = std::chrono::duration<float>;

class GameTimer
{
public:
	GameTimer() :
		_deltaTimeSeconds(-1.0f),
		_isPaused(false)
	{
	}

	float DeltaTimeSeconds() const;
	float SecondsSinceReset() const;
	bool IsPaused() const;
	unsigned long FrameCount() const;

	void Reset();	// call before message loop <--- TODO: Better name
	void Unpause();
	void Pause();
	void Tick();

private:
	float _deltaTimeSeconds = 0.0f;
	float _secondsSinceReset = 0.0f;
	unsigned long _frameCount = 0;
	TimePoint _previousTimeStamp;
	bool _isPaused;
};