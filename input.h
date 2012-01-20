#ifndef INPUT_H
#define INPUT_H
	#define DEAD_ZONE 0.2f
	#define AXIS_MAX 32767.f
	#include <SDL/SDL.h>

	extern bool keys[SDLK_LAST];
	extern SDL_Joystick *joy;

	void init_input();
	void cleanup_input();

	void poll(bool *run);

	bool button_down(int btn);

	float normalized_trigger_value(int axis);
	float normalized_axis_value(int axis);
	float get_hat_up_down(int hat);
	float get_hat_right_left(int hat);
#endif
