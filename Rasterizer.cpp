#ifdef _WIN32
  #include <windows.h>
#endif

#include <stdlib.h>
#include <crtdbg.h>

#include "Rasterizer.h"
#include <iostream>
#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>

static int window_width = 512, window_height = 512;
static float* pixels = new float[window_width * window_height * 3];
double* zbuf = new double[window_width * window_height];

static double rotato = 0.1;

static Animals a = (*new Animals());
static vector<Vector4> bun, drag, home_sweet_home;
static vector<Vector3> bun3, drag3, bunv, dragv;
Vector3 lpos = (*new Vector3(0.0, 100.0, 0));
static Matrix4 modelv, proj, vport;
static Camera camera;
static int mode = 0;
static bool light = false;
static bool shadows = false;
static bool zbuffer = false;
static bool psize = false;
Vector4 *p = new Vector4();
double unpie = 1 / M_PI;
double clr = 0.0;

Camera c0 = (*new Camera(*new Vector3(0, 10, 10),
	*new Vector3(0, 0, 0), *new Vector3(0, 1, 0))); // Default house camera img1
Camera c1 = (*new Camera(*new Vector3(0, 0, 20),
	*new Vector3(0, 0, 0), *new Vector3(0, 1, 0))); // Bunny/Dragon cam

/* HOUSE STARTS HERE */
// This data structure defines a simple house

int nVerts = 42;    // your vertex array needs to have this many entries

// These are the x,y,z coordinates of the vertices of the triangles
float vertices[] = {
            -4,-4,4, 4,-4,4, 4,4,4, -4,4,4,     // front face
            -4,-4,-4, -4,-4,4, -4,4,4, -4,4,-4, // left face
            4,-4,-4,-4,-4,-4, -4,4,-4, 4,4,-4,  // back face
            4,-4,4, 4,-4,-4, 4,4,-4, 4,4,4,     // right face
            4,4,4, 4,4,-4, -4,4,-4, -4,4,4,     // top face
            -4,-4,4, -4,-4,-4, 4,-4,-4, 4,-4,4, // bottom face

            -20,-4,20, 20,-4,20, 20,-4,-20, -20,-4,-20, // grass
            -4,4,4, 4,4,4, 0,8,4,                       // front attic wall
            4,4,4, 4,4,-4, 0,8,-4, 0,8,4,               // left slope
            -4,4,4, 0,8,4, 0,8,-4, -4,4,-4,             // right slope
            4,4,-4, -4,4,-4, 0,8,-4};                   // rear attic wall
/* HOUSE ENDS HERE */

using namespace std;

struct Color    // generic color class
{
  float r,g,b;  // red, green, blue
};

void spin(double d) {
	Matrix4 *m = new Matrix4();
	m->makeRotateY(d);
	modelv = modelv * (*m);
}

void loadData()
{
  // point cloud parser goes here

	// Load point coordinates from files
	a.parseBunny();
	a.parseDragon();

	bun = a.getBunny4();
	bunv = a.getBunnyNorm();
	drag = a.getDragon4();
	dragv = a.getDragonNorm();

	// Load house
	for (int i = 0; i < nVerts; i++) {
		home_sweet_home.push_back((*new Vector4(vertices[i * 3],
			vertices[i * 3 + 1], vertices[i * 3 + 2], 1)));
	}
}

// Clear frame buffer
void clearBuffer()
{
  Color clearColor = {0.0, 0.0, 0.0};   // clear color: black
  for (int i=0; i<window_width*window_height; ++i)
  {
    pixels[i*3]   = clearColor.r;
    pixels[i*3+1] = clearColor.g;
    pixels[i*3+2] = clearColor.b;
  }  

  for (int i = 0; i < window_width*window_height; i++) {
	  zbuf[i] = 1.0;
  }
}

// Draw a point into the frame buffer
void drawPoint(int x, int y, float r, float g, float b)
{
  int offset = y*window_width*3 + x*3;
  pixels[offset]   = r;
  pixels[offset+1] = g;
  pixels[offset+2] = b;
}

