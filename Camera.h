#pragma once

#include "Vector3.h"
#include "Matrix4.h"
#include <GL/glut.h>

class Camera
{
protected:
	Vector3 e; // Center of projection
	Vector3 d; // Look at
	Vector3 up; // Up-vector
	Matrix4 C; // Internal camera

public:
	Camera();
	Camera(Vector3, Vector3, Vector3);

	void setC(Matrix4);
	Matrix4 getC();
	GLdouble* getGLMatrix();
};

