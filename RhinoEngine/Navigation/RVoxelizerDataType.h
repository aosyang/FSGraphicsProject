//=============================================================================
// RVoxelizerDataType.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include <functional>

template<class T>
inline void HashCombine(size_t& HashValue, const T& Element)
{
	std::hash<T> Hash;
	HashValue ^= Hash(Element) + 0x9e3779b9 + (HashValue << 6) + (HashValue >> 2);
}

/// Key for a traversable span. Used for searching a span at a given location
class OpenSpanKey
{
public:
	OpenSpanKey(int InX, int InZ, int InSpanIndex)
		: x(InX), z(InZ), span_idx(InSpanIndex)
	{
		HashValue = CalcHash();
	}

	OpenSpanKey(const OpenSpanKey& Rhs)
	{
		x = Rhs.x; z = Rhs.z; span_idx = Rhs.span_idx; HashValue = Rhs.HashValue;
	}

	OpenSpanKey& operator=(const OpenSpanKey& Rhs)
	{
		x = Rhs.x; z = Rhs.z; span_idx = Rhs.span_idx; HashValue = Rhs.HashValue;
		return *this;
	}

	bool operator==(const OpenSpanKey& Rhs) const
	{
		return HashValue == Rhs.HashValue;
	}

	bool operator!=(const OpenSpanKey& Rhs) const
	{
		return HashValue != Rhs.HashValue;
	}

	bool operator<(const OpenSpanKey& Rhs) const
	{
		return HashValue < Rhs.HashValue;
	}

	bool IsValid() const
	{
		return x >= 0;
	}

	int x, z, span_idx;
	const static OpenSpanKey Invalid;

private:
	size_t CalcHash() const
	{
		size_t Hash = 0;
		HashCombine(Hash, x);
		HashCombine(Hash, z);
		HashCombine(Hash, span_idx);
		return Hash;
	}

private:
	size_t HashValue;
};
