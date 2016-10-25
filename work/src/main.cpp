//---------------------------------------------------------------------------
//
// Copyright (c) 2015 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty. 
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#include "comp308.hpp"
#include "imageLoader.hpp"
#include "shaderLoader.hpp"
#include "geometry.hpp"

using namespace std;
using namespace comp308;


// Global variables
// 
GLuint g_winWidth  = 640;
GLuint g_winHeight = 480;
GLuint g_mainWindow = 0;

// light postions 
GLfloat spotPosX = 0.0f;
GLfloat spotPosY = 8.0f;
GLfloat spotPosZ = 1.0f;
GLfloat spotDirX = 0.2f;
GLfloat spotDirY = -4.0f;
GLfloat spotDirZ = 0.0f;
GLfloat cutoff = 25.00f;

GLfloat coneHeight = 3;
GLfloat coneAngle = 0.0f;
bool drawSpotlight = true;
GLUquadric* q;

float directionPosition[] = { -100.0f, 20.0f, -40.0f, 1.0f };
float pointPosition[] = { 4.0f, 0.2f, 4.0f, 1.0f };
float coneDir[] = { spotDirX, spotDirY, spotDirZ, 1.0 };
// Projection values
// 
float g_fovy = 20.0;
float g_znear = 0.1;
float g_zfar = 1000.0;


// Mouse controlled Camera values
//
bool g_mouseDown = false;
vec2 g_mousePos;
float g_yRotation = 0;
float g_xRotation = 0;
float g_zoomFactor = 1.0;

//key rotation


// light on/off
bool ambientLight = true;
bool pointLight = true;
bool spotlight = true;
bool directionalLight = true;

//key rotation control
float rotationDegrees = 0;
bool rotateOn = false;

// Scene information
///
//GLuint g_texture = 0;
GLuint g_shader = 0;
bool g_useShader = false;
 
//image pointers
Geometry* torus = NULL;
Geometry* sphere = NULL;
Geometry* bunny = NULL;
Geometry* table = NULL;
Geometry* teapot = NULL;
Geometry* box = NULL;

// load obj files
void loadFiles() {
	glPushMatrix(); {
		glPushMatrix(); {
			table->renderGeometry();
		}
		glPopMatrix();
		glPushMatrix(); {
			glTranslatef(4.0f, 1.0f, 4.0f);
			torus->renderGeometry();
		}
		glPopMatrix();
		glPushMatrix(); {
			glTranslatef(-4.0f, 2.0f, 4.0f);
			sphere->renderGeometry();
		}
		glPopMatrix();
		glPushMatrix(); {
			glTranslatef(0.0f, 0.5f, 0.0f);
			bunny->renderGeometry();
		}
		glPopMatrix();
		glPushMatrix(); {
			glTranslatef(-5.0f, 0.5f, -5.0f);
			teapot->renderGeometry();
		}
		glPopMatrix();
		glPushMatrix(); {
			glTranslatef(5.5f, 2.0f, -4.0f);
			box->renderGeometry();
		}
		glPopMatrix();
	}
	glPopMatrix();
}


// Sets up where and what the light is
// Called once on start up
// 


void setupAmbientLight() {
	
	GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuse[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	 
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);	
	glEnable(GL_LIGHT0);
	}

void setupSpotlight() {
	// Spotlight
	GLfloat spot_diffuse[] = { 1, 1, 1, 1.0 };
	GLfloat spot_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat spotPosition[] = { spotPosX, spotPosY, spotPosZ, 1 };
	GLfloat spotDirection[] = { spotDirX, spotDirY, spotDirZ, 1.0 };
	//coneDir = spotDirection;

	glLightfv(GL_LIGHT1, GL_DIFFUSE, spot_diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, spotPosition);
	glLightfv(GL_LIGHT1, GL_SPECULAR, spot_specular);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, cutoff);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 1);

	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);
}
void setupPointLight() {

	float diffuse[] = { 0.9f, 0.8f, 0.8f, 1.0f };
	//	float ambient[] = { 0.6f, 0.2f, 0.2f, 1.0f };
	glLightfv(GL_LIGHT2, GL_POSITION, pointPosition);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);
	//	glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);

	glEnable(GL_LIGHT2);
}
void setupDirectionalLight() {

	float diffuse[] = { 0.8f, 0.8f, .8f, 1.0f };
	//	float ambient[] = { 0.6f, 0.2f, 0.2f, 1.0f };
	glLightfv(GL_LIGHT3, GL_POSITION, directionPosition);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, diffuse);
	//	glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);

	glEnable(GL_LIGHT3);
}

