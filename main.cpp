#include <SDL/SDL.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "renderer.h"
#include "render_object.h"
#include "logic.h"
#include "input.h"
#include "world.h"

#define REF_FPS 30
#define REF_DT (1.0/REF_FPS)

bool fullscreen = false;

Renderer * renderer;

static void setup(){
	renderer = new Renderer(1024, 768, fullscreen);

	init_input();

	create_world(renderer);
}


static void cleanup(){
	cleanup_input();
	SDL_Quit();
	
	delete renderer;
}

int main(int argc, char* argv[]){
	setup();	
	bool run = true;
	struct timeval ref;
	gettimeofday(&ref, NULL);

  while ( run ){
    struct timeval ts;
	gettimeofday(&ts, NULL);

    /* calculate dt */
    double dt = (ts.tv_sec - ref.tv_sec) * 1000000.0;
    dt += ts.tv_usec - ref.tv_usec;
    dt /= 1000000;

    poll(&run);
	 logic(dt, renderer);
	 update_world(dt, renderer);
	 renderer->render(dt);
		 
    /* framelimiter */
    const int delay = (REF_DT - dt) * 1000000;
    if ( delay > 0 ){
      usleep(delay);
    }

    /* store reference time */
    ref = ts;
  }

  cleanup();
}
