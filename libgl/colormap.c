#include "MPGLKMC.h"

void MPGL_ColormapInit(MPGL_Colormap *colormap)
{
	MPGL_ColormapColor(colormap);
	colormap->nscale = 1;
	colormap->range[0] = 0.0, colormap->range[1] = 0.0;
	colormap->size[0] = 0.12f, colormap->size[1] = 1.6f;
	colormap->font_type = MPGL_TextHelvetica18;
	colormap->font_color[0] = 1.0;
	colormap->font_color[1] = 1.0;
	colormap->font_color[2] = 1.0;
}

void MPGL_ColormapColor(MPGL_Colormap *colormap)
{
	int i;
	static float step_color[16][3] = { { 0.0, 0.0, 1.0 },{ 0.0, 1.0, 1.0 },{ 1.0, 1.0, 0.0 },{ 1.0, 0.0, 0.0 },
	{ 1.0, 1.0, 1.0 },{ 0.5, 0.0, 0.0 },{ 0.5, 0.0, 0.5 },{ 1.0, 0.0, 1.0 },
	{ 0.0, 0.5, 0.0 },{ 0.0, 1.0, 0.0 },{ 0.5, 0.5, 0.0 },{ 0.0, 0.0, 0.5 },
	{ 0.0, 0.5, 0.5 },{ 0.5, 0.5, 0.5 },{ 0.75, 0.75, 0.75 },{ 0.0, 0.0, 0.0 } };
	static float grad_color[16][3] = { { 0.0, 0.0, 1.0 },{ 0.0, 1.0, 1.0 },{ 1.0, 1.0, 0.0 },{ 1.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 } };

	colormap->nstep = 16;
	for (i = 0; i < 16; i++) {
		colormap->step_color[i][0] = step_color[i][0];
		colormap->step_color[i][1] = step_color[i][1];
		colormap->step_color[i][2] = step_color[i][2];
	}
	colormap->ngrad = 4;
	for (i = 0; i < 16; i++) {
		colormap->grad_color[i][0] = grad_color[i][0];
		colormap->grad_color[i][1] = grad_color[i][1];
		colormap->grad_color[i][2] = grad_color[i][2];
	}
}

void MPGL_ColormapGrayscale(MPGL_Colormap *colormap)
{
	int i;
	static float step_color[16][3] = { { 0.9f, 0.9f, 0.9f },{ 0.6f, 0.6f, 0.6f },{ 0.3f, 0.3f, 0.3f },{ 0.8f, 0.8f, 0.8f },
	{ 0.5f, 0.5f, 0.5f },{ 0.2f, 0.2f, 0.2f },{ 0.7f, 0.7f, 0.7f },{ 0.4f, 0.4f, 0.4f },
	{ 0.1f, 0.1f, 0.1f },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 } };
	static float grad_color[16][3] = { { 0.1f, 0.1f, 0.1f },{ 0.5f, 0.5f, 0.5f },{ 0.9f, 0.9f, 0.9f },{ 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 },{ 0.0, 0.0, 0.0 } };

	colormap->nstep = 9;
	for (i = 0; i < 16; i++) {
		colormap->step_color[i][0] = step_color[i][0];
		colormap->step_color[i][1] = step_color[i][1];
		colormap->step_color[i][2] = step_color[i][2];
	}
	colormap->ngrad = 3;
	for (i = 0; i < 16; i++) {
		colormap->grad_color[i][0] = grad_color[i][0];
		colormap->grad_color[i][1] = grad_color[i][1];
		colormap->grad_color[i][2] = grad_color[i][2];
	}
}

void MPGL_ColormapStepColor(MPGL_Colormap *colormap, int id, float color[])
{
	if (id < colormap->nstep) {
		color[0] = colormap->step_color[id][0];
		color[1] = colormap->step_color[id][1];
		color[2] = colormap->step_color[id][2];
	}
	else {
		color[0] = 1.0, color[1] = 1.0, color[2] = 1.0;
	}
}

