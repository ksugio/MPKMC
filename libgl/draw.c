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
	draw->axis_dia = 0.1f;
	draw->ntypes = 0;
	for (i = 0; i < MPGL_KMC_TYPES_MAX; i++) {
		draw->disp[i] = TRUE;
		draw->dia[i] = 0.25;
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
		if (data->grid[i].type > 0) {
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

static void realPos(double pv[][3], double x0, double y0, double z0,
	double *x1, double *y1, double *z1)
{
	*x1 = pv[0][0] * x0 + pv[1][0] * y0 + pv[2][0] * z0;
	*y1 = pv[0][1] * x0 + pv[1][1] * y0 + pv[2][1] * z0;
	*z1 = pv[0][2] * x0 + pv[1][2] * y0 + pv[2][2] * z0;
}

static void DrawTransform(MP_KMCData *data)
{
	double m[16];

	m[0] = data->pv[0][0], m[1] = data->pv[1][0], m[2] = data->pv[2][0], m[3] = 0.0;
	m[4] = data->pv[0][1], m[5] = data->pv[1][1], m[6] = data->pv[2][1], m[7] = 0.0;
	m[8] = data->pv[0][2], m[9] = data->pv[1][2], m[10] = data->pv[2][2], m[11] = 0.0;
	m[12] = 0.0, m[13] = 0.0, m[14] = 0.0, m[15] = 1.0;
	glMultMatrixd(&(m[0]));
}

static void DrawSphere(MPGL_KMCDraw *draw, MP_KMCData *data, MPGL_Colormap *colormap)
{
	int i, j;
	short type;
	int p, x, y, z;
	double px, py, pz;
	float color[3];

	for (i = 0; i < data->ntot; i++) {
		type = data->grid[i].type;
		for (j = 0; j < draw->ntypes; j++) {
			if (draw->types[j] == type) break;
		}
		if (draw->disp[j]) {
			glPushMatrix();
			Index2Grid(draw, data, i, &p, &x, &y, &z);
			px = x + data->uc[p][0];
			py = y + data->uc[p][1];
			pz = z + data->uc[p][2];
			glTranslated(px, py, pz);
			glScaled(draw->dia[j], draw->dia[j], draw->dia[j]);
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

static void AddTypes(MPGL_KMCDraw *draw, short type)
{
	int i;

	for (i = 0; i < draw->ntypes; i++) {
		if (draw->types[i] == type) return;
	}
	draw->types[draw->ntypes++] = type;
}

static void UpdateTypes(MPGL_KMCDraw *draw, MP_KMCData *data)
{
	int i;

	draw->ntypes = 0;
	for (i = 0; i < data->nuc; i++) {
		AddTypes(draw, data->uc_types[i]);
	}
	for (i = 0; i < data->nsolute; i++) {
		AddTypes(draw, data->solute[i].type);
	}
}

static void DrawFrame(MPGL_KMCDraw *draw)
{
	int i, j;
	double p0[3], p1[3];
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
			p0[j] = frame[i][j];
			p1[j] = frame[i][j + 3];
		}
		glVertex3d(p0[0], p0[1], p0[2]);
		glVertex3d(p1[0], p1[1], p1[2]);
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

static void DrawAxis(float dia, float lx, float ly, float lz)
{
	glPushMatrix();
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(dia, dia, lx);
	glColor3f(1.0f, 0.0f, 0.0f);
	glCallList(MPGL_KMC_CYLINDER);
	glPopMatrix();
	glPushMatrix();
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glScalef(dia, dia, ly);
	glColor3f(0.0f, 1.0f, 0.0f);
	glCallList(MPGL_KMC_CYLINDER);
	glPopMatrix();
	glPushMatrix();
	glScalef(dia, dia, lz);
	glColor3f(0.0f, 0.0f, 1.0f);
	glCallList(MPGL_KMC_CYLINDER);
	glPopMatrix();
}

void MPGL_KMCDrawAtoms(MPGL_KMCDraw *draw, MP_KMCData *data, MPGL_Colormap *colormap)
{
	int i;

	SphereList(draw->res);
	CylinderList(draw->res);
	UpdateTypes(draw, data);
	if (draw->kind == MPGL_KMCKindType) {
		colormap->mode = MPGL_ColormapStep;
		sprintf(colormap->title, "Type");
		colormap->nstep = draw->ntypes;
		for (i = 0; i < draw->ntypes; i++) {
			sprintf(colormap->label[i], "%d", draw->types[i]);
		}
	}
	else if (draw->kind == MPGL_KMCKindEnergy) {
		colormap->mode = MPGL_ColormapGrad;
		sprintf(colormap->title, "Energy");
		if (colormap->range[0] == colormap->range[1]) {
			MPGL_KMCDrawColormapRange(draw, data, colormap);
		}
	}
	DrawTransform(data);
	DrawSphere(draw, data, colormap);
	glPushMatrix();
	glScalef((float)data->size[0], (float)data->size[1], (float)data->size[2]);
	DrawFrame(draw);
	glPopMatrix();
	DrawAxis(draw->axis_dia, (float)data->size[0], (float)data->size[1], (float)data->size[2]);
}

void MPGL_KMCDrawCluster(MPGL_KMCDraw *draw, MP_KMCData *data, MPGL_Colormap *colormap, int id)
{
	int i, j;
	short type;
	float color[3];

	SphereList(draw->res);
	CylinderList(draw->res);
	UpdateTypes(draw, data);
	colormap->mode = MPGL_ColormapStep;
	sprintf(colormap->title, "Type");
	colormap->nstep = draw->ntypes;
	for (i = 0; i < draw->ntypes; i++) {
		sprintf(colormap->label[i], "%d", draw->types[i]);
	}
	DrawTransform(data);
	for (i = 0; i < data->ncluster; i++) {
		type = data->table[id].types[i];
		for (j = 0; j < draw->ntypes; j++) {
			if (draw->types[j] == type) break;
		}
		glPushMatrix();
		glTranslated(data->cluster[i][0], data->cluster[i][1], data->cluster[i][2]);
		glScaled(draw->dia[j], draw->dia[j], draw->dia[j]);
		MPGL_ColormapStepColor(colormap, j, color);
		glColor3fv(color);
		glCallList(MPGL_KMC_SPHERE);
		glPopMatrix();
	}
	DrawAxis(draw->axis_dia, 1.0, 1.0, 1.0);
}

void MPGL_KMCDrawAtomsRegion(MP_KMCData *data, float region[])
{
	double sx, sy, sz;

	realPos(data->pv, data->size[0], data->size[1], data->size[2], &sx, &sy, &sz);
	region[0] = -0.5f;
	region[1] = -0.5f;
	region[2] = -0.5f;
	region[3] = (float)sx + 0.5f;
	region[4] = (float)sy + 0.5f;
	region[5] = (float)sz + 0.5f;
}

void MPGL_KMCDrawClusterRegion(MP_KMCData *data, float region[])
{
	int i;
	double xmin = 1.0e32;
	double xmax = -1.0e32;
	double ymin = 1.0e32;
	double ymax = -1.0e32;
	double zmin = 1.0e32;
	double zmax = -1.0e32;

	for (i = 0; i < data->ncluster; i++) {
		if (data->rcluster[i][0] < xmin) xmin = data->rcluster[i][0];
		else if (data->rcluster[i][0] > xmax) xmax = data->rcluster[i][0];
		if (data->rcluster[i][1] < ymin) ymin = data->rcluster[i][1];
		else if (data->rcluster[i][1] > ymax) ymax = data->rcluster[i][1];
		if (data->rcluster[i][2] < zmin) zmin = data->rcluster[i][2];
		else if (data->rcluster[i][2] > zmax) zmax = data->rcluster[i][2];
	}
	region[0] = (float)(xmin-0.5);
	region[1] = (float)(ymin-0.5);
	region[2] = (float)(zmin-0.5);
	region[3] = (float)(xmax+0.5);
	region[4] = (float)(ymax+0.5);
	region[5] = (float)(zmax+0.5);
}

/**********************************************************
* for Python
**********************************************************/
#ifdef MP_PYTHON_LIB

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

static PyObject *PyKMCDrawAtoms(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	MPGL_Colormap *cmp;
	static char *kwlist[] = { "kmc", "cmp", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO!", kwlist, &data, &MPGL_ColormapPyType, &cmp)) {
		return NULL;
	}
	MPGL_KMCDrawAtoms(self, data, cmp);
	Py_RETURN_NONE;
}

static PyObject *PyKMCDrawCluster(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	MPGL_Colormap *cmp;
	int id;
	static char *kwlist[] = { "kmc", "cmp", "id", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO!i", kwlist, &data, &MPGL_ColormapPyType, &cmp, &id)) {
		return NULL;
	}
	MPGL_KMCDrawCluster(self, data, cmp, id);
	Py_RETURN_NONE;
}

static PyObject *PyKMCDrawGetDisp(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id >= 0 && id < MPGL_KMC_TYPES_MAX) {
		return Py_BuildValue("i", self->disp[id]);
	}
	return NULL;
}

static PyObject *PyKMCDrawSetDisp(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	int id;
	int disp;
	static char *kwlist[] = { "id", "disp", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, &id, &disp)) {
		return NULL;
	}
	if (id >= 0 && id < MPGL_KMC_TYPES_MAX) {
		self->disp[id] = disp;
	}
	Py_RETURN_NONE;
}

static PyObject *PyKMCDrawGetDia(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id >= 0 && id < MPGL_KMC_TYPES_MAX) {
		return Py_BuildValue("d", self->dia[id]);
	}
	return NULL;
}

static PyObject *PyKMCDrawSetDia(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	int id;
	float dia;
	static char *kwlist[] = { "id", "dia", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "if", kwlist, &id, &dia)) {
		return NULL;
	}
	if (id >= 0 && id < MPGL_KMC_TYPES_MAX) {
		self->dia[id] = dia;
	}
	Py_RETURN_NONE;
}

static PyObject *PyKMCDrawAtomsRegion(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	static char *kwlist[] = { "kmc", NULL };
	float region[6];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &data)) {
		return NULL;
	}
	MPGL_KMCDrawAtomsRegion(data, region);
	return Py_BuildValue("dddddd", region[0], region[1], region[2],
		region[3], region[4], region[5]);
}