void initShader() {
	g_shader = makeShaderProgram("work/res/shaders/shaderDemo.vert", "work/res/shaders/shaderDemo.frag");
}


// Sets up where the camera is in the scene
// Called every frame
// 
void setUpCamera() {
	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(g_fovy, float(g_winWidth) / float(g_winHeight), g_znear, g_zfar);

	// Set up the view part of the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0, 0, -50 * g_zoomFactor);
	glRotatef(g_xRotation, 1, 0, 0);
	glRotatef(g_yRotation, 0, 1, 0);
}

void drawLight() {
	gluQuadricNormals(q, GLU_SMOOTH);
	gluQuadricOrientation(q, GLU_INSIDE);

	// draw position thing
	glPushMatrix(); {
		glTranslatef(spotPosX, spotPosY, spotPosZ);
		glColor3f(1.f, 1.f, 1.f);
		gluSphere(q, .3, 100, 100);
 
		glPushMatrix();
		glRotatef(90.00f, 1, 0, 0);
		glTranslatef(0.0f, 0.0f, -1);
		gluCylinder(q, .05, .05, 2, 100, 100);
		glPopMatrix();
	}
	glPopMatrix();

	// draw cone
	glPushMatrix(); {
		vec2 v1 = vec2(spotPosX, spotDirX);
		vec2 v2 = vec2(spotPosY, spotDirY);
		glColor3f(0.1f, 0.1f, 0.1f);
		glTranslatef(spotPosX, spotPosY, spotPosZ);
		glRotatef(90.0 - spotDirZ * 6, 1, 0, 0);
		glRotatef(spotDirX * 6, 0, 1, 0); // z direction
		gluCylinder(q, 0.01, cutoff / 25, 1, 100, 100);
	}
	glPopMatrix();
	/*
	// draw cone
	glPushMatrix(); {
		vec2 v1 = vec2(spotPosX, spotDirX);
		vec2 v2 = vec2(spotPosY, spotDirY);
		glColor3f(0.1f, 0.1f, 0.1f);
		glTranslatef(spotPosX, spotPosY - coneHeight, spotPosZ);
		
	//	glTranslatef(0, 0, coneHeight);
		glRotatef(-90.0 - spotDirZ * 6, 1, 0, 0);
		glRotatef(-spotDirX * 6, 0, 1, 0); // z direction
		glutWireCone(cutoff/10, coneHeight, 10, 10);
	}
	glPopMatrix();	*/
}
//TODO
//void repositionLights() {
//	glNormal3d(0, 1, 0);
//	glLightfv(GL_LIGHT1, GL_POSITION, spotPosition);
//	glLightfv(GL_LIGHT2, GL_POSITION, pointPosition);
//	glLightfv(GL_LIGHT3, GL_POSITION, directionPosition);	 
//}
//// Draw function
//
void draw() {

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Enable flags for normal rendering
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	if (drawSpotlight) drawLight();

	setUpCamera();
	if (ambientLight) setupAmbientLight();
	else glDisable(GL_LIGHT0);
	if (spotlight) setupSpotlight();
	else glDisable(GL_LIGHT1);
	if (pointLight)	setupPointLight();
	else glDisable(GL_LIGHT2);
	if (directionalLight)	setupDirectionalLight();
	else glDisable(GL_LIGHT3);

	if (!rotateOn) loadFiles();
	else {
		if (rotationDegrees < 360) {
			glPushMatrix();

			glRotatef(rotationDegrees, 0.0f, 1.0f, 0.0f);
			glPushMatrix();

			table->renderGeometry();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(4.0f, 1.0f, 4.0f);
			torus->renderGeometry();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(0.0f, 0.5f, 0.0f);
			bunny->renderGeometry();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(-4.0f, 2.0f, 4.0f);
			sphere->renderGeometry();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(-5.0f, 0.5f, -5.0f);
			teapot->renderGeometry();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(5.5f, 2.0f, -4.0f);
			box->renderGeometry();

			glPopMatrix();
			rotationDegrees += .3f;
			glPopMatrix();
		}
		else {
			rotateOn = false;

		}
	}
	

	// Without shaders
	//
	if (!g_useShader) {

	
		// Texture setup
		//
//		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		// Enable Drawing texures
//		glEnable(GL_TEXTURE_2D);
		// Use Texture as the color
//		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		// Set the location for binding the texture
//		glActiveTexture(GL_TEXTURE0);
		// Bind the texture
//		glBindTexture(GL_TEXTURE_2D, g_texture);

		// Render a single square as our geometry
		// You would normally render your geometry here
		
		

	// With shaders (no lighting)
	//
	} else {

		// Texture setup
		//

		// Enable Drawing texures
//		glEnable(GL_TEXTURE_2D);
		// Set the location for binding the texture
//		glActiveTexture(GL_TEXTURE0);
		// Bind the texture
	//	glBindTexture(GL_TEXTURE_2D, g_texture);

		// Use the shader we made
		glUseProgram(g_shader);

		// Set our sampler (texture0) to use GL_TEXTURE0 as the source
		glUniform1i(glGetUniformLocation(g_shader, "texture0"), 0);


		// Render a single square as our geometry
		// You would normally render your geometry here
		loadFiles();

		// Unbind our shader
		glUseProgram(0);

	}


	// Disable flags for cleanup (optional)
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
	
	

	// Move the buffer we just drew to the front
	glutSwapBuffers();

	// Queue the next frame to be drawn straight away
	glutPostRedisplay();
}


