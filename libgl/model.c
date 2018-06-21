#include "MPGLKMC.h"

static void MatMultMat(float mat[4][4], float x[4][4])
{
	int i;
	float tmp[4][4];
	
	for (i = 0;i < 4;i++) {
		tmp[i][0] = mat[i][0] * x[0][0] + mat[i][1] * x[1][0] + mat[i][2] * x[2][0] + mat[i][3] * x[3][0];
		tmp[i][1] = mat[i][0] * x[0][1] + mat[i][1] * x[1][1] + mat[i][2] * x[2][1] + mat[i][3] * x[3][1];
		tmp[i][2] = mat[i][0] * x[0][2] + mat[i][1] * x[1][2] + mat[i][2] * x[2][2] + mat[i][3] * x[3][2];
		tmp[i][3] = mat[i][0] * x[0][3] + mat[i][1] * x[1][3] + mat[i][2] * x[2][3] + mat[i][3] * x[3][3];
	}
	for (i = 0;i < 4;i++) {
		mat[i][0] = tmp[i][0], mat[i][1] = tmp[i][1], mat[i][2] = tmp[i][2], mat[i][3] = tmp[i][3];
	}
}

static void MatInverse(float mat[4][4], float mati[4][4])
{
	float a[4][4];
	float b[4][4] = {{1.0, 0.0, 0.0, 0.0}, {0.0, 1.0, 0.0, 0.0},
	{0.0, 0.0, 1.0, 0.0}, {0.0, 0.0, 0.0, 1.0}};
	float p,d,max,dumy;
	int i,j,k,s;
	
	for (k=0;k<4;k++) {
		a[k][0] = mat[k][0];
		a[k][1] = mat[k][1];
		a[k][2] = mat[k][2];
		a[k][3] = mat[k][3];
	}
	for (k=0;k<4;k++) {
		max=0.0, s=k;
		for (j=k;j<4;j++) {
			if (fabs(a[j][k])>max) {
				max=(float)fabs(a[j][k]), s=j;
			}
		}
		for (j=0;j<4;j++) {
			dumy=a[k][j];
			a[k][j]=a[s][j];
			a[s][j]=dumy;
			dumy=b[k][j];
			b[k][j]=b[s][j];
			b[s][j]=dumy;
		}
		p=a[k][k];
		for (j=0;j<4;j++) {
			a[k][j]=a[k][j]/p;
			b[k][j]=b[k][j]/p;
		}
		for (i=0;i<4;i++) {
			if (i!=k) {
				d=a[i][k];
				for (j=0;j<4;j++) {
					a[i][j]=a[i][j]-d*a[k][j];
					b[i][j]=b[i][j]-d*b[k][j];
				}
			}
		}
	}
	for (k=0;k<4;k++) {
		mati[k][0] = b[k][0];
		mati[k][1] = b[k][1];
		mati[k][2] = b[k][2];
		mati[k][3] = b[k][3];
	}
}

void MPGL_ModelInit(MPGL_Model *model)
{
	model->mat[0][0] = 1.0, model->mat[0][1] = 0.0, model->mat[0][2] = 0.0, model->mat[0][3] = 0.0;
	model->mat[1][0] = 0.0, model->mat[1][1] = 1.0, model->mat[1][2] = 0.0, model->mat[1][3] = 0.0;
	model->mat[2][0] = 0.0, model->mat[2][1] = 0.0, model->mat[2][2] = 1.0, model->mat[2][3] = 0.0;
	model->mat[3][0] = 0.0, model->mat[3][1] = 0.0, model->mat[3][2] = 0.0, model->mat[3][3] = 1.0;
	MatInverse(model->mat, model->mat_inv);
	model->center[0] = 0.0, model->center[1] = 0.0, model->center[2] = 0.0;
	model->scale = 1.0;
}

void MPGL_ModelZoom(MPGL_Model *model, float s)
{
	model->scale *= s;
}