void MPGL_ColormapGradColor(MPGL_Colormap *colormap, double value, float color[])
{
	int cmp;
	double drange, dy;

	drange = (colormap->range[1]-colormap->range[0])/(float)(colormap->ngrad-1);
	cmp = (int)((value-colormap->range[0])/drange);
	if (cmp >= colormap->ngrad-1) {
		cmp = colormap->ngrad-2;
		dy = 1.0;
	}
	else if (cmp < 0) {
		cmp = 0;
		dy = 0.0;
	}
	else {
		dy = (value-colormap->range[0]-cmp*drange)/drange;
	}
	color[0] = (float)(colormap->grad_color[cmp][0] + dy*(colormap->grad_color[cmp+1][0]-colormap->grad_color[cmp][0]));
	color[1] = (float)(colormap->grad_color[cmp][1] + dy*(colormap->grad_color[cmp+1][1]-colormap->grad_color[cmp][1]));
	color[2] = (float)(colormap->grad_color[cmp][2] + dy*(colormap->grad_color[cmp+1][2]-colormap->grad_color[cmp][2]));
}

void MPGL_ColormapDraw(MPGL_Colormap *colormap)
{
	int i;
	float dh;
	float pos[3], dy;
	char s[32];

	glScalef(colormap->size[0], colormap->size[1], 0.0f);
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	if (colormap->mode == MPGL_ColormapStep) {
		if (colormap->nstep == 0) return;
		dh = 1.0f/colormap->nstep;
		for (i = 0;i < colormap->nstep;i++) {
			glBegin(GL_QUADS);
			glColor3f(colormap->step_color[i][0], colormap->step_color[i][1], colormap->step_color[i][2]);
			pos[0] = 0.0, pos[1] = dh*i, pos[2] = 0.0;
			glVertex3fv(pos);
			pos[0] = 1.0, pos[1] = dh*i, pos[2] = 0.0;
			glVertex3fv(pos);
			pos[0] = 1.0, pos[1] = dh*(i+1), pos[2] = 0.0;
			glVertex3fv(pos);
			pos[0] = 0.0, pos[1] = dh*(i+1), pos[2] = 0.0;
			glVertex3fv(pos);
			glEnd();
		}
	}
	else if (colormap->mode == MPGL_ColormapGrad) {
		if (colormap->ngrad <= 1) return;
		dh = 1.0f/(colormap->ngrad-1);
		for (i = 0;i < colormap->ngrad-1;i++) {
			glBegin(GL_QUADS);
			glColor3f(colormap->grad_color[i][0], colormap->grad_color[i][1], colormap->grad_color[i][2]);
			pos[0] = 0.0, pos[1] = dh*i, pos[2] = 0.0;
			glVertex3fv(pos);
			pos[0] = 1.0, pos[1] = dh*i, pos[2] = 0.0;
			glVertex3fv(pos);
			glColor3f(colormap->grad_color[i+1][0], colormap->grad_color[i+1][1], colormap->grad_color[i+1][2]);
			pos[0] = 1.0, pos[1] = dh*(i+1), pos[2] = 0.0;
			glVertex3fv(pos);
			pos[0] = 0.0, pos[1] = dh*(i+1), pos[2] = 0.0;
			glVertex3fv(pos);
			glEnd();
		}
	}
	glColor3fv(colormap->font_color);
	if (colormap->mode == MPGL_ColormapStep) {
		dh = 1.0f/colormap->nstep;
		for (i = 0;i < colormap->nstep;i++) {
			pos[0] = 1.1f, pos[1] = dh*i + dh/2, pos[2] = 0.0;
			glRasterPos3fv(pos);
			sprintf(s, "%s", colormap->label[i]);
			MPGL_TextBitmap(s, colormap->font_type);
		}
	}
	else if (colormap->mode == MPGL_ColormapGrad) {
		dh = 1.0f/(colormap->nscale+1);
		dy = (float)((colormap->range[1]-colormap->range[0])/(colormap->nscale+1));
		for (i = 0;i <= colormap->nscale+1;i++) {
			pos[0] = 1.1f, pos[1] = dh*i; pos[2] = 0.0;
			glRasterPos3fv(pos);
			sprintf(s, "%g", colormap->range[0]+i*dy);
			MPGL_TextBitmap(s, colormap->font_type);
		}
	}
	pos[0] = 0.0;
	pos[1] = -0.1f;
	pos[2] = 0.0;
	glRasterPos3f(pos[0], pos[1], pos[2]);
	MPGL_TextBitmap(colormap->title, colormap->font_type);
	glPopAttrib();
}

/**********************************************************
* for Python
**********************************************************/
#ifdef MP_PYTHON_LIB

static void PyDealloc(MPGL_Colormap* self)
{
#ifndef PY3
	self->ob_type->tp_free((PyObject*)self);
#endif
}

