#pragma once
#include <vector>

template <typename T> class Arena;
class GameObject;
class GameTimer;

class TransformUpdateSystem
{
public:
	bool Update(const GameTimer& gt, Arena<GameObject>& bricks);

private:
	bool DoUpdate(Arena<GameObject>& bricks);

private:
	std::vector<GameObject*> _bricksToUpdate;
	std::size_t _bricksToUpdateIndex = 0;

	float _elapsedSinceLastUpdate = 0.0f;
	float _updatePeriod = 0.016f;

};
