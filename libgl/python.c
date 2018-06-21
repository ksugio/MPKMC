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

#endif /* MP_PYTHON_LIB */