void MPGL_ModelTranslateZ(MPGL_Model *model, float mz)
{
	float smz;
	
	smz = mz/model->scale;
	model->center[0] -= smz*model->mat_inv[2][0];
	model->center[1] -= smz*model->mat_inv[2][1];
	model->center[2] -= smz*model->mat_inv[2][2];
}

void MPGL_ModelTranslateY(MPGL_Model *model, float my)
{
	float smy;

	smy = my/model->scale;
	model->center[0] -= smy*model->mat_inv[1][0];
	model->center[1] -= smy*model->mat_inv[1][1];
	model->center[2] -= smy*model->mat_inv[1][2];
}

void MPGL_ModelTranslateX(MPGL_Model *model, float mx)
{
	float smx;

	smx = mx/model->scale;
	model->center[0] -= smx*model->mat_inv[0][0];
	model->center[1] -= smx*model->mat_inv[0][1];
	model->center[2] -= smx*model->mat_inv[0][2];
}

void MPGL_ModelRotateZ(MPGL_Model *model, float az)
{
	float x[4][4];

	x[0][0] = (float)cos(az), x[0][1] = -(float)sin(az), x[0][2] = 0.0, x[0][3] = 0.0;
	x[1][0] = (float)sin(az), x[1][1] = (float)cos(az), x[1][2] = 0.0, x[1][3] = 0.0;
	x[2][0] = 0.0, x[2][1] = 0.0, x[2][2] = 1.0, x[2][3] = 0.0;
	x[3][0] = 0.0, x[3][1] = 0.0, x[3][2] = 0.0, x[3][3] = 1.0;
	MatMultMat(model->mat, x);
}

void MPGL_ModelRotateY(MPGL_Model *model, float ay)
{
	float x[4][4];

	x[0][0] = (float)cos(ay), x[0][1] = 0.0, x[0][2] = (float)sin(ay), x[0][3] = 0.0;
	x[1][0] = 0.0, x[1][1] = 1.0, x[1][2] = 0.0, x[1][3] = 0.0;
	x[2][0] = -(float)sin(ay), x[2][1] = 0.0, x[2][2] = (float)cos(ay), x[2][3] = 0.0;
	x[3][0] = 0.0, x[3][1] = 0.0, x[3][2] = 0.0, x[3][3] = 1.0;
	MatMultMat(model->mat, x);
}

void MPGL_ModelRotateX(MPGL_Model *model, float ax)
{
	float x[4][4];

	x[0][0] = 1.0, x[0][1] = 0.0, x[0][2] = 0.0, x[0][3] = 0.0;
	x[1][0] = 0.0, x[1][1] = (float)cos(ax), x[1][2] = -(float)sin(ax), x[1][3] = 0.0;
	x[2][0] = 0.0, x[2][1] = (float)sin(ax), x[2][2] = (float)cos(ax), x[2][3] = 0.0;
	x[3][0] = 0.0, x[3][1] = 0.0, x[3][2] = 0.0, x[3][3] = 1.0;
	MatMultMat(model->mat, x);
}

void MPGL_ModelInverse(MPGL_Model *model)
{
	MatInverse(model->mat, model->mat_inv);
}

