#pragma once
#include <algorithm>
#include <vector>
#include <iostream>

#define OUT 

struct Gap
{
	Gap(std::size_t start, std::size_t siz) : startIndex(start), size(siz) {}
	std::size_t startIndex;
	std::size_t size;

	void Print() const
	{
		std::cout << "Gap size: " << size << ", start: " << startIndex << "\n";
	}
};

template <typename T> struct ArenaIterator;

template <typename T>
class Arena
{
public:
	Arena() : _actualSize(0), _begin(0), _end(0)
	{
	}

	Arena(std::size_t reservedSize) : _actualSize(0), _begin(0), _end(0)
	{
		_items.reserve(reservedSize);
		_isUsed.reserve(reservedSize);
	}

	T& operator[](std::size_t index) { ThrowIfNotUsed(index); return _items[index]; }
	const T& operator[](std::size_t index) const { ThrowIfNotUsed(index); return _items[index]; }

	ArenaIterator<typename T> begin() 
	{ 
		auto firstIndex = _actualSize > 0 ? (_isUsed[0] ? 0 : GetNextValidIndex(0)) : _end;
		return ArenaIterator<T>(this, firstIndex);
	}

	ArenaIterator<typename T> end() { return ArenaIterator<T>(this, _end); }

	std::size_t Add(T item);
	std::size_t Add(typename std::vector<T>::iterator begin,
		typename std::vector<T>::iterator end);

	void Remove(std::size_t index, std::size_t count = 1);
	void Clear();

	void PrintGaps() const;
	const std::size_t OccupiedSize() const { return _items.size(); }
	const std::size_t ActualSize() const { return _actualSize; }

	std::size_t GetNextValidIndex(std::size_t current) const;

private:
	bool TryFindBestFittingGap(std::size_t size, OUT std::size_t& placementIndex) const;
	void RefreshGaps();
	void ThrowIfNotUsed(std::size_t index) const;

private:
	std::vector<T> _items;
	std::vector<bool> _isUsed;
	std::size_t _actualSize;
	std::vector<Gap> _gaps;

	std::size_t _begin;
	std::size_t _end;
};

template <typename T>
std::size_t Arena<T>::GetNextValidIndex(std::size_t current) const
{
	std::size_t nextValid = current + 1;

	while (nextValid < _end && !_isUsed[nextValid])
	{
		nextValid++;
	}

	return nextValid;
}

template <typename T>
void Arena<T>::PrintGaps() const
{
	for (const auto& gap : _gaps) { gap.Print(); }
}

template <typename T>
void Arena<T>::ThrowIfNotUsed(std::size_t index) const
{
	if (index >= _isUsed.size() || !_isUsed[index])
	{
		throw std::runtime_error("[Arena] Trying to access unused element.");
	}
}

template <typename T>
void Arena<T>::Clear()
{
	_items.clear();
	_isUsed.clear();
	_gaps.clear();
	_actualSize = 0;
	_end = 0;
}

template <typename T>
std::size_t Arena<T>::Add(typename std::vector<T>::iterator begin,
	typename std::vector<T>::iterator end)
{
	std::size_t placementIndex;
	std::size_t requestedSize = end - begin;

	if (TryFindBestFittingGap(requestedSize, OUT placementIndex))
	{
		auto index = placementIndex;

		while (begin != end)
		{
			_items[index] = *begin;
			_isUsed[index] = true;
			index++;
			begin++;
		}

		_actualSize += requestedSize;
		RefreshGaps();
	}
	else
	{
		placementIndex = _items.size();

		while (begin != end)
		{
			_items.push_back(*begin);
			_isUsed.push_back(true);
			begin++;
			_actualSize++;
		}
	}

	return placementIndex;
}

template <typename T>
std::size_t Arena<T>::Add(T item)
{
	std::size_t placementIndex;
	if (TryFindBestFittingGap(1, OUT placementIndex))
	{
		_items[placementIndex] = item;
		_isUsed[placementIndex] = true;

		_actualSize++;

		RefreshGaps();

		return placementIndex;
	}
	else
	{
		_items.push_back(item);
		_isUsed.push_back(true);

		_actualSize++;
		_end = _items.size();

		return _items.size() - 1;
	}
}

template <typename T>
bool Arena<T>::TryFindBestFittingGap(std::size_t size, OUT std::size_t& placementIndex) const
{
	auto foundGap = false;

	for (const auto& gap : _gaps)
	{
		if (gap.size >= size)
		{
			foundGap = true;
			placementIndex = gap.startIndex;
		}
		else
		{
			break;
		}
	}

	return foundGap;
}

template <typename T>
void Arena<T>::RefreshGaps()
{
	_gaps.clear();

	std::size_t gapStart = 0;
	std::size_t gapSize = 0;
	auto hasGaps = false;

	for (std::size_t i = 0; i < _items.size(); ++i)
	{
		if (!_isUsed[i])
		{
			if (gapSize == 0) { gapStart = i; gapSize = 1; }
			else { gapSize++; }
		}
		else
		{
			if (gapSize > 0)
			{
				_gaps.emplace_back(Gap(gapStart, gapSize));
				gapSize = 0;
			}
		}
	}

	//Maybe there's a gap in the end
	if (gapSize > 0)
	{
		_gaps.emplace_back(Gap(gapStart, gapSize));
	}

	std::sort(_gaps.begin(), _gaps.end(), [](const Gap& a, const Gap& b) { return a.size > b.size; });
}

template <typename T>
void Arena<T>::Remove(std::size_t index, std::size_t count)
{
	for (int i = 0; i < count && (index + i) < _isUsed.size(); ++i)
	{
		_isUsed[index + i] = false;
	}

	_actualSize -= count;
	RefreshGaps();
}

template <typename T>
struct ArenaIterator
{
	ArenaIterator(Arena<T>* const arena, std::size_t index)
		: arena(arena), index(index) {}

	ArenaIterator& operator++()
	{
		index = arena->GetNextValidIndex(index);
		return *this;
	}

	const T& operator*()
	{
		return (*arena)[index];
	}

	std::size_t index;
	Arena<T>* const arena;
};

template <typename T>
bool operator==(const ArenaIterator<T>& a, const ArenaIterator<T>& b)
{
	return a.arena == b.arena && a.index == b.index;
}

template <typename T>
bool operator!=(const ArenaIterator<T>& a, const ArenaIterator<T>& b)
{
	return !(a == b);
}