// Reshape function
// 
void reshape(int w, int h) {
    if (h == 0) h = 1;

	g_winWidth = w;
	g_winHeight = h;
    
    // Sets the openGL rendering window to match the window size
    glViewport(0, 0, g_winWidth, g_winHeight);  
}


// Keyboard callback
// Called once per button state change
//
void keyboardCallback(unsigned char key, int x, int y) {
	cout << "Keyboard Callback :: key=" << key << ", x,y=(" << x << "," << y << ")" << endl;
	if (key == 'a' || key == 'A') {
		if (ambientLight) ambientLight = false;
		else ambientLight = true;
	}
	if (key == 'p' || key == 'P') {
		if (pointLight) pointLight = false;
		else pointLight = true;
	}
	if (key == 's' || key == 'S') {
		if (spotlight) spotlight = false;
		else spotlight = true;
	}
	if (key == 'd' || key == 'D') {
		if (directionalLight) directionalLight = false;
		else directionalLight = true;
	}
	if (key == 't' || key == 'T') {
		// rotate scene 360d
		rotationDegrees = 0; // reset rotation
		if (rotateOn == false) rotateOn = true;
		else rotateOn = false;
	}
	if (key == '+' && cutoff < 180) {
		cutoff += 5;

	}
	if (key == '-' && cutoff < 180) {
		cutoff -= 5;

	}
	if (key == ',') {
		spotPosY -= 0.5;

	}
	if (key == '.' && cutoff > 0) {
		spotPosY += 0.5f;

	}
	if (key == '8') {
		spotDirZ -= 0.3f;
		coneAngle -= 3.0f;
	}
	if (key == '2') {
		spotDirZ += 0.3f;
		coneAngle += 3.0f;
	}
	if (key == '4') {
		spotDirX -= 0.3f;
		coneAngle -= 3.0f;
	}
	if (key == '6') {
		spotDirX += 0.3f;
		coneAngle += 3.0f;
	}
}



// Special Keyboard callback
// Called once per button state change
//
void specialCallback(int key, int x, int y) {
	cout << "Special Callback :: key=" << key << ", x,y=(" << x << "," << y << ")" << endl;
	
	if (key == 101) {
		spotPosZ -= 1.0f;
		cout << "spotlight Z :" << spotPosZ;
	}
	if (key == 103) {
		spotPosZ += 1.0f;
		cout << "spotlight Z :" << spotPosZ;
	}
	if (key == 100) {
		spotPosX -= 1.0f;
		cout << "spotlight X :" << spotPosX;
	}
	if (key == 102) {
		spotPosX += 1.0f;
		cout << "spotlight X :" << spotPosX;
	}
}


