
#ifndef __CONTROLS_H__
#define __CONTROLS_H__
#include <stdlib.h>
#include <stdio.h>
#ifndef _WINDOWS_
#include <windows.h>
#include <math.h>
#endif
#ifndef __GL_H__
#include "GL.h"
#endif
#ifndef __GLU_H__
#include "GLU.h"
#endif
#ifndef __glut_h__
#include "GLUT.h"
#endif
#ifndef __GLAUX_H__
#include "GLAUX.h"
#endif
#pragma comment(lib,"OpenGL32.Lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glut32.lib")
#pragma comment(lib, "GLAUX.lib")

#ifndef __ORBIT_H__
#include "orbit.h"
#endif
typedef struct{
	Pos RotAngle;
	Pos EyesPos;
	Pos CenterPos;
	Pos UpPos;
	Pos TransPos;
	real DRot;
	real ViewAngle;
	int idnum;
	uint ids[NPLANET];
}ViewSet;
typedef struct{
	BOOL key[3], keyp[3];
	uint x, xp, y, yp;
	uint event;
}Mouse;
typedef struct{		//convert a paletted image into a non-paletted one

	uint bpp;				//bits per pixel
	uint width, height;		//size
	uint stride;			//bytes per row, including padding bytes
	GLuint format;					//format (eg GL_RGBA)
	//Data
	GLubyte * data;
	//palette data (256 entries)
	BOOL paletted;
	unsigned int paletteBpp;
	GLuint paletteFormat;
	GLubyte * palette;

}Bitmap;
#define MAX_CHAR       128

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
*	title			- Title To Appear At The Top Of The Window				*
*	width			- Width Of The GL Window Or Fullscreen Mode				*
*	height			- Height Of The GL Window Or Fullscreen Mode			*
*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
*	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
BOOL CreateGLWindow(char* title, int width, int height, int bits, BOOL fullscreenflag);
GLvoid ReSizeGLScene(GLsizei width, GLsizei height);
int InitGL(GLvoid);
GLvoid KillGLWindow(GLvoid);
int DrawGLScene(GLvoid);
GLvoid UpdateData(GLvoid);
void drawString(const char* str);
uint UpdateKeys(GLvoid);
GLvoid OutPutU(GLvoid);
BOOL LoadT8(char* filename, GLuint*texture);
BOOL readInitData();
#endif