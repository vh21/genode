/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * This is a port of the infamous "glxgears" demo to straight EGL
 * Port by Dane Rushton 10 July 2005
 *
 * Slightly modified by Norman Feske in July 2010
 */

#define EGL_EGLEXT_PROTOTYPES

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define MAX_CONFIGS 10
#define MAX_MODES 100


#ifndef M_PI
#define M_PI 3.14159265
#endif


static GLfloat view_rotx = 20.0, view_roty = 30.0, view_rotz = 0.0;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0;

#if 0
static GLfloat eyesep = 5.0;		/* Eye separation. */
static GLfloat fix_point = 40.0;	/* Fixation point distance.  */
static GLfloat left, right, asp;	/* Stereo frustum params.  */
#endif


/*
 *
 *  Draw a gear wheel.  You'll probably want to call this function when
 *  building a display list since we do a lot of trig here.
 * 
 *  Input:  inner_radius - radius of hole at center
 *          outer_radius - radius at center of teeth
 *          width - width of gear
 *          teeth - number of teeth
 *          tooth_depth - depth of tooth
 */
static void
gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
     GLint teeth, GLfloat tooth_depth)
{
	GLint i;
	GLfloat r0, r1, r2;
	GLfloat angle, da;
	GLfloat u, v, len;

	r0 = inner_radius;
	r1 = outer_radius - tooth_depth / 2.0;
	r2 = outer_radius + tooth_depth / 2.0;

	da = 2.0 * M_PI / teeth / 4.0;

	glShadeModel(GL_FLAT);

	glNormal3f(0.0, 0.0, 1.0);

	/* draw front face */
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;
		glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
		glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
		if (i < teeth) {
			glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
			glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
		}
	}
	glEnd();

	/* draw front sides of teeth */
	glBegin(GL_QUADS);
	da = 2.0 * M_PI / teeth / 4.0;
	for (i = 0; i < teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
	}
	glEnd();

	glNormal3f(0.0, 0.0, -1.0);

	/* draw back face */
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;
		glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
		if (i < teeth) {
			glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
			glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
		}
	}
	glEnd();

	/* draw back sides of teeth */
	glBegin(GL_QUADS);
	da = 2.0 * M_PI / teeth / 4.0;
	for (i = 0; i < teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
		glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
	}
	glEnd();

	/* draw outward faces of teeth */
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
		glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
		u = r2 * cos(angle + da) - r1 * cos(angle);
		v = r2 * sin(angle + da) - r1 * sin(angle);
		len = sqrt(u * u + v * v);
		u /= len;
		v /= len;
		glNormal3f(v, -u, 0.0);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
		glNormal3f(cos(angle), sin(angle), 0.0);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
		u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
		v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
		glNormal3f(v, -u, 0.0);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
		glNormal3f(cos(angle), sin(angle), 0.0);
	}

	glVertex3f(r1 * cos(0), r1 * sin(0), width * 0.5);
	glVertex3f(r1 * cos(0), r1 * sin(0), -width * 0.5);

	glEnd();

	glShadeModel(GL_SMOOTH);

	/* draw inside radius cylinder */
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;
		glNormal3f(-cos(angle), -sin(angle), 0.0);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
	}
	glEnd();
}


