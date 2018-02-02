#include "MPGLKMC.h"

static MP_KMCData *Data;
static MPGL_KMCDraw *Draw;
static MPGL_Colormap *Colormap;
static MPGL_Scene *Scene;
static MPGL_Model *Model;

static unsigned char *CaptureBuffer;

static void DisplayFunc(void)
{
	MPGL_KMCDrawDisplay(Draw, Data, Colormap, Scene, Model);
	glutSwapBuffers();
}

static void ReshapeFunc(int width, int height)
{
	MPGL_KMCDrawReshape(Draw, Scene, width, height);
}

static void MouseFunc(int button, int state, int x, int y)
{
	if (button ==  GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			MPGL_KMCDrawButton(Draw, Model, x, y, TRUE);
		}
		else if (state == GLUT_UP) {
			MPGL_KMCDrawButton(Draw, Model, x, y, FALSE);
		}
	}
}

static void MotionFunc(int x, int y)
{
	if (glutGetModifiers() == GLUT_ACTIVE_CTRL) {
		MPGL_KMCDrawMotion(Draw, Model, x, y, TRUE);
	}
	else {
		MPGL_KMCDrawMotion(Draw, Model, x, y, FALSE);
	}
	glutPostRedisplay();
}

/*static void KeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'f':
	case 'F':
		MP_KMCStepForward(Data);
		glutPostRedisplay();
		break;
	case 'b':
	case 'B':
		MP_KMCStepBackward(Data);
		glutPostRedisplay();
		break;
	default:
		break;
	}
}*/

static void SubMenu1(int value)
{
	if (value == 1) Draw->button_mode = MPGL_KMCModeRotate;
	else if (value == 2) Draw->button_mode = MPGL_KMCModeTranslate;
	else if (value == 3) Draw->button_mode = MPGL_KMCModeZoom;
}

static void SubMenu2(int value)
{
	if (value == 1) {
		MPGL_KMCDrawFit(Draw, Data, Model);
		glutPostRedisplay();
	}
	else if (value == 2) {
		MPGL_ModelInit(Model);
		glutPostRedisplay();
	}
}

static void SubMenu3(int value)
{
	if (value == 1) Draw->kind = MPGL_KMCKindType;
	else if (value == 2) Draw->kind = MPGL_KMCKindEnergy;
	glutPostRedisplay();
}

void MPGL_KMCWindow(MP_KMCData *data, MPGL_KMCDraw *draw,
	MPGL_Colormap *colormap, MPGL_Scene *scene, MPGL_Model *model,
	int width, int height, void(*func)(void), int argc, char **argv)
{
	int sub1, sub2, sub3;

	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("MPGL_KMCWindow");
	glutDisplayFunc(DisplayFunc);
	glutReshapeFunc(ReshapeFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
	//glutKeyboardFunc(KeyboardFunc);
	if (func != NULL) glutIdleFunc(func);
	MPGL_SceneSetup(scene);
	// menu
	sub1 = glutCreateMenu(SubMenu1);
	glutAddMenuEntry("Rotate", 1);
	glutAddMenuEntry("Translate", 2);
	glutAddMenuEntry("Zoom", 3);
	sub2 = glutCreateMenu(SubMenu2);
	glutAddMenuEntry("Fit", 1);
	glutAddMenuEntry("Init", 2);
	sub3 = glutCreateMenu(SubMenu3);
	glutAddMenuEntry("Type", 1);
	glutAddMenuEntry("Energy", 2);
	glutCreateMenu(NULL);
	glutAddSubMenu("Mouse", sub1);
	glutAddSubMenu("Transform", sub2);
	glutAddSubMenu("Kind", sub3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	// main loop
	Data = data;
	Draw = draw;
	Colormap = colormap;
	Scene = scene;
	Model = model;
	glutMainLoop();
}

void CaptureFunc(void)
{
	glReadPixels(0, 0, Draw->width, Draw->height, GL_RGB, GL_UNSIGNED_BYTE, CaptureBuffer);
	glutLeaveMainLoop();
}

void MPGL_KMCImage(MP_KMCData *data, MPGL_KMCDraw *draw,
	MPGL_Colormap *colormap, MPGL_Scene *scene, MPGL_Model *model,
	int width, int height, unsigned char *buffer, int argc, char **argv)
{
	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("MPGL_KMCImage");
	glutDisplayFunc(DisplayFunc);
	glutReshapeFunc(ReshapeFunc);
	glutIdleFunc(CaptureFunc);
	MPGL_SceneSetup(scene);
	// set pointer
	Data = data;
	Draw = draw;
	Colormap = colormap;
	Scene = scene;
	Model = model;
	CaptureBuffer = buffer;
	// main loop
	glutMainLoop();
}
