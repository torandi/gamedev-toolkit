#include "render_object.h"
#include "render.h"
#include <string>

#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>

RenderObject::RenderObject(std::string model) {
	scene = aiImportFile( model.c_str(),   aiProcessPreset_TargetRealtime_MaxQuality);
}


void RenderObject::render(double dt) {
}
