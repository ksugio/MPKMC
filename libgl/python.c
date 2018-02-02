#ifndef _DEBUG

#include "MPGLKMC.h"
#include <numpy/arrayobject.h>

static PyObject *FuncObj;

static void IdleFunc(void)
{
	PyObject_CallObject(FuncObj, NULL);
}

/*
static PyObject *PyKMCWindow(PyObject *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	MPGL_KMCDraw *draw;
	int width, height;
	PyObject *func = NULL;
	static char *kwlist[] = { "kmc", "draw", "width", "height", "func", NULL };
	int argc = 0;
	char *argv[1];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO!ii|O", kwlist, &data, &MPGL_KMCDrawDataPyType, &draw,
		&width, &height, &PyTuple_Type, &func)) {
		return NULL;
	}
	if (func == NULL) {
		MPGL_KMCWindow(data, draw, width, height, NULL, argc, argv);
	}
	else {
		if (!PyCallable_Check(func)) {
			PyErr_SetString(PyExc_TypeError, "func must be callable");
			return NULL;
		}
		FuncObj = func;
		MPGL_KMCWindow(data, draw, width, height, IdleFunc, argc, argv);
	}
	Py_RETURN_NONE;
}

static PyObject *PyKMCImage(PyObject *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	MPGL_KMCDraw *draw;
	int width, height;
	static char *kwlist[] = { "kmc", "draw", "width", "height", NULL };
	npy_intp dims[1];
	PyArrayObject *buffer_arr;
	unsigned char *buffer;
	int argc = 0;
	char *argv[1];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO!ii", kwlist,
		&data, &MPGL_KMCDrawDataPyType, &draw, &width, &height)) {
		return NULL;
	}
	dims[0] = 3 * width * height;
	buffer_arr = (PyArrayObject *)PyArray_SimpleNew(1, dims, NPY_UBYTE);
	buffer = (unsigned char *)PyArray_DATA(buffer_arr);
	MPGL_KMCImage(data, draw, width, height, buffer, argc, argv);
	return (PyObject *)buffer_arr;
}*/

static PyObject *PyKMCPostRedisplay(PyObject *self, PyObject *args)
{
	glutPostRedisplay();
	Py_RETURN_NONE;
}

static PyObject *PyKMCTextBitmap(PyObject *self, PyObject *args, PyObject *kwds)
{
	const char *string;
	int font_type;
	static char *kwlist[] = { "string", "font_type", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "si", kwlist, &string, &font_type)) {
		return NULL;
	}
	MPGL_TextBitmap(string, font_type);
	Py_RETURN_NONE;
}

static PyMethodDef MPGLKMCPyMethods[] = {
//	{ "window", (PyCFunction)PyKMCWindow, METH_VARARGS | METH_KEYWORDS,
//	"window(kmc, draw, width, height, [func]) : create window for kmc data" },
//	{ "image", (PyCFunction)PyKMCImage, METH_VARARGS | METH_KEYWORDS,
//	"image(kmc, draw, width, height) : create image for kmc data" },
	{ "post_redisplay", (PyCFunction)PyKMCPostRedisplay, METH_NOARGS,
	"post_redisplay() : post_redisplay" },
	{ "text_bitmap", (PyCFunction)PyKMCTextBitmap, METH_VARARGS | METH_KEYWORDS,
	"text_bitmap(string, font_type) : draw text" },
	{ NULL }  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC initMPGLKMC(void)
{
	PyObject *m;

	if (PyType_Ready(&MPGL_KMCDrawDataPyType) < 0) return;
	if (PyType_Ready(&MPGL_ModelPyType) < 0) return;
	if (PyType_Ready(&MPGL_ColormapPyType) < 0) return;
	if (PyType_Ready(&MPGL_ScenePyType) < 0) return;
	m = Py_InitModule3("MPGLKMC", MPGLKMCPyMethods, "MPGLKMC extention");
	if (m == NULL) return;
	import_array();
	Py_INCREF(&MPGL_KMCDrawDataPyType);
	PyModule_AddObject(m, "draw", (PyObject *)&MPGL_KMCDrawDataPyType);
	Py_INCREF(&MPGL_ModelPyType);
	PyModule_AddObject(m, "model", (PyObject *)&MPGL_ModelPyType);
	Py_INCREF(&MPGL_ColormapPyType);
	PyModule_AddObject(m, "colormap", (PyObject *)&MPGL_ColormapPyType);
	Py_INCREF(&MPGL_ScenePyType);
	PyModule_AddObject(m, "scene", (PyObject *)&MPGL_ScenePyType);
}

#endif /* _DEBUG */