static void MatDir2Trigon(float dir[6], double trigon[6])
{
	double x, y, z, x1, y1, z1;
	
	/* set front vector */
	x = dir[0], y = dir[1], z = dir[2];
	if (x*x+y*y != 0.0) {
		trigon[0] = x/sqrt(x*x+y*y);
		if (y > 0.0) trigon[1] = sqrt(1-trigon[0]*trigon[0]);
		else trigon[1] = -sqrt(1-trigon[0]*trigon[0]);
	}
	else {
		trigon[0] = 1.0, trigon[1] = 0.0;
	}
	x1 = trigon[0]*x + trigon[1]*y;
	y1 = -trigon[1]*x + trigon[0]*y;
	z1 = z;
	if (x1*x1+z1*z1 != 0.0) {
		trigon[2] = x1/sqrt(x1*x1+z1*z1);
		if (z1 > 0.0) trigon[3] = sqrt(1-trigon[2]*trigon[2]);
		else trigon[3] = -sqrt(1-trigon[2]*trigon[2]);
	}
	else {
		trigon[2] = 1.0, trigon[3] = 0.0;
	}
	/* set head vector */
	x = trigon[0]*trigon[2]*dir[3] + trigon[1]*trigon[2]*dir[4] + trigon[3]*dir[5];
	y = -trigon[1]*dir[3] + trigon[0]*dir[4];
	z = -trigon[0]*trigon[3]*dir[3] - trigon[1]*trigon[3]*dir[4] + trigon[2]*dir[5];
	if (y*y+z*z != 0.0) {
		trigon[4] = z/sqrt(y*y+z*z);
		if (y < 0.0) trigon[5] = sqrt(1-trigon[4]*trigon[4]);
		else trigon[5] = -sqrt(1-trigon[4]*trigon[4]);
	}
	else {
		trigon[4] = 1.0, trigon[5] = 0.0;
	}
}

static void MatSetTrigon(float mat[4][4], double trigon[6])
{
	mat[0][0] = (float)(trigon[0]*trigon[2]);
	mat[0][1] = (float)(-trigon[1]*trigon[4]-trigon[0]*trigon[3]*trigon[5]);
	mat[0][2] = (float)(trigon[1]*trigon[5]-trigon[0]*trigon[3]*trigon[4]);
	mat[0][3] = 0.0;
	mat[1][0] = (float)(trigon[1]*trigon[2]);
	mat[1][1] = (float)(trigon[0]*trigon[4]-trigon[1]*trigon[3]*trigon[5]);
	mat[1][2] = (float)(-trigon[0]*trigon[5]-trigon[1]*trigon[3]*trigon[4]);
	mat[1][3] = 0.0;
	mat[2][0] = (float)trigon[3];
	mat[2][1] = (float)(trigon[2]*trigon[5]);
	mat[2][2] = (float)(trigon[2]*trigon[4]);
	mat[2][3] = 0.0;
	mat[3][0] = 0.0;
	mat[3][1] = 0.0;
	mat[3][2] = 0.0;
	mat[3][3] = 1.0;
}

void MPGL_ModelSetDirection(MPGL_Model *model, float dir[6])
{
	double trigon[6];
	
	MatDir2Trigon(dir, trigon);
	MatSetTrigon(model->mat, trigon);
	MatInverse(model->mat, model->mat_inv);
}

void MPGL_ModelGetDirection(MPGL_Model *model, float dir[6])
{
	int i;
	double minf = 1.0e32;
	double minh = 1.0e32;

	dir[0] = model->mat_inv[0][0], dir[1] = model->mat_inv[0][1], dir[2] = model->mat_inv[0][2];
	dir[3] = model->mat_inv[2][0], dir[4] = model->mat_inv[2][1], dir[5] = model->mat_inv[2][2];
	for (i = 0;i < 3;i++) {
		if (fabs(dir[i]) > 0.0 && fabs(dir[i]) < minf) minf = fabs(dir[i]);
		if (fabs(dir[i+3]) > 0.0 && fabs(dir[i+3]) < minh) minh = fabs(dir[i+3]);
	}
	for (i = 0;i < 3;i++) {
		dir[i] /= (float)minf;
		dir[i+3] /= (float)minh;
	}
}

void MPGL_ModelSetAngle(MPGL_Model *model, float angle[3])
{
	double trigon[6];
	
	trigon[0] = cos((double)angle[0]*M_PI/180.0);
	trigon[1] = sin((double)angle[0]*M_PI/180.0);
	trigon[2] = cos((double)angle[1]*M_PI/180.0);
	trigon[3] = sin((double)angle[1]*M_PI/180.0);
	trigon[4] = cos((double)angle[2]*M_PI/180.0);
	trigon[5] = sin((double)angle[2]*M_PI/180.0);
	MatSetTrigon(model->mat, trigon);
	MatInverse(model->mat, model->mat_inv);
}

