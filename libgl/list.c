#include "MPGLGrid.h"

void MPGL_ListSphere(unsigned int list, unsigned int res)
{
	GLUquadricObj *quadObj = gluNewQuadric();

	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricOrientation(quadObj, GLU_OUTSIDE);
	gluQuadricNormals(quadObj, GLU_SMOOTH);
	if (glIsList(list) == GL_TRUE) glDeleteLists(list, 1);
	glNewList(list, GL_COMPILE);
	gluSphere(quadObj, 1.0, res, res);
	glEndList();
	gluDeleteQuadric(quadObj);
}

void MPGL_ListCube(unsigned int list)
{
	static GLfloat vertices[8][3] =
	{
		{ -0.5f, -0.5f,  0.5f },{ 0.5f, -0.5f,  0.5f },{ 0.5f,  0.5f,  0.5f },{ -0.5f,  0.5f,  0.5f },
		{ 0.5f, -0.5f, -0.5f },{ -0.5f, -0.5f, -0.5f },{ -0.5f,  0.5f, -0.5f },{ 0.5f,  0.5f, -0.5f }
	};
	static GLfloat normals[6][3] =
	{
		{ 0.0f,  0.0f,  1.0f },{ 0.0f,  0.0f, -1.0f },{ 1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },{ 0.0f,  1.0f,  0.0f },{ 0.0f, -1.0f,  0.0f }
	};

	if (glIsList(list) == GL_TRUE) glDeleteLists(list, 1);
	glNewList(list, GL_COMPILE);
	glBegin(GL_POLYGON);
	glNormal3fv(normals[0]);
	glVertex3fv(vertices[0]);
	glVertex3fv(vertices[1]);
	glVertex3fv(vertices[2]);
	glVertex3fv(vertices[3]);
	glEnd();
	glBegin(GL_POLYGON);
	glNormal3fv(normals[1]);
	glVertex3fv(vertices[4]);
	glVertex3fv(vertices[5]);
	glVertex3fv(vertices[6]);
	glVertex3fv(vertices[7]);
	glEnd();
	glBegin(GL_POLYGON);
	glNormal3fv(normals[2]);
	glVertex3fv(vertices[1]);
	glVertex3fv(vertices[4]);
	glVertex3fv(vertices[7]);
	glVertex3fv(vertices[2]);
	glEnd();
	glBegin(GL_POLYGON);
	glNormal3fv(normals[3]);
	glVertex3fv(vertices[5]);
	glVertex3fv(vertices[0]);
	glVertex3fv(vertices[3]);
	glVertex3fv(vertices[6]);
	glEnd();
	glBegin(GL_POLYGON);
	glNormal3fv(normals[4]);
	glVertex3fv(vertices[3]);
	glVertex3fv(vertices[2]);
	glVertex3fv(vertices[7]);
	glVertex3fv(vertices[6]);
	glEnd();
	glBegin(GL_POLYGON);
	glNormal3fv(normals[5]);
	glVertex3fv(vertices[1]);
	glVertex3fv(vertices[0]);
	glVertex3fv(vertices[5]);
	glVertex3fv(vertices[4]);
	glEnd();
	glEndList();
}

void MPGL_ListCylinder(unsigned int list, unsigned int res)
{
	GLUquadricObj *quadObj = gluNewQuadric();

	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricOrientation(quadObj, GLU_OUTSIDE);
	gluQuadricNormals(quadObj, GLU_SMOOTH);
	if (glIsList(list) == GL_TRUE) glDeleteLists(list, 1);
	glNewList(list, GL_COMPILE);
	gluCylinder(quadObj, 1.0, 1.0, 1.0, res, res);
	glTranslatef(0.0, 0.0, 1.0);
	gluDisk(quadObj, 0.0, 1.0, res, res);
	glTranslatef(0.0, 0.0, -1.0);
	glRotatef(180, 1, 0, 0);
	gluDisk(quadObj, 0.0, 1.0, res, res);
	glEndList();
	gluDeleteQuadric(quadObj);
}

void MPGL_ListCone(unsigned int list, unsigned int res)
{
	GLUquadricObj *quadObj = gluNewQuadric();

	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricOrientation(quadObj, GLU_OUTSIDE);
	gluQuadricNormals(quadObj, GLU_SMOOTH);
	if (glIsList(list) == GL_TRUE) glDeleteLists(list, 1);
	glNewList(list, GL_COMPILE);
	gluCylinder(quadObj, 1.0, 0.0, 1.0, res, res);
	glRotatef(180, 1, 0, 0);
	gluDisk(quadObj, 0.0, 1.0, res, res);
	glEndList();
}

void MPGL_ListDisk(unsigned int list, unsigned int res)
{
	GLUquadricObj *quadObj = gluNewQuadric();

	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricOrientation(quadObj, GLU_OUTSIDE);
	gluQuadricNormals(quadObj, GLU_SMOOTH);
	if (glIsList(list) == GL_TRUE) glDeleteLists(list, 1);
	glNewList(list, GL_COMPILE);
	gluDisk(quadObj, 0.0, 1.0, res, res);
	glEndList();
}

void MPGL_ListWireCube(unsigned int list)
{
	int i, j;
	double pos[6];
	static int frame[12][6] = { { 0,0,0,1,0,0 },{ 1,0,0,1,1,0 },{ 1,0,0,1,0,1 },
	{ 0,0,0,0,1,0 },{ 0,1,0,1,1,0 },{ 0,1,0,0,1,1 },
	{ 0,0,0,0,0,1 },{ 0,0,1,1,0,1 },{ 0,0,1,0,1,1 },
	{ 1,1,1,0,1,1 },{ 1,1,1,1,0,1 },{ 1,1,1,1,1,0 } };

	if (glIsList(list) == GL_TRUE) glDeleteLists(list, 1);
	glNewList(list, GL_COMPILE);
	glBegin(GL_LINES);
	for (i = 0; i < 12; i++) {
		for (j = 0; j < 3; j++) {
			pos[j] = (double)frame[i][j] - 0.5;
			pos[j + 3] = (double)frame[i][j + 3] - 0.5;
		}
		glVertex3d(pos[0], pos[1], pos[2]);
		glVertex3d(pos[3], pos[4], pos[5]);
	}
	glEnd();
	glEndList();
}
