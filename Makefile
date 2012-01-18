GLSDK_PATH = ../glsdk

OBJS = main.o renderer.o render_object.o logic.o input.o camera.o movable_object.o light.o render_group.o move_group.o world.o shader.o
CFLAGS += -Wall `sdl-config --cflags` -g
LDFLAGS += `sdl-config --libs` -lassimp

CFLAGS  += -I$(GLSDK_PATH)/glload/include -I$(GLSDK_PATH)/glimg/include -I$(GLSDK_PATH)/glm -I$(GLSDK_PATH)/glutil/include -I$(GLSDK_PATH)/glmesh/include  -I $(GLSDK_PATH)/glimg/include
LDFLAGS += -L$(GLSDK_PATH)/glload/lib -L$(GLSDK_PATH)/glimg/lib -L$(GLSDK_PATH)/glutil/lib -L$(GLSDK_PATH)/glmesh/lib -L $(GLSDK_PATH)/glimg/lib
LDFLAGS += -lglloadD -lglutilD -lGL -lGLU -lglimgD #-lglmeshD 

all: gamedev
 
gamedev: $(OBJS) $(SPRITES)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -rf *.o *.d gamedev

%.o : %.cpp
	@$(CXX) -MM $(CFLAGS) $< > $*.d
	$(CXX) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)