void drawPointSize(int x, int y, float r, float g, float b, int s)
{
	int offset = y*window_width * 3 + x * 3;
	pixels[offset] = r;
	pixels[offset + 1] = g;
	pixels[offset + 2] = b;

	if (p->getZ() > 0 && offset < window_width * window_height) {
		if (s > 1) {
			offset = (y + 1)*window_width + x;
			if (offset < window_width * window_height && p->getZ() < zbuf[offset]) {
				zbuf[offset] = p->getZ();
				drawPoint(x, y + 1, 0, clr, 1 - clr);
			}
			offset = (y - 1)*window_width + x;
			if (offset < window_width * window_height && p->getZ() < zbuf[offset]) {
				zbuf[offset] = p->getZ();
				drawPoint(x, y - 1, 0, clr, 1 - clr);
			}
			offset = y*window_width + (x + 1);
			if (offset < window_width * window_height && p->getZ() < zbuf[offset]) {
				zbuf[offset] = p->getZ();
				drawPoint(x + 1, y, 0, clr, 1 - clr);
			}
			offset = y*window_width + (x - 1);
			if (offset < window_width * window_height && p->getZ() < zbuf[offset]) {
				zbuf[offset] = p->getZ();
				drawPoint(x - 1, y, 0, clr, 1 - clr);
			}
		}
		if (s > 2) {
			offset = (y + 1)*window_width + (x + 1);
			if (offset < window_width * window_height && p->getZ() < zbuf[offset]) {
				zbuf[offset] = p->getZ();
				drawPoint(x + 1, y + 1, 0, clr, 1 - clr);
			}
			offset = (y - 1)*window_width + (x + 1);
			if (offset < window_width * window_height && p->getZ() < zbuf[offset]) {
				zbuf[offset] = p->getZ();
				drawPoint(x + 1, y - 1, 0, clr, 1 - clr);
			}
			offset = (y + 1)*window_width + (x - 1);
			if (offset < window_width * window_height && p->getZ() < zbuf[offset]) {
				zbuf[offset] = p->getZ();
				drawPoint(x - 1, y + 1, 0, clr, 1 - clr);
			}
			offset = (y - 1)*window_width + (x - 1);
			if (offset < window_width * window_height && p->getZ() < zbuf[offset]) {
				zbuf[offset] = p->getZ();
				drawPoint(x - 1, y - 1, 0, clr, 1 - clr);
			}
		}
	}
}

