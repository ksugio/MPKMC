#include "MPGLKMC.h"

void MPGL_KMCDrawInit(MPGL_KMCDraw *draw)
{
	int i;

	draw->kind = MPGL_KMCKindType;
	draw->res = 32;
	draw->frame_color[0] = 1.0;
	draw->frame_color[1] = 1.0;
	draw->frame_color[2] = 1.0;
	draw->frame_width = 1.0;
	for (i = 0; i < MP_KMC_TYPES_MAX; i++) {
		draw->disp[i] = TRUE;
	}
	draw->shift[0] = 0;
	draw->shift[1] = 0;
	draw->shift[2] = 0;
}

void MPGL_KMCDrawColormapRange(MPGL_KMCDraw *draw, MP_KMCData *data, MPGL_Colormap *colormap)
{
	int i;
	double min = 1.0e32;
	double max = -1.0e32;

	for (i = 0; i < data->ntot; i++) {
		if (data->grid[i].type >= 0) {
			if (data->grid[i].energy < min) min = data->grid[i].energy;
			if (data->grid[i].energy > max) max = data->grid[i].energy;
		}
	}
	colormap->range[0] = min;
	colormap->range[1] = max;
}

static void SphereList(int res)
{
	GLUquadricObj *quadObj;

	quadObj = gluNewQuadric();
	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricOrientation(quadObj, GLU_OUTSIDE);
	gluQuadricNormals(quadObj, GLU_SMOOTH);
	/* sphere */
	if (glIsList(MPGL_KMC_SPHERE) == GL_TRUE) glDeleteLists(MPGL_KMC_SPHERE, 1);
	glNewList(MPGL_KMC_SPHERE, GL_COMPILE);
	gluSphere(quadObj, 1.0, res, res);
	glEndList();
	gluDeleteQuadric(quadObj);
}

static void Index2Grid(MPGL_KMCDraw *draw, MP_KMCData *data, int id, int *p, int *x, int *y, int *z)
{
	int a, b, c, d;

	a = data->nuc * data->size[0] * data->size[1];
	*z = id / a;
	b = id - *z * a;
	c = data->nuc * data->size[0];
	*y = b / c;
	d = b - *y * c;
	*x = d / data->nuc;
	*p = d - *x * data->nuc;
	/* shift */
	*x += draw->shift[0];
	if (*x < 0) {
		*x += data->size[0];
	}
	else if (*x >= data->size[0]) {
		*x -= data->size[0];
	}
	*y += draw->shift[1];
	if (*y < 0) {
		*y += data->size[1];
	}
	else if (*y >= data->size[1]) {
		*y -= data->size[1];
	}
	*z += draw->shift[2];
	if (*z < 0) {
		*z += data->size[2];
	}
	else if (*z >= data->size[2]) {
		*z -= data->size[2];
	}
}

static void DrawSphere(MPGL_KMCDraw *draw, MP_KMCData *data, MPGL_Colormap *colormap)
{
	int i, j;
	short type;
	int p, x, y, z;
	double ax, ay, az;
	float color[3];

	for (i = 0; i < data->ntot; i++) {
		type = data->grid[i].type;
		if (type >= 0) {
			for (j = 0; j < data->ntypes; j++) {
				if (data->types[j] == type) break;
			}
			if (draw->disp[j]) {
				glPushMatrix();
				Index2Grid(draw, data, i, &p, &x, &y, &z);
				ax = x + data->uc[p][0];
				ay = y + data->uc[p][1];
				az = z + data->uc[p][2];
				glTranslated(ax, ay, az);
				glScaled(0.25, 0.25, 0.25);
				if (draw->kind == MPGL_KMCKindType) {
					MPGL_ColormapStepColor(colormap, j, color);
				}
				else if (draw->kind == MPGL_KMCKindEnergy) {
					MPGL_ColormapGradColor(colormap, data->grid[i].energy, color);
				}
				glColor3fv(color);
				glCallList(MPGL_KMC_SPHERE);
				glPopMatrix();
			}
		}
	}
}

void MPGL_KMCDrawGrid(MPGL_KMCDraw *draw, MP_KMCData *data, MPGL_Colormap *colormap)
{
	int i;

	SphereList(draw->res);
	if (draw->kind == MPGL_KMCKindType) {
		colormap->mode = MPGL_ColormapStep;
		sprintf(colormap->title, "Type");
		colormap->nstep = data->ntypes;
		for (i = 0; i < data->ntypes; i++) {
			sprintf(colormap->label[i], "%d", data->types[i]);
		}
	}
	else if (draw->kind == MPGL_KMCKindEnergy) {
		colormap->mode = MPGL_ColormapGrad;
		sprintf(colormap->title, "Energy");
		if (colormap->range[0] == colormap->range[1]) {
			MPGL_KMCDrawColormapRange(draw, data, colormap);
		}
	}
	DrawSphere(draw, data, colormap);
}

