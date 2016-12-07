#pragma once

#include "glm/glm.hpp"

using namespace glm;

class Camera{
public:
	vec3 dir;
	vec3 up;
	vec3 right;
	vec3 pos;

	Camera();
	Camera(vec3 dir, vec3 pos);

	mat4 getMatrix();
	void cameraRotation(float x, float y); 

private:
   mat4 rotateAbout(vec3 axis, float radians);
};