void rasterize()
{
  // Put your main rasterization loop here
  // It should go over the point model and call drawPoint for every point in it

	// Draw Bunny or Dragon
	if (mode == 1) {
		for (unsigned int i = 0; i < bun.size(); i++) {
			*p = proj * (camera.getC() * (modelv * bun[i]));
			p->dehomogenize();
			*p = vport * *p;
			if (p->getX() >= 0 && p->getX() <= window_width &&
				p->getY() >= 0 && p->getY() <= window_height) {
				clr = unpie * .8 * lpos.dot((lpos - p->getV3()), bunv[i]);
				if (psize) {
					int offset = p->getY()*window_width + p->getX();
					int size = 1;
					if (p->getZ() < zbuf[offset] && p->getZ() > 0) {
						zbuf[offset] = p->getZ();
						if (p->getZ() < 8.0 && p->getZ() >= 6.0) size = 2;
						if (p->getZ() < 6.0) size = 3;
						drawPointSize(p->getX(), p->getY(), 0, clr, 1 - clr, size);
					}
				}
				else if (zbuffer){
					int offset = p->getY()*window_width + p->getX();
					if (p->getZ() < zbuf[offset] && p->getZ() > 0) {
						zbuf[offset] = p->getZ();
						drawPoint(p->getX(), p->getY(), 0, clr, 1 - clr);
					}
				}
				else if (shadows) drawPoint(p->getX(), p->getY(), 0, clr, 1 - clr);
				else if (light) drawPoint(p->getX(), p->getY(), 0, clr, 0);
				else drawPoint(p->getX(), p->getY(), 1.0, 1.0, 1.0);
			}
		}
	}
	else if (mode == 2) {
		for (unsigned int i = 0; i < drag.size(); i++) {
			*p = proj * (camera.getC() * (modelv * drag[i]));
			p->dehomogenize();
			*p = vport * *p;
			if (p->getX() >= 0 && p->getX() <= window_width &&
				p->getY() >= 0 && p->getY() <= window_height) {
				clr = unpie * .8 * lpos.dot((lpos - p->getV3()), dragv[i]);
				if (psize) {
					int offset = p->getY()*window_width + p->getX();
					int size = 1;
					if (p->getZ() < zbuf[offset] && p->getZ() > 0) {
						zbuf[offset] = p->getZ();
						if (p->getZ() < 8.0 && p->getZ() >= 6.0) size = 2;
						if (p->getZ() < 6.0) size = 3;
						drawPointSize(p->getX(), p->getY(), 0, clr, 1 - clr, size);
					}
				}
				else if (zbuffer){
					int offset = p->getY()*window_width + p->getX();
					if (p->getZ() < zbuf[offset] && p->getZ() > 0) {
						zbuf[offset] = p->getZ();
						drawPoint(p->getX(), p->getY(), 0, clr, 1 - clr);
					}
				}
				else if (shadows) drawPoint(p->getX(), p->getY(), 0, clr, 1 - clr);
				else if (light) drawPoint(p->getX(), p->getY(), 0, clr, 0);
				else drawPoint(p->getX(), p->getY(), 1.0, 1.0, 1.0);
			}
		}
	}
	else { // 0 display house
		for (int i = 0; i < nVerts; i++) {
			*p = proj * (camera.getC() * (modelv * home_sweet_home[i]));
			p->dehomogenize();
			*p = vport * *p;
			if (p->getX() >= 0 && p->getX() <= window_width &&
				p->getY() >= 0 && p->getY() <= window_height) {
				drawPoint(p->getX(), p->getY(), 1.0, 1.0, 1.0);
			}
		}
	}
}

void displayCallback()
{
	clearBuffer();
	rasterize();

	// glDrawPixels writes a block of pixels to the framebuffer
	glDrawPixels(window_width, window_height, GL_RGB, GL_FLOAT, pixels);

	glutSwapBuffers();
}

// Called whenever the window size changes
void reshapeCallback(int new_width, int new_height)
{
  window_width  = new_width;
  window_height = new_height;
  delete[] pixels;

  proj.setProjection(60.0, (double)new_width / (double)new_height, 1.0, 1000.0);
  vport.setViewport(0, new_width, 0, new_height);

  pixels = new float[window_width * window_height * 3];
  zbuf = new double[window_width * window_height];

  displayCallback();
}

void idleCallback() {
	//spin(rotato);
	displayCallback();
}

void keyboardCallback(unsigned char key, int, int)
{
  //cerr << "Key pressed: " << key << endl;
	Matrix4 *m = new Matrix4();
	switch (key) {
		case 'x':
			m->makeTranslate(-1, 0, 0);
			modelv = modelv * (*m);
			break;
		case 'X':
			m->makeTranslate(1, 0, 0);
			modelv = modelv * (*m);
			break;
		case 'y':
			m->makeTranslate(0, -1, 0);
			modelv = modelv * (*m);
			break;
		case 'Y':
			m->makeTranslate(0, 1, 0);
			modelv = modelv * (*m);
			break;
		case 'z':
			m->makeTranslate(0, 0, -1);
			modelv = modelv * (*m);
			break;
		case 'Z':
			m->makeTranslate(0, 0, 1);
			modelv = modelv * (*m);
			break;
		case 's':
			m->makeScale(0.9, 0.9, 0.9);
			modelv = modelv * (*m);
			break;
		case 'S':
			m->makeScale(1.1, 1.1, 1.1);
			modelv = modelv * (*m);
			break;
		case 'r':
			m->makeRotateY(-10);
			modelv = modelv * (*m);
			break;
		case 'R':
			m->makeRotateY(10);
			modelv = modelv * (*m);
			break;
		case '1':
			light = !light;
			break;
		case '2':
			shadows = !shadows;
			break;
		case '3':
			zbuffer = !zbuffer;
			break;
		case '4':
			psize = !psize;
			break;
	}
}

