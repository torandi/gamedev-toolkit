#include <SDL/SDL.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "render.h"
#include "render_object.h"
#include "logic.h"
#include "input.h"

#define REF_FPS 30
#define REF_DT (1.0/REF_FPS)

bool fullscreen = false;


static void setup(){
	render_init(1024, 768, fullscreen);

	init_input();

	//load models:
	//objects.push_back(RenderObject("models/vadertie.blend"));
	//objects.push_back(RenderObject("models/apple.obj"));
	objects.push_back(RenderObject("models/mario_obj.obj"));
	//objects.push_back(RenderObject("models/ninja.b3d"));
	objects.back().rotationMatrix.RotateY(180);
	//objects.push_back(RenderObject("models/cube.obj"));
	//objects.back().position+=glm::vec3(0.0, 0.0, 100.f);
	objects.push_back(RenderObject("models/nintendo.obj"));
	objects.back().position-=glm::vec3(0.0,1.f,0.0f);
}


static void cleanup(){
	cleanup_input();
	SDL_Quit();
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
	 logic(dt);
	 render(dt);
		 
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
