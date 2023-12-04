// Based on Cube drawing code by Matt Daisley @ wikihow
#include <iostream>
#include <vector>
#include <string>

#include <stdarg.h>
#include <math.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "polygons.h"

#include <CL/cl2.hpp>

#include "openCLsharable.h"

void display();
void specialKeys();

double rotate_y=0; 
double rotate_x=0;

std::vector<polygon> getPolysForFunction_OpenCL(int num_polygons);

static std::vector<polygon> polygon_vec;
extern openCLsharable global_ocs;
extern void runOpenCLkernel(const openCLsharable &ocs, const float *A, const float *B, float *C, std::string kernel_string, int buff_size);

void render_gl_polygon(const std::vector<tricoord> &vertices, const tricoord color=tricoord(.5, .5, .5)) {
	glBegin(GL_POLYGON);
	
	glColor3f(color.x, color.y, color.z);
	
	for(auto vertex: vertices)
		glVertex3f(vertex.x, vertex.y, vertex.z);
	
	glEnd();
	
	return;
}

// is a callback
void display(){
	// Clear screen and Z-buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();

	// Other Transformations
	// glTranslatef(0.1, 0.0, 0.0);      // Not included
	// glRotatef(180, 0.0, 1.0, 0.0 );    // Not included

	// Rotate when user changes rotate_x and rotate_y
	glRotatef(rotate_x, 1.0, 0.0, 0.0);
	glRotatef(rotate_y, 0.0, 1.0, 0.0);

	// Other Transformations
	// glScalef( 2.0, 2.0, 0.0 );          // Not included
	
	for(auto curr_poly: polygon_vec)
		render_gl_polygon(curr_poly.vertices, curr_poly.color);

	//glFlush();
	glutSwapBuffers();
	
	return;
}

// ----------------------------------------------------------
// specialKeys() Callback Function
// ----------------------------------------------------------
void specialKeys( int key, int x, int y ) {
	//  Right arrow - increase rotation by 5 degree
	if (key == GLUT_KEY_RIGHT)
		rotate_y += 5;
	//  Left arrow - decrease rotation by 5 degree
	else if (key == GLUT_KEY_LEFT)
		rotate_y -= 5;
	else if (key == GLUT_KEY_UP)
		rotate_x += 5;
	else if (key == GLUT_KEY_DOWN)
		rotate_x -= 5;

	//  Request display update
	polygon_vec = getPolysForFunction_OpenCL(100000);
	glutPostRedisplay();
}

// ----------------------------------------------------------
// main() function
// ----------------------------------------------------------

std::vector<polygon> getPolysForFunction(int num_polygons) {
	static std::default_random_engine generator;
	static std::normal_distribution<float> distribution(0.0,0.999);
	
	auto eqn = [] (float x, float y) {
		float z = sqrt(pow(1.0, 2) - pow(x, 2) - pow(y, 2));
		return z;
	};
	
	auto constrained_rand = [&] () {
		float num;
		while((num = distribution(generator)) > 0.999);
		
		return num;
	};
	
	std::vector<polygon> polygon_vec;
	
	// randomly sample a point on x and y axis
	for(int i=0; i<num_polygons; i++) {
		tricoord base;
		base.x = constrained_rand();
		base.y = constrained_rand();
		base.z = eqn(base.x, base.y);
		
		polygon_vec.push_back(getRandomPolygon(base, 3));
	}
	
	return polygon_vec;
}

std::vector<polygon> getPolysForFunction_OpenCL(int num_polygons) {
	static std::default_random_engine generator;
	static std::normal_distribution<float> distribution(0.0,0.999);
	
	auto eqn = [] (float x, float y) {
		float z = sqrt(pow(1.0, 2) - pow(x, 2) - pow(y, 2));
		return z;
	};
	
	auto constrained_rand = [&] () {
		float num;
		while((num = distribution(generator)) > 0.999);
		
		return num;
	};
	
	std::vector<polygon> polygon_vec;
	
	float A[num_polygons];
	float B[num_polygons];
	float C[num_polygons];
	
	for(int i=0; i<num_polygons; i++) {
		A[i] = constrained_rand();
		B[i] = constrained_rand();
	}
	
	global_ocs.buff_size = num_polygons;
	global_ocs.createNewBuffers(num_polygons);
	
	runOpenCLkernel(global_ocs, A, B, C, "unit_sphere", num_polygons);	
	
	// randomly sample a point on x and y axis
	for(int i=0; i<num_polygons; i++) {
		tricoord base;
		base.x = A[i];
		base.y = B[i];
		base.z = C[i];
		
		polygon_vec.push_back(getRandomPolygon(base, 3));
	}
	
	return polygon_vec;
}

int gl_main(int argc, char* argv[]){
	//  Initialize GLUT and process user parameters
	glutInit(&argc,argv);

	//  Request double buffered true color window with Z-buffer
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// Create window
	glutCreateWindow("Awesome Cube");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	//  Enable Z-buffer depth test
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);  

	// Callback functions
	glutDisplayFunc(display);
	glutSpecialFunc(specialKeys);
	
	polygon_vec = getPolysForFunction_OpenCL(100000);

	//  Pass control to GLUT for events
	glutMainLoop();

	std::cout << "Hello" << std::endl;

	//  Return to OS
	return 0;
}

