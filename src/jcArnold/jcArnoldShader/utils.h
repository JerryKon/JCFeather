#ifndef UTILS_H
#define UTILS_H

#include "ai.h"

inline void rgbToHsv(AtRGB& input)
{
	AtFloat comp_max = std::max(input.r, std::max(input.g, input.b));
	AtFloat comp_min = std::min(input.r, std::min(input.g, input.b));
	AtFloat delta = comp_max - comp_min;

	//--- v
	AtFloat v = comp_max;

	//--- s
	AtFloat s = (comp_max != 0.0f) ? (delta / comp_max) : 0.0f;

	//-- h
	AtFloat h;
	if (s == 0.0)
		h = 0.0;
	else
	{
		AtFloat inv_delta = 1.0f / delta;

		if (input.r == comp_max)			h = inv_delta * (input.g - input.b);
		else if (input.g == comp_max)		h = 2.0f + inv_delta * (input.b - input.r);
		else								h = 4.0f + inv_delta * (input.r - input.g);

		h *= 60.0;

		if (h < 0.0f)
			h += 360.0f;
	}

	input.r = h;
	input.g = s;
	input.b = v;
}

inline float hueClosestDist(float h0,float h1)
{
	float d0=abs(h0-h1);
	float d1=360-d0;
	if(d0<=d1) return d0;
	else return d1;
	}
	
#endif // UTILS_H