void specialCallback(int k, int x, int y) {
	//cerr << "Key pressed (spec): " << k << endl;
	Matrix4 *m = new Matrix4();
	switch(k) {
		case GLUT_KEY_F1: // Bunny
			if (mode != 1) {
				modelv.identity();
				double s = 36 * tan(M_PI / 6) /
					(a.getLargeBunny().getX() - a.getSmallBunny().getX());
				double x = -(a.getLargeBunny().getX() + a.getSmallBunny().getX()) / 2;
				double y = -(a.getLargeBunny().getY() + a.getSmallBunny().getY()) / 2;
				double z = -(a.getLargeBunny().getZ() + a.getSmallBunny().getZ()) / 2;
				m->makeTranslate(x, y, z);
				modelv = modelv * (*m);
				m->makeScale(s, s, s);
				modelv = modelv * (*m);
			}
			camera = c1;
			m->makeTranslate(0, 0, -20);
			camera.setC(camera.getC() * (*m));
			rotato = 1.0;
			mode = 1;
			break;
		case GLUT_KEY_F2: // Dragon
			if (mode != 2) {
				modelv.identity();
				double s = 40 * tan(M_PI / 6) /
					(a.getLargeDragon().getX() - a.getSmallDragon().getX());
				double x = -(a.getLargeDragon().getX() + a.getSmallDragon().getX()) / 2;
				double y = -(a.getLargeDragon().getY() + a.getSmallDragon().getY()) / 2;
				double z = -(a.getLargeDragon().getZ() + a.getSmallDragon().getZ()) / 2;
				m->makeTranslate(x, y, z);
				modelv = modelv * (*m);
				m->makeScale(s, s, s);
				modelv = modelv * (*m);
			}
			camera = c1;
			m->makeTranslate(0, 0, -20);
			camera.setC(camera.getC() * (*m));
			rotato = 1.5;
			mode = 2;
			break;
	}
}

int main(int argc, char** argv) {
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  glutInitWindowSize(window_width, window_height);
  glutCreateWindow("Rasterizer");

  // Initialize Modelview as identity matrix
  Matrix4 *m = new Matrix4();
  //m->makeScale(10, 10, 10);
  modelv = (*new Matrix4());
  modelv.identity();
  //modelv = modelv * (*m);

  // Default camera-1 parameters from Proj 2 (Window::c0)
  camera = (*new Camera(*new Vector3(0,10,10),
	  *new Vector3(0,0,0), *new Vector3(0,1,0)));
  m->makeTranslate(0, 0, -20);
  camera.setC(camera.getC() * (*m));

  // Initialize Projection matrix (04_Projection2 16/51)
  proj = (*new Matrix4());
  // sP(left, right, top, bottom, near, far)
  //proj.setProjection(-10.0, 10.0, 10.0, -10.0, 1, 1000.0);

  // sP(fov, aspect, near, far);
  proj.setProjection(60.0, window_width/window_height, 1, 1000);

  // Initalize Viewport matrix (04_Projection2 19/51)
  vport = (*new Matrix4());
  // D(x0, x1, y0, y1)
  vport.setViewport(0, window_width, 0, window_height);
  
  // Parse Bunny & Dragon
  loadData();

  glutReshapeFunc(reshapeCallback);
  glutDisplayFunc(displayCallback);
  glutIdleFunc(idleCallback); // Added IdleFunc

  glutKeyboardFunc(keyboardCallback);
  glutSpecialFunc(specialCallback); // Added implementation for F-keys

  glutMainLoop();

  return 0;
}