static PyObject *PyNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	MPGL_Colormap *self;

	self = (MPGL_Colormap *)type->tp_alloc(type, 0);
	MPGL_ColormapInit(self);
	return (PyObject *)self;
}

static PyMemberDef PyMembers[] = {
	{ "mode", T_INT, offsetof(MPGL_Colormap, mode), 0, "mode = {0:step | 1:gradation} : colormap mode" },
	{ "title", T_STRING, offsetof(MPGL_Colormap, title), 0, "title = txt : colormap title" },
	{ "nstep", T_INT, offsetof(MPGL_Colormap, nstep), 0, "nstep = num : number of step color" },
	{ "ngrad", T_INT, offsetof(MPGL_Colormap, ngrad), 0, "ngrad = num : number of gradiation color" },
	{ "nscale", T_INT, offsetof(MPGL_Colormap, nscale), 0, "nscale = num : number of scale" },
	{ "font_type", T_INT, offsetof(MPGL_Colormap, font_type), 0, "font_type = {0:10pt | 1:12pt | 2:18pt} : font type" },
	{ NULL }  /* Sentinel */
};

static PyObject *PyColor(MPGL_Colormap *self, PyObject *args)
{
	MPGL_ColormapColor(self);
	Py_RETURN_NONE;
}

static PyObject *PyGrayscale(MPGL_Colormap *self, PyObject *args)
{
	MPGL_ColormapGrayscale(self);
	Py_RETURN_NONE;
}

static PyObject *PySetStepColor(MPGL_Colormap *self, PyObject *args, PyObject *kwds)
{
	int id;
	float red, green, blue;
	static char *kwlist[] = { "id", "red", "green", "blue", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ifff", kwlist, &id, &red, &blue, &green)) {
		return NULL;
	}
	if (id < 0 || id >= MPGL_COLORMAP_MAX) {
		PyErr_SetString(PyExc_ValueError, "invalid id");
		return NULL;
	}
	self->step_color[id][0] = red;
	self->step_color[id][1] = green;
	self->step_color[id][2] = blue;
	Py_RETURN_NONE;
}

static PyObject *PySetGradColor(MPGL_Colormap *self, PyObject *args, PyObject *kwds)
{
	int id;
	float red, green, blue;
	static char *kwlist[] = { "id", "red", "green", "blue", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ifff", kwlist, &id, &red, &blue, &green)) {
		return NULL;
	}
	if (id < 0 || id >= MPGL_COLORMAP_MAX) {
		PyErr_SetString(PyExc_ValueError, "invalid id");
		return NULL;
	}
	self->grad_color[id][0] = red;
	self->grad_color[id][1] = green;
	self->grad_color[id][2] = blue;
	Py_RETURN_NONE;
}

static PyObject *PySetLabel(MPGL_Colormap *self, PyObject *args, PyObject *kwds)
{
	int id;
	char *label;
	static char *kwlist[] = { "id", "label", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "is", kwlist, &id, &label)) {
		return NULL;
	}
	if (id < 0 || id >= MPGL_COLORMAP_MAX) {
		PyErr_SetString(PyExc_ValueError, "invalid id");
		return NULL;
	}
	strcpy(self->label[id], label);
	Py_RETURN_NONE;
}

static PyObject *PyStepColor(MPGL_Colormap *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };
	float color[3];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id < 0 || id >= MPGL_COLORMAP_MAX) {
		PyErr_SetString(PyExc_ValueError, "invalid id");
		return NULL;
	}
	MPGL_ColormapStepColor(self, id, color);
	return Py_BuildValue("ddd", color[0], color[1], color[2]);
}

static PyObject *PyGradColor(MPGL_Colormap *self, PyObject *args, PyObject *kwds)
{
	double value;
	static char *kwlist[] = { "value", NULL };
	float color[3];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "d", kwlist, &value)) {
		return NULL;
	}
	MPGL_ColormapGradColor(self, value, color);
	return Py_BuildValue("ddd", color[0], color[1], color[2]);
}

static PyObject *PyDraw(MPGL_Colormap *self, PyObject *args)
{
	MPGL_ColormapDraw(self);
	Py_RETURN_NONE;
}