static void
draw(void)
{
	GLfloat a;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glRotatef(view_rotx, 1.0, 0.0, 0.0);
	glRotatef(view_roty, 0.0, 1.0, 0.0);
	glRotatef(view_rotz, 0.0, 0.0, 1.0);

	glPushMatrix();
	glTranslatef(-3.0, -2.0, 0.0);
	glRotatef(angle, 0.0, 0.0, 1.0);
	glCallList(gear1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(3.1, -2.0, 0.0);
	a = -2.0*angle - 9.0;
	while (a < 0) a += 360;
	glRotatef(a, 0.0, 0.0, 1.0);
	glCallList(gear2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-3.1, 4.2, 0.0);
	a = -2.0*angle - 25.0;
	while (a < 0) a += 360;
	glRotatef(a, 0.0, 0.0, 1.0);
	glCallList(gear3);
	glPopMatrix();

	glPopMatrix();
}


/* new window size or exposure */
static void
reshape(int width, int height)
{
	GLfloat h = (GLfloat) height / (GLfloat) width;

	glViewport(0, 0, (GLint) width, (GLint) height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -40.0);
}


static void
init(void)
{
	static GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };
	static GLfloat red[4] = { 1.5*0.8, 0.1, 0.0, 0.5 };
	static GLfloat green[4] = { 0.0, 1.5*0.8, 0.2, 0.5 };
	static GLfloat blue[4] = { 0.2, 0.2, 1.5*1.0, 0.5 };

	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	/* make the gears */
	gear1 = glGenLists(1);
	glNewList(gear1, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
	gear(1.0, 4.0, 1.0, 20, 0.7);
	glEndList();

	gear2 = glGenLists(1);
	glNewList(gear2, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
	gear(0.5, 2.0, 2.0, 10, 0.7);
	glEndList();

	gear3 = glGenLists(1);
	glNewList(gear3, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
	gear(1.3, 2.0, 0.5, 10, 0.7);
	glEndList();

	glEnable(GL_NORMALIZE);
}


static void run_gears(EGLDisplay dpy, EGLSurface surf, int ttr)
{
	while (1)
	{
		/* advance rotation for next frame */
		angle += 3;
		if (angle > 360)
			angle -= 360;

		view_rotx += 2;
		if (view_rotx > 360)
			view_rotx -= 360;

		view_roty += 1.1;
		if (view_roty > 360)
			view_roty -= 360;

		view_rotz += 0.77;
		if (view_rotz > 360)
			view_rotz -= 360;

		draw();

		eglSwapBuffers(dpy, surf);
	}
}


int
main(int argc, char *argv[])
{
	int maj, min;
	EGLContext ctx;
	EGLSurface screen_surf;
	EGLConfig configs[MAX_CONFIGS];
	EGLint numConfigs, i;
	EGLBoolean b;
	EGLDisplay d;
	EGLint screenAttribs[10];
	EGLModeMESA mode[MAX_MODES];
	EGLScreenMESA screen;
	EGLint count;
	EGLint chosenMode = 0;
	GLboolean printInfo = GL_FALSE;
	EGLint width = 0, height = 0;

	/* DBR : Create EGL context/surface etc */
	d = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(d);

	if (!eglInitialize(d, &maj, &min)) {
		printf("eglgears: eglInitialize failed\n");
		exit(1);
	}
	
	printf("eglgears: EGL version = %d.%d\n", maj, min);
	printf("eglgears: EGL_VENDOR = %s\n", eglQueryString(d, EGL_VENDOR));
	
        /* XXX use ChooseConfig */
	eglGetConfigs(d, configs, MAX_CONFIGS, &numConfigs);
	eglGetScreensMESA(d, &screen, 1, &count);

	if (!eglGetModesMESA(d, screen, mode, MAX_MODES, &count) || count == 0) {
		printf("eglgears: eglGetModesMESA failed!\n");
		return 0;
	}

        /* Print list of modes, and find the one to use */
	printf("eglgears: Found %d modes:\n", count);
	for (i = 0; i < count; i++) {
		EGLint w, h;
		eglGetModeAttribMESA(d, mode[i], EGL_WIDTH, &w);
		eglGetModeAttribMESA(d, mode[i], EGL_HEIGHT, &h);
		printf("%3d: %d x %d\n", i, w, h);
		if (w > width && h > height) {
			width = w;
			height = h;
                        chosenMode = i;
		}
	}
	printf("eglgears: Using screen mode/size %d: %d x %d\n", chosenMode, width, height);

	eglBindAPI(EGL_OPENGL_API);
	ctx = eglCreateContext(d, configs[0], EGL_NO_CONTEXT, NULL);
	if (ctx == EGL_NO_CONTEXT) {
		printf("eglgears: failed to create context\n");
		return 0;
	}
	
	/* build up screenAttribs array */
	i = 0;
	screenAttribs[i++] = EGL_WIDTH;
	screenAttribs[i++] = width;
	screenAttribs[i++] = EGL_HEIGHT;
	screenAttribs[i++] = height;
	screenAttribs[i++] = EGL_NONE;

	screen_surf = eglCreateScreenSurfaceMESA(d, configs[0], screenAttribs);
	if (screen_surf == EGL_NO_SURFACE) {
		printf("eglgears: failed to create screen surface\n");
		return 0;
	}
	printf("eglgears: returned from eglCreateScreenSurfaceMESA\n");
	
	b = eglShowScreenSurfaceMESA(d, screen, screen_surf, mode[chosenMode]);
	if (!b) {
		printf("eglgears: show surface failed\n");
		return 0;
	}
	printf("eglgears: returned from eglShowScreenSurfaceMESA\n");

	b = eglMakeCurrent(d, screen_surf, screen_surf, ctx);
	if (!b) {
		printf("eglgears: make current failed\n");
		return 0;
	}
	printf("eglgears: returned from eglMakeCurrent\n");
	
	if (printInfo)
	{
		printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
		printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
		printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
		printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
	}
	
	init();
	reshape(width, height);

        glDrawBuffer( GL_BACK );

	run_gears(d, screen_surf, 5.0);
	
	eglDestroySurface(d, screen_surf);
	eglDestroyContext(d, ctx);
	eglTerminate(d);
	
	return 0;
}