// Mouse Button Callback function
// Called once per button state change
// 
void mouseCallback(int button, int state, int x, int y) {
	cout << "Mouse Callback :: button=" << button << ", state=" << state << ", x,y=(" << x << "," << y << ")" << endl;
	// YOUR CODE GOES HERE
	// ...
	switch(button){

		case 0: // left mouse button
			g_mouseDown = (state==0);
			g_mousePos = vec2(x, y);
			break;

		case 2: // right mouse button
			if (state==0)
				g_useShader = !g_useShader;
			break;

		case 3: // scroll foward/up
			g_zoomFactor /= 1.1;
			break;

		case 4: // scroll back/down
			g_zoomFactor *= 1.1;
			break;
	}
}


// Mouse Motion Callback function
// Called once per frame if the mouse has moved and
// at least one mouse button has an active state
// 
void mouseMotionCallback(int x, int y) {
	cout << "Mouse Motion Callback :: x,y=(" << x << "," << y << ")" << endl;
	// YOUR CODE GOES HERE
	// ...
	if (g_mouseDown) {
		vec2 dif = vec2(x,y) - g_mousePos;
		g_mousePos = vec2(x,y);
		g_yRotation += 0.3 * dif.x;
		g_xRotation += 0.3 * dif.y;
	}
}




//Main program
// 
int main(int argc, char **argv) {

	if(argc != 1){
		cout << "No arguments expected" << endl;
		exit(EXIT_FAILURE);
	}

	// Initialise GL, GLU and GLUT
	glutInit(&argc, argv);

	// Setting up the display
	// - RGB color model + Alpha Channel = GLUT_RGBA
	// - Double buffered = GLUT_DOUBLE
	// - Depth buffer = GLUT_DEPTH
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);

	// Initialise window size and create window
	glutInitWindowSize(g_winWidth, g_winHeight);
	g_mainWindow = glutCreateWindow("COMP308 Assignment 3");


	// Initilise GLEW
	// must be done after creating GL context (glutCreateWindow in this case)
	GLenum err = glewInit();
	if (GLEW_OK != err) { // Problem: glewInit failed, something is seriously wrong.
		cerr << "Error: " << glewGetErrorString(err) << endl;
		abort(); // Unrecoverable error
	}

	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;



	// Register functions for callback
	glutDisplayFunc(draw);
	glutReshapeFunc(reshape);
	
	glutKeyboardFunc(keyboardCallback);
	glutSpecialFunc(specialCallback);

	glutMouseFunc(mouseCallback);
	glutMotionFunc(mouseMotionCallback);

	// create image objects
	torus = new Geometry("work/res/assets/torus.obj", "noTexture", 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.7f, 0.6f, 0.6f, 0.8f);
	sphere = new Geometry("work/res/assets/sphere.obj", "noTexture", 0.2125, 0.1275, 0.054, 0.714, 0.4284, 0.18144, 0.393548, 0.271906, 0.166721, 0.2);
	bunny = new Geometry("work/res/assets/bunny.obj", "noTexture", 0.7f, 0.7f, 0.7f, 0.55f, 0.55f, 0.55f, 0.7f, 0.7f, 0.7f, 0.95f);
	box = new Geometry("work/res/assets/box.obj", "work/res/textures/brick.jpg");
	table = new Geometry("work/res/assets/table.obj", "work/res/textures/wood.jpg");
	teapot = new Geometry("work/res/assets/teapot.obj", "noTexture", 0.25f, 0.25f, 0.75f, 0.4f, 0.4f, 0.8f, 0.774597, 0.774597, 0.774597, 0.6);
	
	q = gluNewQuadric();

	// Loop required by GLUT
	// This will not return until we tell GLUT to finish
	glutMainLoop();

	// Don't forget to delete all pointers that we made
	return 0;
}