static PyObject *PyKMCDrawClusterRegion(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	static char *kwlist[] = { "kmc", NULL };
	float region[6];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &data)) {
		return NULL;
	}
	MPGL_KMCDrawClusterRegion(data, region);
	return Py_BuildValue("dddddd", region[0], region[1], region[2],
		region[3], region[4], region[5]);
}

static PyObject *PyKMCDrawTypes(MPGL_KMCDraw *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	return Py_BuildValue("h", self->types[id]);
}

static PyMethodDef PyMethods[] = {
	{ "colormap_range", (PyCFunction)PyKMCDrawColormapRange, METH_VARARGS | METH_KEYWORDS,
	"colormap_range(kmc, cmp) : set colormap range" },
	{ "atoms", (PyCFunction)PyKMCDrawAtoms, METH_VARARGS | METH_KEYWORDS,
	"atoms(kmc, cmp) : draw atoms" },
	{ "cluster", (PyCFunction)PyKMCDrawCluster, METH_VARARGS | METH_KEYWORDS,
	"cluster(kmc, cmp, id) : draw a cluster registered in energy table" },
	{ "get_disp", (PyCFunction)PyKMCDrawGetDisp, METH_VARARGS | METH_KEYWORDS,
	"set_disp(id) : get display of spheres" },
	{ "set_disp", (PyCFunction)PyKMCDrawSetDisp, METH_VARARGS | METH_KEYWORDS,
	"set_disp(id, disp) : set display of spheres" },
	{ "get_dia", (PyCFunction)PyKMCDrawGetDia, METH_VARARGS | METH_KEYWORDS,
	"set_dia(id) : get diameter of spheres" },
	{ "set_dia", (PyCFunction)PyKMCDrawSetDia, METH_VARARGS | METH_KEYWORDS,
	"set_dia(id, dia) : set diameter of spheres" },
	{ "atoms_region", (PyCFunction)PyKMCDrawAtomsRegion, METH_VARARGS | METH_KEYWORDS,
	"atoms_region(kmc) : return atoms region for drawing" },
	{ "cluster_region", (PyCFunction)PyKMCDrawClusterRegion, METH_VARARGS | METH_KEYWORDS,
	"cluster_region(kmc) : return cluster region for drawing" },
	{ "types", (PyCFunction)PyKMCDrawTypes, METH_VARARGS | METH_KEYWORDS,
	"types(id) : return registered type" },
	{ NULL }  /* Sentinel */
};

static PyMemberDef PyMembers[] = {
	{ "kind", T_INT, offsetof(MPGL_KMCDraw, kind), 0, "kind = {0:type | 1:energy} : draw kind, " },
	{ "res", T_INT, offsetof(MPGL_KMCDraw, res), 0, "res = res : resolution of sphere" },
	{ "frame_width", T_FLOAT, offsetof(MPGL_KMCDraw, frame_width), 0, "frame_width = width : frame width" },
	{ "axis_dia", T_FLOAT, offsetof(MPGL_KMCDraw, axis_dia), 0, "axis_dia = dia : diameter of axis" },
	{ "ntypes", T_INT, offsetof(MPGL_KMCDraw, ntypes), 1, "ntypes : number of registered types" },
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
	{ "frame_color", (getter)PyGetFrameColor, (setter)PySetFrameColor, "frame_color = (red, green, blue) : frame color", NULL },
	{ "shift", (getter)PyGetShift, (setter)PySetShift, "shift = (dx, dy, dz) : shift in drawing", NULL },
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

#endif /* MP_PYTHON_LIB */