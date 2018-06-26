#include "MPGLKMC.h"
#include <GL/freeglut.h>

static MP_KMCData *Data;
static MPGL_KMCDraw Draw;
static MPGL_Colormap Colormap;
static MPGL_Scene Scene;
static MPGL_Model Model;

static void DisplayFunc(void)
{
	char s[32];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	MPGL_ModelTransform(&Model);
	MPGL_KMCDrawTransform(Data);
	MPGL_KMCDrawAtoms(&Draw, Data, &Colormap);
	MPGL_KMCDrawFrame(&Draw, Data);
	glTranslatef(-0.3f, -0.3f, -0.3f);
	MPGL_KMCDrawAxis(&Draw, Data->size, 0.1);
	glPopMatrix();
	/* colormap draw */
	glPushMatrix();
	glRotatef(90.0, 0.0, 0.0, 1.0);
	glRotatef(90.0, 1.0, 0.0, 0.0);
	glTranslated((2.0 - Scene.width) / Scene.height, -Colormap.size[1] / 2, Scene.znear - 1.0e-6);
	MPGL_ColormapDraw(&Colormap);
	glPopMatrix();
	/* step */
	sprintf(s, "%d step", Data->step);
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glColor3fv(Colormap.font_color);
	MPGL_SceneFrontText(&Scene, 10, 20, s, Colormap.font_type);
	glPopAttrib();
	glutSwapBuffers();
}

static void ReshapeFunc(int width, int height)
{
	MPGL_SceneResize(&Scene, width, height);
}

static void MouseFunc(int button, int state, int x, int y)
{
	if (button ==  GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			MPGL_ModelButton(&Model, x, y, TRUE);
		}
		else if (state == GLUT_UP) {
			MPGL_ModelButton(&Model, x, y, FALSE);
		}
	}	
}

static void MotionFunc(int x, int y)
{
	int ctrl;

	if (glutGetModifiers() == GLUT_ACTIVE_CTRL) ctrl = TRUE;
	else ctrl = FALSE;
	if (MPGL_ModelMotion(&Model, &Scene, x, y, ctrl)) {
		glutPostRedisplay();
	}
}

static void SubMenu1(int value)
{
	if (value == 1) Model.button_mode = MPGL_ModelModeRotate;
	else if (value == 2) Model.button_mode = MPGL_ModelModeTranslate;
	else if (value == 3) Model.button_mode = MPGL_ModelModeZoom;
}

static void SubMenu2(int value)
{
	if (value == 1) {
		MPGL_ModelFit(&Model);
		glutPostRedisplay();
	}
	else if (value == 2) {
		MPGL_ModelReset(&Model);
		glutPostRedisplay();
	}
}

static void SubMenu3(int value)
{
//	if (value == 1) Draw->kind  = MPGL_DrawKindType;
//	else if (value == 2) Draw->kind = MPGL_DrawKindUpdate;
//	else if (value == 3) Draw->kind = MPGL_DrawKindVal;
	glutPostRedisplay();
}

void GlutWindow(MP_KMCData *data, int width, int height, int argc, char **argv)
{
	int sub1, sub2;
	float init_rot[] = { 0.0, 0.0, 0.0 };
	float region[6];

	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("MPGL_GridWindow");
	glutDisplayFunc(DisplayFunc);
	glutReshapeFunc(ReshapeFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
	MPGL_KMCDrawInit(&Draw);
	MPGL_ColormapInit(&Colormap);
	MPGL_SceneInit(&Scene);
	MPGL_SceneLightAdd(&Scene, 1.0, 1.0, 1.0, 0.0);
	Scene.proj = 0;
	MPGL_SceneSetup(&Scene);
	MPGL_KMCDrawAtomsRegion(data, region);
	MPGL_ModelInit(&Model, init_rot, region);
	Data = data;
	// menu
	sub1 = glutCreateMenu(SubMenu1);
	glutAddMenuEntry("Rotate", 1);
	glutAddMenuEntry("Translate", 2);
	glutAddMenuEntry("Zoom", 3);
	sub2 = glutCreateMenu(SubMenu2);
	glutAddMenuEntry("Fit", 1);
	glutAddMenuEntry("Reset", 2);
/*	sub3 = glutCreateMenu(SubMenu3);
	glutAddMenuEntry("Type", 1);
	glutAddMenuEntry("Updata", 2);
	glutAddMenuEntry("Value", 3);*/
	glutCreateMenu(NULL);
	glutAddSubMenu("Mouse", sub1);
	glutAddSubMenu("Model", sub2);
//	glutAddSubMenu("Kind", sub3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	// main loop
	glutMainLoop();
}

