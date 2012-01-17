#ifndef INPUT_H
#define INPUT_H
	#define DEAD_ZONE 0.2f
	#define AXIS_MAX 32767.f
	#include <SDL/SDL.h>

	extern bool keys[SDLK_LAST];

	void init_input();
	void cleanup_input();

	void poll(bool *run);

	float normalized_trigger_value(int axis);
	float normalized_axis_value(int axis);
#endif