void MPGL_ModelGetAngle(MPGL_Model *model, float angle[3])
{
	float dir[6];
	double trigon[6];

	MPGL_ModelGetDirection(model, dir);
	MatDir2Trigon(dir, trigon);
	if (trigon[1] >= 0.0) {
		angle[0] = (float)(acos(trigon[0])*180.0/M_PI);
	}
	else {
		angle[0] = (float)(-acos(trigon[0])*180.0/M_PI);
	}
	if (trigon[3] >= 0.0) {
		angle[1] = (float)(acos(trigon[2])*180.0/M_PI);
	}
	else {
		angle[1] = (float)(-acos(trigon[2])*180.0/M_PI);
	}
	if (trigon[5] >= 0.0) {
		angle[2] = (float)(acos(trigon[4])*180.0/M_PI);
	}
	else {
		angle[2] = (float)(-acos(trigon[4])*180.0/M_PI);
	}
}

void MPGL_ModelFitCenter(MPGL_Model *model, float region[])
{
	model->center[0] = (region[0] + region[3]) / 2;
	model->center[1] = (region[1] + region[4]) / 2;
	model->center[2] = (region[2] + region[5]) / 2;
}

void MPGL_ModelFitScale(MPGL_Model *model, float region[], float aspect)
{
	int i;
	int bid[8][3] = {{0,1,2}, {0,4,2}, {3,4,2}, {3,1,2},
	{3,4,5}, {3,1,5}, {0,1,5}, {0,4,5}};
	float v[3], x, y, z;
	float sx, sy;
	float xmax = -1.0e10;
	float xmin = 1.0e10;
	float ymax = -1.0e10;
	float ymin = 1.0e10;

	for (i = 0;i < 8;i++) {
		v[0] = region[bid[i][0]];
		v[1] = region[bid[i][1]];
		v[2] = region[bid[i][2]];
		v[0] -= model->center[0], v[1] -= model->center[1], v[2] -= model->center[2];
		x = v[0]*model->mat[0][0] + v[1]*model->mat[1][0] + v[2]*model->mat[2][0] + model->mat[3][0];
		y = v[0]*model->mat[0][1] + v[1]*model->mat[1][1] + v[2]*model->mat[2][1] + model->mat[3][1];
		z = v[0]*model->mat[0][2] + v[1]*model->mat[1][2] + v[2]*model->mat[2][2] + model->mat[3][2];
		if (x > xmax) xmax = x;
		if (x < xmin) xmin = x;
		if (y > ymax) ymax = y;
		if (y < ymin) ymin = y;
	}
	if (fabs(xmax) > fabs(xmin)) xmax = (float)fabs(xmax);
	else xmax = (float)fabs(xmin);
	if (fabs(ymax) > fabs(ymin)) ymax = (float)fabs(ymax);
	else ymax = (float)fabs(ymin);
	if (xmax == 0.0) xmax = 1.0;
	if (ymax == 0.0) ymax = 1.0;
	sx = (float)2.0/xmax*(float)0.45;
	sy = (float)2.0*aspect/ymax*(float)0.45;
	if (sx < sy) model->scale = sx;
	else model->scale = sy;
}

void MPGL_ModelTransform(MPGL_Model *model)
{
	glScalef(model->scale, model->scale, model->scale);
	glMultMatrixf(&(model->mat[0][0]));
	glTranslatef(-model->center[0], -model->center[1], -model->center[2]);
}

/**********************************************************
* for Python
**********************************************************/
#ifdef PYTHON_DLL

