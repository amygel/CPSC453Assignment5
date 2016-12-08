#include "camera.h"
#include <cstdio>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
   : dir(vec3(0, 0, -1)), 
			right(vec3(1, 0, 0)), 
			up(vec3(0, 1, 0)),
			pos(vec3(0, 0, 0))
{}

Camera::Camera(vec3 dir, vec3 pos)
   : dir(normalize(dir)), pos(pos)
{
	right = normalize(cross(dir, vec3(0, 1, 0)));
	up =  normalize(cross(right, dir));
}

mat4 Camera::getMatrix()
{
   return lookAt(pos, vec3(0.0f), up);
}

void Camera::cameraRotation(float x, float y)
{
	mat4 rotateAroundY = rotateAbout(up, x);
	mat4 rotateAroundX = rotateAbout(right, y);

	dir = normalize(rotateAroundX * rotateAroundY * vec4(dir,0));
   right = normalize(cross(dir, vec3(0, 1, 0)));
	up =  normalize(cross(right, dir));
}

mat4 Camera::rotateAbout(vec3 axis, float radians)
{
   mat4 I(1.0f);
   return rotate(I, radians, axis);
}
