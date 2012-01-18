OBJS = main.o renderer.o render_object.o logic.o input.o camera.o movable_object.o light.o render_group.o move_group.o world.o shader.o
CFLAGS += -Wall `sdl-config --cflags` -g
LDFLAGS += `sdl-config --libs` -lassimp

CFLAGS  += -I../glsdk/glload/include -I../glsdk/glimg/include -I../glsdk/glm -I../glsdk/glutil/include -I../glsdk/glmesh/include  -I ../glsdk/glimg/include
LDFLAGS += -L../glsdk/glload/lib -L../glsdk/glimg/lib -L../glsdk/glutil/lib -L../glsdk/glmesh/lib -L ../glsdk/glimg/lib
LDFLAGS += -lglloadD -lglutilD -lGL -lGLU -lglimgD #-lglmeshD 

all: gamedev
 
gamemenu: $(OBJS) $(SPRITES)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -rf *.o *.d gamedev

%.o : %.cpp
	@$(CXX) -MM $(CFLAGS) $< > $*.d
	$(CXX) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)