static void PyDealloc(MPGL_Model* self)
{
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *PyNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	MPGL_Model *self;

	self = (MPGL_Model *)type->tp_alloc(type, 0);
	MPGL_ModelInit(self);
	return (PyObject *)self;
}

static PyMemberDef PyMembers[] = {
	{ "scale", T_FLOAT, offsetof(MPGL_Model, scale), 0, "scale = scale : scale" },
	{ NULL }  /* Sentinel */
};

static PyObject *PyInit(MPGL_Model *self, PyObject *args)
{
	MPGL_ModelInit(self);
	Py_RETURN_NONE;
}

static PyObject *PyZoom(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float scale;
	static char *kwlist[] = { "scale", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "f", kwlist, &scale)) {
		return NULL;
	}
	MPGL_ModelZoom(self, scale);
	Py_RETURN_NONE;
}

static PyObject *PyTranslateZ(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float dist;
	static char *kwlist[] = { "dist", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "f", kwlist, &dist)) {
		return NULL;
	}
	MPGL_ModelTranslateZ(self, dist);
	Py_RETURN_NONE;
}

static PyObject *PyTranslateY(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float dist;
	static char *kwlist[] = { "dist", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "f", kwlist, &dist)) {
		return NULL;
	}
	MPGL_ModelTranslateY(self, dist);
	Py_RETURN_NONE;
}

static PyObject *PyTranslateX(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float dist;
	static char *kwlist[] = { "dist", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "f", kwlist, &dist)) {
		return NULL;
	}
	MPGL_ModelTranslateX(self, dist);
	Py_RETURN_NONE;
}

static PyObject *PyRotateZ(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float angle;
	static char *kwlist[] = { "angle", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "f", kwlist, &angle)) {
		return NULL;
	}
	MPGL_ModelRotateZ(self, angle);
	Py_RETURN_NONE;
}

static PyObject *PyRotateY(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float angle;
	static char *kwlist[] = { "angle", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "f", kwlist, &angle)) {
		return NULL;
	}
	MPGL_ModelRotateY(self, angle);
	Py_RETURN_NONE;
}

static PyObject *PyRotateX(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float angle;
	static char *kwlist[] = { "angle", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "f", kwlist, &angle)) {
		return NULL;
	}
	MPGL_ModelRotateX(self, angle);
	Py_RETURN_NONE;
}

static PyObject *PyInverse(MPGL_Model *self, PyObject *args)
{
	MPGL_ModelInverse(self);
	Py_RETURN_NONE;
}

static PyObject *PySetDirection(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float dir[6] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
	static char *kwlist[] = { "x0", "x1", "x2", "z0", "z1", "z2", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "fff|fff", kwlist, &(dir[0]), &(dir[1]), &(dir[2]),
		&(dir[3]), &(dir[4]), &(dir[5]))) {
		return NULL;
	}
	MPGL_ModelSetDirection(self, dir);
	Py_RETURN_NONE;
}

static PyObject *PyGetDirection(MPGL_Model *self, PyObject *args)
{
	float dir[6];

	MPGL_ModelGetDirection(self, dir);
	return Py_BuildValue("dddddd", dir[0], dir[1], dir[2], dir[3], dir[4], dir[5]);
}

static PyObject *PySetAngle(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float angle[3];
	static char *kwlist[] = { "alpha", "beta", "gamma", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "fff", kwlist, &(angle[0]), &(angle[1]), &(angle[2]))) {
		return NULL;
	}
	MPGL_ModelSetAngle(self, angle);
	Py_RETURN_NONE;
}

static PyObject *PyGetAngle(MPGL_Model *self, PyObject *args)
{
	float angle[3];

	MPGL_ModelGetAngle(self, angle);
	return Py_BuildValue("ddd", angle[0], angle[1], angle[2]);
}

static PyObject *PyFitCenter(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float region[6];
	static char *kwlist[] = { "region", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "(ffffff)", kwlist,
		&(region[0]), &(region[1]), &(region[2]), &(region[3]), &(region[4]), &(region[5]))) {
		return NULL;
	}
	MPGL_ModelFitCenter(self, region);
	Py_RETURN_NONE;
}