void MPGL_KMCDrawFrame(MPGL_KMCDraw *draw, MP_KMCData *data)
{
	int i, j;
	double pos[6];
	static int frame[12][6] = { { 0, 0, 0, 1, 0, 0 },{ 1, 0, 0, 1, 1, 0 },{ 1, 0, 0, 1, 0, 1 },
	{ 0, 0, 0, 0, 1, 0 },{ 0, 1, 0, 1, 1, 0 },{ 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 0, 0, 0, 1 },{ 0, 0, 1, 1, 0, 1 },{ 0, 0, 1, 0, 1, 1 },
	{ 1, 1, 1, 0, 1, 1 },{ 1, 1, 1, 1, 0, 1 },{ 1, 1, 1, 1, 1, 0 } };

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glColor3fv(draw->frame_color);
	glLineWidth(draw->frame_width);
	glBegin(GL_LINES);
	for (i = 0; i < 12; i++) {
		for (j = 0; j < 3; j++) {
			pos[j] = frame[i][j] * data->size[j];
			pos[j + 3] = frame[i][j + 3] * data->size[j];
		}
		glVertex3d(pos[0], pos[1], pos[2]);
		glVertex3d(pos[3], pos[4], pos[5]);

	}
	glEnd();
	glPopAttrib();
}

static void CylinderList(unsigned int res)
{
	GLUquadricObj *quadObj = gluNewQuadric();

	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricOrientation(quadObj, GLU_OUTSIDE);
	gluQuadricNormals(quadObj, GLU_SMOOTH);
	if (glIsList(MPGL_KMC_CYLINDER) == GL_TRUE) glDeleteLists(MPGL_KMC_CYLINDER, 1);
	glNewList(MPGL_KMC_CYLINDER, GL_COMPILE);
	gluCylinder(quadObj, 1.0, 1.0, 1.0, res, res);
	glTranslatef(0.0, 0.0, 1.0);
	gluDisk(quadObj, 0.0, 1.0, res, res);
	glTranslatef(0.0, 0.0, -1.0);
	glRotatef(180, 1, 0, 0);
	gluDisk(quadObj, 0.0, 1.0, res, res);
	glEndList();
	gluDeleteQuadric(quadObj);
}

void MPGL_KMCDrawAxis(int size[])
{
	CylinderList(32);
	glPushMatrix();
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.1f, 0.1f, (float)size[0]);
	glColor3f(1.0f, 0.0f, 0.0f);
	glCallList(MPGL_KMC_CYLINDER);
	glPopMatrix();
	glPushMatrix();
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glScalef(0.1f, 0.1f, (float)size[1]);
	glColor3f(0.0f, 1.0f, 0.0f);
	glCallList(MPGL_KMC_CYLINDER);
	glPopMatrix();
	glPushMatrix();
	glScalef(0.1f, 0.1f, (float)size[2]);
	glColor3f(0.0f, 0.0f, 1.0f);
	glCallList(MPGL_KMC_CYLINDER);
	glPopMatrix();
}

void MPGL_KMCDrawRegion(MP_KMCData *data, float region[])
{
	region[0] = -0.5f;
	region[1] = -0.5f;
	region[2] = -0.5f;
	region[3] = (float)data->size[0] + 0.5f;
	region[4] = (float)data->size[1] + 0.5f;
	region[5] = (float)data->size[2] + 0.5f;
}

/**********************************************************
* for Python
**********************************************************/
#ifndef _DEBUG

static void PyDealloc(MPGL_KMCDraw *self)
{
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *PyNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	MPGL_KMCDraw *self;

	self = (MPGL_KMCDraw *)type->tp_alloc(type, 0);
	MPGL_KMCDrawInit(self);
	return (PyObject *)self;
}

static PyObject *PyKMCDrawColormapRange(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	MPGL_Colormap *cmp;
	static char *kwlist[] = { "kmc", "cmp", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO!", kwlist, &data, &MPGL_ColormapPyType, &cmp)) {
		return NULL;
	}
	MPGL_KMCDrawColormapRange(self, data, cmp);
	Py_RETURN_NONE;
}

static PyObject *PyKMCDrawGrid(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	MPGL_Colormap *cmp;
	static char *kwlist[] = { "kmc", "cmp", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO!", kwlist, &data, &MPGL_ColormapPyType, &cmp)) {
		return NULL;
	}
	MPGL_KMCDrawGrid(self, data, cmp);
	Py_RETURN_NONE;
}

static PyObject *PyKMCDrawFrame(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	static char *kwlist[] = { "kmc", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &data)) {
		return NULL;
	}
	MPGL_KMCDrawFrame(self, data);
	Py_RETURN_NONE;
}

static PyObject *PyKMCDrawAxis(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	int size[3];
	static char *kwlist[] = { "size", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "(iii)", kwlist, &(size[0]), &(size[1]), &(size[2]))) {
		return NULL;
	}
	MPGL_KMCDrawAxis(size);
	Py_RETURN_NONE;
}

