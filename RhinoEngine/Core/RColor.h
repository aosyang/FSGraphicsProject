//=============================================================================
// RColor.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

class RColor
{
public:
	float r, g, b, a;

	RColor()
	{}

	RColor(float _r, float _g, float _b, float _a = 1.0f)
		: r(_r), g(_g), b(_b), a(_a)
	{}

	RColor(const float* c)
		: r(c[0]), g(c[1]), b(c[2]), a(c[3])
	{}

	RColor(const RColor& rhs)
		: r(rhs.r), g(rhs.g), b(rhs.b), a(rhs.a)
	{}

	RColor& operator=(const RColor& rhs)
	{
		r = rhs.r; g = rhs.g; b = rhs.b; a = rhs.a;
		return *this;
	}

	// Predefined colors
	static RColor Red;
	static RColor Green;
	static RColor Blue;
	static RColor Cyan;
	static RColor Magenta;
	static RColor Yellow;
	static RColor Black;
	static RColor White;
};
