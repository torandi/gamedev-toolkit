GLSDK_PATH = ../glsdk

OBJS = main.o renderer.o render_object.o logic.o input.o camera.o movable_object.o light.o render_group.o move_group.o world.o shader.o texture.o terrain.o mesh.o util.o

INCLUDES =  -I$(GLSDK_PATH)/glload/include -I$(GLSDK_PATH)/glm -I$(GLSDK_PATH)/glutil/include  -I$(GLSDK_PATH)/glimg/include
LIB_PATHS = -L$(GLSDK_PATH)/glload/lib -L$(GLSDK_PATH)/glutil/lib -L$(GLSDK_PATH)/glimg/lib

CFLAGS += $(INCLUDES) -Wall `sdl-config --cflags` -g -std=c++0x
LDFLAGS += $(LIB_PATHS) `sdl-config --libs` -lassimp -lglloadD -lglutilD -lGL -lGLU  -lglimgD -lSDL -lSDL_image


all: gamedev
 
gamedev: $(OBJS) $(SPRITES)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -rf *.o *.d gamedev

%.o : %.cpp
	@$(CXX) -MM $(CFLAGS) $< > $*.d
	$(CXX) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)
