#ifdef MP_PYTHON_LIB

#include "MPGLKMC.h"
#include <numpy/arrayobject.h>

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
	{ "text_bitmap", (PyCFunction)PyKMCTextBitmap, METH_VARARGS | METH_KEYWORDS,
	"text_bitmap(string, font_type) : draw text" },
	{ NULL }  /* Sentinel */
};

#ifdef PY3
static struct PyModuleDef MPGLKMCPyModule = {
	PyModuleDef_HEAD_INIT,
	"MPGLKMC",
	NULL,
	-1,
	MPGLKMCPyMethods,
};
#endif

#ifndef PY3
PyMODINIT_FUNC initMPGLKMC(void)
#else
PyMODINIT_FUNC PyInit_MPGLKMC(void)
#endif
{
	PyObject *m;

#ifndef PY3
	if (PyType_Ready(&MPGL_KMCDrawDataPyType) < 0) return;
	if (PyType_Ready(&MPGL_ModelPyType) < 0) return;
	if (PyType_Ready(&MPGL_ColormapPyType) < 0) return;
	if (PyType_Ready(&MPGL_ScenePyType) < 0) return;
	m = Py_InitModule3("MPGLKMC", MPGLKMCPyMethods, "MPGLKMC extention");
	if (m == NULL) return;
#else
	if (PyType_Ready(&MPGL_KMCDrawDataPyType) < 0) return NULL;
	if (PyType_Ready(&MPGL_ModelPyType) < 0) return NULL;
	if (PyType_Ready(&MPGL_ColormapPyType) < 0) return NULL;
	if (PyType_Ready(&MPGL_ScenePyType) < 0) return NULL;
	m = PyModule_Create(&MPGLKMCPyModule);
	if (m == NULL) return NULL;
#endif
	import_array();
	Py_INCREF(&MPGL_KMCDrawDataPyType);
	PyModule_AddObject(m, "draw", (PyObject *)&MPGL_KMCDrawDataPyType);
	Py_INCREF(&MPGL_ModelPyType);
	PyModule_AddObject(m, "model", (PyObject *)&MPGL_ModelPyType);
	Py_INCREF(&MPGL_ColormapPyType);
	PyModule_AddObject(m, "colormap", (PyObject *)&MPGL_ColormapPyType);
	Py_INCREF(&MPGL_ScenePyType);
	PyModule_AddObject(m, "scene", (PyObject *)&MPGL_ScenePyType);
#ifdef PY3
	return m;
#endif
}

#endif /* MP_PYTHON_LIB */