static PyMethodDef PyMethods[] = {
	{ "color", (PyCFunction)PyColor, METH_NOARGS,
	"color() : set default color" },
	{ "grayscale", (PyCFunction)PyGrayscale, METH_NOARGS,
	"grayscale() : set default grayscale" },
	{ "set_step_color", (PyCFunction)PySetStepColor, METH_VARARGS | METH_KEYWORDS,
	"set_step_color(id, red, green, blue) : set step color" },
	{ "set_grad_color", (PyCFunction)PySetGradColor, METH_VARARGS | METH_KEYWORDS,
	"set_grad_color(id, red, green, blue) : set grad color" },
	{ "set_label", (PyCFunction)PySetLabel, METH_VARARGS | METH_KEYWORDS,
	"set_label(id, label) : set label" },
	{ "step_color", (PyCFunction)PyStepColor, METH_VARARGS | METH_KEYWORDS,
	"step_color(id) : get step color" },
	{ "grad_color", (PyCFunction)PyGradColor, METH_VARARGS | METH_KEYWORDS,
	"grad_color(value) : get grad color" },
	{ "draw", (PyCFunction)PyDraw, METH_NOARGS,
	"draw() : draw colormap" },
	{ NULL }  /* Sentinel */
};

static PyObject *PyGetRange(MPGL_Colormap *self, void *closure)
{
	return Py_BuildValue("dd", self->range[0], self->range[1]);
}

static int PySetRange(MPGL_Colormap *self, PyObject *value, void *closure)
{
	double min, max;

	if (!PyArg_ParseTuple(value, "dd", &min, &max)) {
		return -1;
	}
	self->range[0] = min, self->range[1] = max;
	return 0;
}

static PyObject *PyGetSize(MPGL_Colormap *self, void *closure)
{
	return Py_BuildValue("dd", self->size[0], self->size[1]);
}

static int PySetSize(MPGL_Colormap *self, PyObject *value, void *closure)
{
	float width, height;

	if (!PyArg_ParseTuple(value, "ff", &width, &height)) {
		return -1;
	}
	self->size[0] = width, self->range[1] = height;
	return 0;
}

static PyObject *PyGetFontColor(MPGL_Colormap *self, void *closure)
{
	return Py_BuildValue("ddd", self->font_color[0], self->font_color[1], self->font_color[2]);
}

static int PySetFontColor(MPGL_Colormap *self, PyObject *value, void *closure)
{
	float red, green, blue;

	if (!PyArg_ParseTuple(value, "fff", &red, &green, &blue)) {
		return -1;
	}
	self->font_color[0] = red, self->font_color[1] = green, self->font_color[2] = blue;
	return 0;
}

static PyGetSetDef PyGetSet[] = {
	{ "range", (getter)PyGetRange, (setter)PySetRange, "range = (min, max) : colormap range", NULL },
	{ "size", (getter)PyGetSize, (setter)PySetSize, "size = (width, height) : colormap size", NULL },
	{ "font_color", (getter)PyGetFontColor, (setter)PySetFontColor, "font_color = (red, green, blue) : font color", NULL },
	{ NULL }  /* Sentinel */
};

PyTypeObject MPGL_ColormapPyType = {
	PyObject_HEAD_INIT(NULL)
#ifndef PY3
	0,							/*ob_size*/
#endif
	"MPGLKMC.colormap",			/*tp_name*/
	sizeof(MPGL_Colormap),		/*tp_basicsize*/
	0,							/*tp_itemsize*/
	(destructor)PyDealloc,		/*tp_dealloc*/
	0,							/*tp_print*/
	0,							/*tp_getattr*/
	0,							/*tp_setattr*/
	0,							/*tp_compare*/
	0,							/*tp_repr*/
	0,							/*tp_as_number*/
	0,							/*tp_as_sequence*/
	0,							/*tp_as_mapping*/
	0,							/*tp_hash */
	0,							/*tp_call*/
	0,							/*tp_str*/
	0,							/*tp_getattro*/
	0,							/*tp_setattro*/
	0,							/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/*tp_flags*/
	"colormap()",				/* tp_doc */
	0,							/* tp_traverse */
	0,							/* tp_clear */
	0,							/* tp_richcompare */
	0,							/* tp_weaklistoffset */
	0,							/* tp_iter */
	0,							/* tp_iternext */
	PyMethods,					/* tp_methods */
	PyMembers,					/* tp_members */
	PyGetSet,							/* tp_getset */
	0,							/* tp_base */
	0,							/* tp_dict */
	0,							/* tp_descr_get */
	0,							/* tp_descr_set */
	0,							/* tp_dictoffset */
	0,							/* tp_init */
	0,							/* tp_alloc */
	PyNew,						/* tp_new */
};

#endif /* MP_PYTHON_LIB */