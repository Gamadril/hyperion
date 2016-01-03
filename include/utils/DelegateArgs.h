#pragma once

typedef struct
{
	int priority;
	std::vector<ColorRgb> &ledColors;
	int timeout_ms;
	bool clearEffects;
} ColorSetArgs;
