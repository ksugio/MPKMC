#include "MPGLKMC.h"
#include <GL/freeglut.h>

static MP_KMCData *Data;
static MPGL_KMCDraw Draw;
static MPGL_Colormap Colormap;
static MPGL_Scene Scene;
static MPGL_Model Model[2];

static int dispMode = 0;
static int tbID = 1;

static void DisplayFunc(void)
{
	char s[32];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	MPGL_ModelTransform(&Model[dispMode]);
	if (dispMode == 0) MPGL_KMCDrawAtoms(&Draw, Data, &Colormap);
	else if (dispMode == 1) MPGL_KMCDrawCluster(&Draw, Data, &Colormap, Data->table[tbID].types);
	glPopMatrix();
	/* colormap draw */
	glPushMatrix();
	glRotatef(90.0, 0.0, 0.0, 1.0);
	glRotatef(90.0, 1.0, 0.0, 0.0);
	glTranslated((2.0 - Scene.width) / Scene.height, -Colormap.size[1] / 2, Scene.znear - 1.0e-6);
	MPGL_ColormapDraw(&Colormap);
	glPopMatrix();
	/* step */
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glColor3fv(Colormap.font_color);
	sprintf(s, "%d MCS", Data->mcs);
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
			MPGL_ModelButton(&Model[dispMode], x, y, TRUE);
		}
		else if (state == GLUT_UP) {
			MPGL_ModelButton(&Model[dispMode], x, y, FALSE);
		}
	}	
}

static void MotionFunc(int x, int y)
{
	int ctrl;

	if (glutGetModifiers() == GLUT_ACTIVE_CTRL) ctrl = TRUE;
	else ctrl = FALSE;
	if (MPGL_ModelMotion(&Model[dispMode], &Scene, x, y, ctrl)) {
		glutPostRedisplay();
	}
}

static void SubMenu1(int value)
{
	if (value == 1) Model[dispMode].button_mode = MPGL_ModelModeRotate;
	else if (value == 2) Model[dispMode].button_mode = MPGL_ModelModeTranslate;
	else if (value == 3) Model[dispMode].button_mode = MPGL_ModelModeZoom;
}

static void SubMenu2(int value)
{
	if (value == 1) {
		MPGL_ModelFit(&Model[dispMode]);
		glutPostRedisplay();
	}
	else if (value == 2) {
		MPGL_ModelReset(&Model[dispMode]);
		glutPostRedisplay();
	}
}

static void SubMenu3(int value)
{
	if (value == 1) {
		Draw.kind = MPGL_KMCKindType;
		dispMode = 0;
	}
	else if (value == 2) {
		Draw.kind = MPGL_KMCKindEnergy;
		dispMode = 0;
	}
	else if (value == 3) {
		dispMode = 1;
	}
	glutPostRedisplay();
}

void GlutWindow(MP_KMCData *data, int width, int height, int argc, char **argv)
{
	int sub1, sub2, sub3;
	float init_dir[] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
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
	MPGL_ModelInit(&Model[0], init_dir, region);
	MPGL_KMCDrawClusterRegion(data, region);
	MPGL_ModelInit(&Model[1], init_dir, region);
	Data = data;
	// menu
	sub1 = glutCreateMenu(SubMenu1);
	glutAddMenuEntry("Rotate", 1);
	glutAddMenuEntry("Translate", 2);
	glutAddMenuEntry("Zoom", 3);
	sub2 = glutCreateMenu(SubMenu2);
	glutAddMenuEntry("Fit", 1);
	glutAddMenuEntry("Reset", 2);
	sub3 = glutCreateMenu(SubMenu3);
	glutAddMenuEntry("Type", 1);
	glutAddMenuEntry("Energy", 2);
	glutAddMenuEntry("Cluster", 3);
	glutCreateMenu(NULL);
	glutAddSubMenu("Mouse", sub1);
	glutAddSubMenu("Model", sub2);
	glutAddSubMenu("Kind", sub3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	// main loop
	glutMainLoop();
}