static PyObject *PyFitScale(MPGL_Model *self, PyObject *args, PyObject *kwds)
{
	float region[6];
	float aspect;
	static char *kwlist[] = { "region", "aspect", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "(ffffff)f", kwlist,
		&(region[0]), &(region[1]), &(region[2]), &(region[3]), &(region[4]), &(region[5]), &aspect)) {
		return NULL;
	}
	MPGL_ModelFitScale(self, region, aspect);
	Py_RETURN_NONE;
}

static PyObject *PyTransform(MPGL_Model *self, PyObject *args)
{
	MPGL_ModelTransform(self);
	Py_RETURN_NONE;
}

static PyMethodDef PyMethods[] = {
	{ "init", (PyCFunction)PyInit, METH_NOARGS,
	"init() : initialize matrix" },
	{ "zoom", (PyCFunction)PyZoom, METH_VARARGS | METH_KEYWORDS,
	"zoom(scale) : zoom" },
	{ "trans_z", (PyCFunction)PyTranslateZ, METH_VARARGS | METH_KEYWORDS,
	"trans_z(dist) : translate to Z direction" },
	{ "trans_y", (PyCFunction)PyTranslateY, METH_VARARGS | METH_KEYWORDS,
	"trans_y(dist) : translate to Y direction" },
	{ "trans_x", (PyCFunction)PyTranslateX, METH_VARARGS | METH_KEYWORDS,
	"trans_x(dist) : translate to X direction" },
	{ "rot_z", (PyCFunction)PyRotateZ, METH_VARARGS | METH_KEYWORDS,
	"rot_z(angle) : rotate around Z axis" },
	{ "rot_y", (PyCFunction)PyRotateY, METH_VARARGS | METH_KEYWORDS,
	"rot_y(angle) : rotate around Y axis" },
	{ "rot_x", (PyCFunction)PyRotateX, METH_VARARGS | METH_KEYWORDS,
	"rot_x(angle) : rotate around X axis" },
	{ "inverse", (PyCFunction)PyInverse, METH_NOARGS,
	"inverse() : set inverse matrix" },
	{ "set_dir", (PyCFunction)PySetDirection, METH_VARARGS | METH_KEYWORDS,
	"set_dir(x0, x1, x2, [z0, z1, z2]) : set direction" },
	{ "get_dir", (PyCFunction)PyGetDirection, METH_NOARGS,
	"get_dir() : get direction" },
	{ "set_angle", (PyCFunction)PySetAngle, METH_VARARGS | METH_KEYWORDS,
	"set_angle(alpha, beta, gamma) : set angle" },
	{ "get_angle", (PyCFunction)PyGetAngle, METH_NOARGS,
	"get_angle() : get angle" },
	{ "fit_center", (PyCFunction)PyFitCenter, METH_VARARGS | METH_KEYWORDS,
	"fit_center(region) : fit center" },
	{ "fit_scale", (PyCFunction)PyFitScale, METH_VARARGS | METH_KEYWORDS,
	"fit_scale(region, aspect) : fit scale" },
	{ "transform", (PyCFunction)PyTransform, METH_NOARGS,
	"transform() : OpenGL transformation" },
	{ NULL }  /* Sentinel */
};

static PyObject *PyGetMat(MPGL_Model *self, void *closure)
{
	return Py_BuildValue("(dddd)(dddd)(dddd)(dddd)", 
		self->mat[0][0], self->mat[0][1], self->mat[0][2], self->mat[0][3],
		self->mat[1][0], self->mat[1][1], self->mat[1][2], self->mat[1][3], 
		self->mat[2][0], self->mat[2][1], self->mat[2][2], self->mat[2][3], 
		self->mat[3][0], self->mat[3][1], self->mat[3][2], self->mat[3][3]);
}

