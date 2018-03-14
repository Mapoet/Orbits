// GlutPointszDemo.cpp : 定义控制台应用程序的入口点。
//

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
#include <math.h>
//圆周率宏
#define GL_PI 3.1415f
//获取屏幕的宽度
GLint SCREEN_WIDTH = 0;
GLint SCREEN_HEIGHT = 0;
//设置程序的窗口大小
GLint windowWidth = 400;
GLint windowHeight = 300;
//绕x轴旋转角度
GLfloat xRotAngle = 0.0f;
//绕y轴旋转角度
GLfloat yRotAngle = 0.0f;
//显示回调函数
//受支持的点大小范围
GLfloat sizes[2];
//受支持的点大小增量
GLfloat step;
void renderScreen(void){
	GLfloat x, y, z, angle;
	//当前点的大小
	GLfloat curSize = 0.0f;
	//把整个窗口清理为当前清理颜色：黑色
	glClear(GL_COLOR_BUFFER_BIT);
	//将当前Matrix状态如栈
	glPushMatrix();
	//坐标系绕x轴旋转xRotAngle
	glRotatef(xRotAngle, 1.0f, 0.0f, 0.0f);
	//坐标系绕y轴旋转yRotAngle
	glRotatef(yRotAngle, 0.0f, 1.0f, 0.0f);
	//当前点的大小
	curSize = sizes[0];
	z = -50.0f;
	//绘制四个螺纹
	for (angle = 0.0f; angle <= ((2.0f*GL_PI)*4.0f); angle += 0.1f){
		x = 50.0f*cos(angle);
		y = 50.0f*sin(angle);
		//设置点的大小
		glPointSize(curSize);
		//进行平滑处理　
		glEnable(GL_POINT_SMOOTH);
		glHint(GL_POINT_SMOOTH, GL_NICEST);
		//开始绘点
		glBegin(GL_POINTS);
		glVertex3f(x, y, z);
		//结束绘点
		glEnd();
		z += 0.4f;
		//点变大
		curSize += step;
	}
	//恢复压入栈的Matrix
	glPopMatrix();
	//交换两个缓冲区的指针
	glutSwapBuffers();
}
//设置Redering State 
void setupRederingState(void){
	//设置清理颜色为白色
	glClearColor(0.0f, 0.0, 0.0, 1.0f);
	//设置绘画颜色为红色
	glColor3f(0.0f, 1.0f, 0.0f);
	//获取受支持的点大小范围
	glGetFloatv(GL_POINT_SIZE_RANGE, sizes);
	//获取受支持的点大小增量
	glGetFloatv(GL_POINT_SIZE_GRANULARITY, &step);
	printf("point size range:%f-%f\n", sizes[0], sizes[1]);
	printf("point step:%f\n", step);
}
//窗口大小变化回调函数
void changSize(GLint w, GLint h){
	//横宽比率
	GLfloat ratio;
	//设置坐标系为x(-100.0f,100.0f)、y(-100.0f,100.0f)、z(-100.0f,100.0f)
	GLfloat coordinatesize = 100.0f;
	//窗口宽高为零直接返回
	if ((w == 0) || (h == 0))
		return;
	//设置视口和窗口大小一致
	glViewport(0, 0, w, h);
	//对投影矩阵应用随后的矩阵操作
	glMatrixMode(GL_PROJECTION);
	//重置当前指定的矩阵为单位矩阵　
	glLoadIdentity();
	ratio = (GLfloat)w / (GLfloat)h;
	//正交投影
	if (w<h)
		glOrtho(-coordinatesize, coordinatesize, -coordinatesize / ratio, coordinatesize / ratio, -coordinatesize, coordinatesize);
	else
		glOrtho(-coordinatesize*ratio, coordinatesize*ratio, -coordinatesize, coordinatesize, -coordinatesize, coordinatesize);
	//对模型视图矩阵堆栈应用随后的矩阵操作
	glMatrixMode(GL_MODELVIEW);
	//重置当前指定的矩阵为单位矩阵　
	glLoadIdentity();
}

//按键输入处理函数
void sPecialkeyFuc(int key, int x, int y){
	if (key == GLUT_KEY_UP){
		xRotAngle -= 5.0f;
	}
	else if (key == GLUT_KEY_DOWN){
		xRotAngle += 5.0f;
	}
	else if (key == GLUT_KEY_LEFT){
		yRotAngle -= 5.0f;
	}
	else if (key == GLUT_KEY_RIGHT){
		yRotAngle += 5.0f;
	}
	//重新绘制
	glutPostRedisplay();
}

int testmain(int argc, char* argv[])
{
	glutInit(&argc, argv);
	//使用双缓冲区模式
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	//获取系统的宽像素
	SCREEN_WIDTH = glutGet(GLUT_SCREEN_WIDTH);
	//获取系统的高像素
	SCREEN_HEIGHT = glutGet(GLUT_SCREEN_HEIGHT);
	//创建窗口，窗口名字为PointsDemo
	glutCreateWindow("PointsDemo");
	//设置窗口大小
	glutReshapeWindow(windowWidth, windowHeight);
	//窗口居中显示
	glutPositionWindow((SCREEN_WIDTH - windowWidth) / 2, (SCREEN_HEIGHT - windowHeight) / 2);
	//设置窗口大小变化时的回调函数
	glutReshapeFunc(changSize);
	//设置显示回调函数 
	glutDisplayFunc(renderScreen);
	//设置按键输入处理回调函数
	glutSpecialFunc(sPecialkeyFuc);
	//设置全局渲染参数
	setupRederingState();
	glutMainLoop();
	return 0;
}