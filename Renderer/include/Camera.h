#pragma once
#include <GL/glut.h>
#include "GL/freeglut_ext.h"

class Camera
{
public:

	// angle of rotation for the camera direction
	float angle = 0.0f;
	// actual vector representing the camera's direction
	float lx = 0.0f,ly = 0.0f, lz = -1.0f;
	// XZ position of the camera
	float x = 0.0f, y = 1.0f, z = 500.0f;
	// the key states. These variables will be zero
	//when no key is being presses
	float deltaAngle = 0.0f;
	float deltaMove = 0;
	float deltaVertical = 0;

	void Init();

	void computePos(float deltaMove);

	void computeDir(float deltaAngle);

	void computeVertical(float deltaVertical);

private:

};