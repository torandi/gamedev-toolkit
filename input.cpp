#include "input.h"
#include "camera.h"
#include <SDL/SDL.h>
#include <cmath>

bool keys[SDLK_LAST];
SDL_Joystick *joy = NULL;

bool * moved_triggers;

float normalized_axis_value(int axis) {
	int value = SDL_JoystickGetAxis(joy, axis);
	float n_value = value/AXIS_MAX;

	if(fabs(n_value) < DEAD_ZONE)
		return 0;
	else
		return n_value;
}

//For triggers
float normalized_trigger_value(int axis) {
	if(moved_triggers[axis]) {
		int value = SDL_JoystickGetAxis(joy, axis);
		float n_value = (value+AXIS_MAX)/(2*AXIS_MAX);
		if(fabs(n_value) < DEAD_ZONE)
			return 0;
		else
			return n_value;
	} else {
		return 0;
	}
}

float get_hat_up_down(int hat) {
	int val = SDL_JoystickGetHat(joy, hat);
	if(val & SDL_HAT_UP)
		return 1.f;
	else if(val & SDL_HAT_DOWN)
		return -1.f;
	else
		return 0.f;
	
}

float get_hat_right_left(int hat) {
	int val = SDL_JoystickGetHat(joy, hat);
	if(val & SDL_HAT_RIGHT)
		return 1.f;
	else if(val & SDL_HAT_LEFT)
		return -1.f;
	else
		return 0.f;
	
}
void init_input() {
	SDL_JoystickEventState(SDL_TRUE);
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);


	if(SDL_NumJoysticks()>0){
		// Open joystick
		joy=SDL_JoystickOpen(0);
		if(joy) {
			printf("Opened Joystick 0\n");
			printf("Name: %s\n", SDL_JoystickName(0));
			printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joy));
			printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joy));
			printf("Number of Balls: %d\n", SDL_JoystickNumBalls(joy));
			printf("Number of Hats: %d\n", SDL_JoystickNumHats(joy));
			moved_triggers = new bool[SDL_JoystickNumAxes(joy)];
			for(int i=0; i < SDL_JoystickNumAxes(joy); ++i)
				moved_triggers[i]=false;
		} else 
			printf("Couldn't open joystick 0\n");
	}
}

void cleanup_input() {
	if(SDL_JoystickOpened(0) && joy)
		SDL_JoystickClose(joy);
}

void poll(bool* run){
	SDL_Event event;
	//SDL_JoystickUpdate();
	while ( SDL_PollEvent(&event) ){
		switch (event.type){
			case SDL_QUIT:
				*run = false;
				break;
			case SDL_KEYDOWN:
				keys[event.key.keysym.sym] = false;
				break;
			case SDL_KEYUP:
				if(event.key.keysym.sym == SDLK_ESCAPE) 
					*run = false;
				else
					keys[event.key.keysym.sym] = true;
				break;
			case SDL_JOYAXISMOTION:
				moved_triggers[event.jaxis.axis]=true;
				break;


			case SDL_JOYBUTTONDOWN:
				break;
			/*
			case SDL_JOYBUTTONUP:
				break;
			case SDL_JOYHATMOTION:
				//joystickIndex=event.jhat.which;
				break;
*/
		}
	}
}

bool button_down(int btn) {
	return SDL_JoystickGetButton(joy, btn)==1;
}