static PyObject *PyKMCDrawGetDisp(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id >= 0 && id < MP_KMC_TYPES_MAX) {
		return Py_BuildValue("i", self->disp[id]);
	}
	Py_RETURN_NONE;
}

static PyObject *PyKMCDrawSetDisp(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	int id;
	int disp;
	static char *kwlist[] = { "id", "disp", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, &id, &disp)) {
		return NULL;
	}
	if (id >= 0 && id < MP_KMC_TYPES_MAX) {
		self->disp[id] = disp;
	}
	Py_RETURN_NONE;
}

static PyObject *PyKMCDrawRegion(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	static char *kwlist[] = { "kmc", NULL };
	float region[6];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &data)) {
		return NULL;
	}
	MPGL_KMCDrawRegion(data, region);
	return Py_BuildValue("dddddd", region[0], region[1], region[2],
		region[3], region[4], region[5]);
}

static PyMethodDef PyMethods[] = {
	{ "colormap_range", (PyCFunction)PyKMCDrawColormapRange, METH_VARARGS | METH_KEYWORDS,
	"colormap_range(kmc, cmp) : set colormap range" },
	{ "draw_grid", (PyCFunction)PyKMCDrawGrid, METH_VARARGS | METH_KEYWORDS,
	"draw_grid(kmc, cmp) : draw kmc grid" },
	{ "draw_frame", (PyCFunction)PyKMCDrawFrame, METH_VARARGS | METH_KEYWORDS,
	"draw_frame(kmc) : draw frame" },
	{ "draw_axis", (PyCFunction)PyKMCDrawAxis, METH_VARARGS | METH_KEYWORDS,
	"draw_axis(sx, sy, sz) : draw axis" },
	{ "get_disp", (PyCFunction)PyKMCDrawGetDisp, METH_VARARGS | METH_KEYWORDS,
	"set_disp(id) : get display" },
	{ "set_disp", (PyCFunction)PyKMCDrawSetDisp, METH_VARARGS | METH_KEYWORDS,
	"set_disp(id, disp) : set display" },
	{ "region", (PyCFunction)PyKMCDrawRegion, METH_VARARGS | METH_KEYWORDS,
	"region(kmc) : return draw region" },
	{ NULL }  /* Sentinel */
};

static PyMemberDef PyMembers[] = {
	{ "kind", T_INT, offsetof(MPGL_KMCDraw, kind), 0, "draw kind, 0:type 1:energy" },
	{ "res", T_INT, offsetof(MPGL_KMCDraw, res), 0, "resolution of sphere" },
	{ "frame_width", T_FLOAT, offsetof(MPGL_KMCDraw, frame_width), 0, "frame width" },
	{ NULL }  /* Sentinel */
};

static PyObject *PyGetFrameColor(MPGL_KMCDraw *self, void *closure)
{
	return Py_BuildValue("fff", self->frame_color[0], self->frame_color[1], self->frame_color[2]);
}

static int PySetFrameColor(MPGL_KMCDraw *self, PyObject *value, void *closure)
{
	float r, g, b;

	if (!PyArg_ParseTuple(value, "fff", &r, &g, &b)) {
		return -1;
	}
	self->frame_color[0] = r, self->frame_color[1] = g, self->frame_color[2] = b;
	return 0;
}

static PyObject *PyGetShift(MPGL_KMCDraw *self, void *closure)
{
	return Py_BuildValue("iii", self->shift[0], self->shift[1], self->shift[2]);
}

static int PySetShift(MPGL_KMCDraw *self, PyObject *value, void *closure)
{
	int x, y, z;

	if (!PyArg_ParseTuple(value, "iii", &x, &y, &z)) {
		return -1;
	}
	self->shift[0] = x, self->shift[1] = y, self->shift[2] = z;
	return 0;
}

static PyGetSetDef PyGetSet[] = {
	{ "frame_color", (getter)PyGetFrameColor, (setter)PySetFrameColor, "frame color", NULL },
	{ "shift", (getter)PyGetShift, (setter)PySetShift, "shift", NULL },
	{ NULL }  /* Sentinel */
};

PyTypeObject MPGL_KMCDrawDataPyType = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"MPGLKMC.draw",				/*tp_name*/
	sizeof(MPGL_KMCDraw),	/*tp_basicsize*/
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
	"draw()",					/* tp_doc */
	0,							/* tp_traverse */
	0,							/* tp_clear */
	0,							/* tp_richcompare */
	0,							/* tp_weaklistoffset */
	0,							/* tp_iter */
	0,							/* tp_iternext */
	PyMethods,					/* tp_methods */
	PyMembers,					/* tp_members */
	PyGetSet,					/* tp_getset */
	0,							/* tp_base */
	0,							/* tp_dict */
	0,							/* tp_descr_get */
	0,							/* tp_descr_set */
	0,							/* tp_dictoffset */
	0,							/* tp_init */
	0,							/* tp_alloc */
	PyNew,						/* tp_new */
};

#endif /* _DEBUG */