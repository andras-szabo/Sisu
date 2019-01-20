#include "stdafx.h"
#include "TransformUpdateSystem.h"
#include "GameTimer.h"
#include "Arena.h"
#include "GameObject.h"

bool TransformUpdateSystem::Update(const GameTimer& gt, Arena<GameObject>& bricks)
{
	_elapsedSinceLastUpdate += gt.DeltaTimeSeconds();

	auto somethingChanged = false;

	while (_elapsedSinceLastUpdate > _updatePeriod)
	{
		_elapsedSinceLastUpdate -= _updatePeriod;
		somethingChanged = DoUpdate(bricks) || somethingChanged;
	}

	return somethingChanged;
}

bool TransformUpdateSystem::DoUpdate(Arena<GameObject>& bricks)
{
	auto somethingChanged = false;
	_bricksToUpdate.clear();
	_bricksToUpdateIndex = 0;
	for (auto& brick : bricks)
	{
		if (brick.isRoot)
		{
			_bricksToUpdate.push_back(&brick);
		}
	}

	while (_bricksToUpdateIndex < _bricksToUpdate.size())
	{
		auto brick = _bricksToUpdate[_bricksToUpdateIndex];
		auto delta = brick->velocityPerSec * _updatePeriod;
		brick->localPosition += delta;
		// TODO update rotation
		// TODO update scale maybe

		auto parentTransform = brick->isRoot ? nullptr : &bricks[brick->parentIndex].transform;
		brick->RefreshTransform(parentTransform);

		if (brick->hasChildren)
		{
			for (int i = brick->childrenStartIndex; i <= brick->childrenEndIndex; ++i)
			{
				_bricksToUpdate.push_back(&(bricks[i]));
			}
		}

		_bricksToUpdateIndex++;
		somethingChanged = true;
	}

	return somethingChanged;
}