static int PySetMat(MPGL_Model *self, PyObject *value, void *closure)
{
	float mat[4][4];
	int i, j;

	if (!PyArg_ParseTuple(value, "(ffff)(ffff)(ffff)(ffff)",
		&(mat[0][0]), &(mat[0][1]), &(mat[0][2]), &(mat[0][3]),
		&(mat[1][0]), &(mat[1][1]), &(mat[1][2]), &(mat[1][3]),
		&(mat[2][0]), &(mat[2][1]), &(mat[2][2]), &(mat[2][3]),
		&(mat[3][0]), &(mat[3][1]), &(mat[3][2]), &(mat[3][3]))) {
		return -1;
	}
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			self->mat[i][j] = mat[i][j];
		}
	}
	return 0;
}

static PyObject *PyGetCenter(MPGL_Model *self, void *closure)
{
	return Py_BuildValue("ddd", self->center[0], self->center[1], self->center[2]);
}

static int PySetCenter(MPGL_Model *self, PyObject *value, void *closure)
{
	float cx, cy, cz;

	if (!PyArg_ParseTuple(value, "fff", &cx, &cy, &cz)) {
		return -1;
	}
	self->center[0] = cx, self->center[1] = cy, self->center[2] = cz;
	return 0;
}

static PyObject *PyGetMatInv(MPGL_Model *self, void *closure)
{
	return Py_BuildValue("(dddd)(dddd)(dddd)(dddd)",
		self->mat_inv[0][0], self->mat_inv[0][1], self->mat_inv[0][2], self->mat_inv[0][3],
		self->mat_inv[1][0], self->mat_inv[1][1], self->mat_inv[1][2], self->mat_inv[1][3],
		self->mat_inv[2][0], self->mat_inv[2][1], self->mat_inv[2][2], self->mat_inv[2][3],
		self->mat_inv[3][0], self->mat_inv[3][1], self->mat_inv[3][2], self->mat_inv[3][3]);
}

static int PySetMatInv(MPGL_Model *self, PyObject *value, void *closure)
{
	float mat_inv[4][4];
	int i, j;

	if (!PyArg_ParseTuple(value, "(ffff)(ffff)(ffff)(ffff)",
		&(mat_inv[0][0]), &(mat_inv[0][1]), &(mat_inv[0][2]), &(mat_inv[0][3]),
		&(mat_inv[1][0]), &(mat_inv[1][1]), &(mat_inv[1][2]), &(mat_inv[1][3]),
		&(mat_inv[2][0]), &(mat_inv[2][1]), &(mat_inv[2][2]), &(mat_inv[2][3]),
		&(mat_inv[3][0]), &(mat_inv[3][1]), &(mat_inv[3][2]), &(mat_inv[3][3]))) {
		return -1;
	}
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			self->mat_inv[i][j] = mat_inv[i][j];
		}
	}
	return 0;
}

static PyGetSetDef PyGetSet[] = {
	{ "mat", (getter)PyGetMat, (setter)PySetMat, "mat = ((m00, m01, m02, m03), (...), (...), (...)) : transformation matrix", NULL },
	{ "center", (getter)PyGetCenter, (setter)PySetCenter, "center = (cx, cy, cz) : center of rotation", NULL },
	{ "mat_inv", (getter)PyGetMatInv, (setter)PySetMatInv, "mat_inv = ((i00, i01, i02, i03), (...), (...), (...)) : inversed transformation matrix", NULL },
	{ NULL }  /* Sentinel */
};

PyTypeObject MPGL_ModelPyType = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"MPGLKMC.model",			/*tp_name*/
	sizeof(MPGL_Model),			/*tp_basicsize*/
	0,							/*tp_itemsize*/
	(destructor)PyDealloc,	/*tp_dealloc*/
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
	"model()",					/* tp_doc */
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

#endif /* PYTHON_DLL */