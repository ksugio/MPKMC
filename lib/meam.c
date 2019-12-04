#include "MPKMC.h"

/*
	Reference
	M. I. Baskes
	Modified embedded-atom potentials for cubic materials and impurities
	Physical Review B vol.46 pp.2727-2742 (1992)
*/

static MP_MEAMParm MEAMParmTable[] = {
	{29, 3.540, 2.56, 5.22, 1.07, {3.63, 2.2, 6.0, 2.2}, {1, 3.14, 2.49, 2.95}},
	{47, 2.850, 2.88, 5.89, 1.06, {4.46, 2.2, 6.0, 2.2}, {1, 5.54, 2.45, 1.29}},
	{79, 3.930, 2.88, 6.34, 1.04, {5.45, 2.2, 6.0, 2.2}, {1, 1.59, 1.51, 2.61}},
	{28, 4.450, 2.49, 4.99, 1.10, {2.45, 2.2, 6.0, 2.2}, {1, 3.57, 1.60, 3.70}},
	{46, 3.910, 2.75, 6.43, 1.01, {4.98, 2.2, 6.0, 2.2}, {1, 2.34, 1.38, 4.48}},
	{78, 5.770, 2.77, 6.44, 1.04, {4.67, 2.2, 6.0, 2.2}, {1, 2.73, -1.38, 3.29}},
	{13, 3.580, 2.86, 4.61, 1.07, {2.21, 2.2, 6.0, 2.2}, {1, -1.78, -2.21, 8.01}},
	{82, 2.040, 3.50, 6.06, 1.01, {5.31, 2.2, 6.0, 2.2}, {1, 2.74, 3.06, 1.20}},
	{45, 5.750, 2.69, 6.00, 1.05, {1.13, 1.0, 2.0, 1.0}, {1, 2.99, 4.61, 4.80}},
	{77, 6.930, 2.72, 6.52, 1.05, {1.13, 1.0, 2.0, 1.0}, {1, 1.50, 8.10, 4.80}},
	{3, 1.650, 3.04, 2.97, 0.87, {1.43, 1.0, 1.0, 1.0}, {1, 0.26, 0.44, -0.20}},
	{11, 1.130, 3.72, 3.64, 0.90, {2.31, 1.0, 1.0, 1.0}, {1, 3.55, 0.69, -0.20}},
	{19, 0.941, 4.63, 3.90, 0.92, {2.69, 1.0, 1.0, 1.0}, {1, 5.10, 0.69, -0.20}},
	{23, 5.300, 2.63, 4.83, 1.00, {4.11, 1.0, 1.0, 1.0}, {1, 4.20, 4.10, -1.00}},
	{41, 7.470, 2.86, 4.79, 1.00, {4.37, 1.0, 1.0, 1.0}, {1, 3.76, 3.83, -1.00}},
	{73, 8.089, 2.86, 4.90, 0.99, {3.71, 1.0, 1.0, 1.0}, {1, 4.96, 3.35, -1.50}},
	{24, 4.100, 2.50, 5.12, 0.94, {3.22, 1.0, 1.0, 1.0}, {1, -0.21, 12.26, -1.90}},
	{42, 6.810, 2.73, 5.85, 0.99, {4.48, 1.0, 1.0, 1.0}, {1, 3.48, 9.49, -2.90}},
	{74, 8.660, 2.74, 5.63, 0.98, {3.98, 1.0, 1.0, 1.0}, {1, 3.16, 8.25, -2.70}},
	{26, 4.290, 2.48, 5.07, 0.89, {2.94, 1.0, 1.0, 1.0}, {1, 3.94, 4.12, -1.50}},
	{6, 7.370, 1.54, 4.31, 1.80, {5.50, 4.3, 3.1, 6.0}, {1, 5.57, 1.94, -0.77}},
	{14, 4.630, 2.35, 4.87, 1.00, {4.40, 5.5, 5.5, 5.5}, {1, 3.13, 4.47, -1.80}},
	{32, 3.850, 2.45, 4.98, 1.00, {4.55, 5.5, 5.5, 5.5}, {1, 4.02, 5.23, -1.60}}
};

void MP_MEAMInit(MP_MEAM *meam)
{
	int i;
	int ntable = sizeof(MEAMParmTable)/sizeof(MEAMParmTable[0]);

	meam->nparm = 0;
	for (i = 0; i < ntable; i++) {
		if (MP_MEAMSetParm(meam, MEAMParmTable[i]) < 0) break;
	}
	meam->Zd = 12;
	meam->S1 = 0.0, meam->S2 = 0.0, meam->S3 = 0.0;
}

int MP_MEAMSetParm(MP_MEAM *meam, MP_MEAMParm parm)
{
	int i;

	if (meam->nparm >= MP_MEAM_NPARM_MAX) {
		fprintf(stderr, "Error : can't add MEAM parameter (MP_MEAMSetParm)\n");
		return -1;
	}
	for (i = 0; i < meam->nparm; i++) {
		if (meam->parm[i].type == parm.type) {
			meam->parm[i] = parm;
			return i;
		}
	}
	meam->parm[meam->nparm] = parm;
	return meam->nparm++;
}

static MP_MEAMParm GetMEAMParm(MP_MEAM *meam, short type)
{
	int i;
	MP_MEAMParm err = { -1, 0.0, 0.0, 0.0, 0.0, { 0.0, 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0, 0.0 } };

	for (i = 0; i < meam->nparm; i++) {
		if (meam->parm[i].type == type) return meam->parm[i];
	}
	return err;
}

static int RijXij(MP_KMCData *data, short types[], short ntypes[], double rij[], double xij[][3])
{
	int i;
	int zi = 0;
	double dx, dy, dz, r;

	for (i = 1; i < data->ncluster; i++) {
		ntypes[zi] = types[i];
		dx = data->rcluster[i][0] - data->rcluster[0][0];
		dy = data->rcluster[i][1] - data->rcluster[0][1];
		dz = data->rcluster[i][2] - data->rcluster[0][2];
		r = sqrt(dx * dx + dy * dy + dz * dz);
		rij[zi] = r;
		xij[zi][0] = dx / r;
		xij[zi][1] = dy / r;
		xij[zi][2] = dz / r;
		zi++;
	}
	return zi;
}

static double Rhoia(double R, double Betai, double R0i)
{
	return exp(-Betai * (R / R0i - 1.0));
}

static double Rho_i(MP_MEAM *meam, int zi, short ntypes[], double rij[], double xij[][3], double Ti[])
{
	int j, l;
	int a, b, c;
	MP_MEAMParm p;
	double Betaj[MP_KMC_NCLUSTER_MAX][4];
	double R0j[MP_KMC_NCLUSTER_MAX];
	double t0, t1, t2;
	double rhoi2[4];

	for (j = 0; j < zi; j++) {
		if (ntypes[j] > 0) {
			p = GetMEAMParm(meam, ntypes[j]);
			if (p.type == -1) {
				fprintf(stderr, "Error : can't find MEAM parameter for type %d (Rho_i)\n", ntypes[j]);
				return zi;
			}
			else {
				Betaj[j][0] = p.Betai[0];
				Betaj[j][1] = p.Betai[1];
				Betaj[j][2] = p.Betai[2];
				Betaj[j][3] = p.Betai[3];
				R0j[j] = p.R0i;
			}
		}
	}
	// rhoi0
	t0 = 0.0;
	for (j = 0; j < zi; j++) {
		if (ntypes[j] > 0) {
			t0 += Rhoia(rij[j], Betaj[j][0], R0j[j]);
		}
	}
	rhoi2[0] = t0 * t0;
	// rhoi1
	t0 = 0.0;
	for (a = 0; a < 3; a++) {
		t1 = 0.0;
		for (j = 0; j < zi; j++) {
			if (ntypes[j] > 0) {
				t1 += xij[j][a] * Rhoia(rij[j], Betaj[j][1], R0j[j]);
			}
		}
		t0 += t1 * t1;
	}
	rhoi2[1] = t0;
	// rhoi2
	t0 = 0.0;
	for (a = 0; a < 3; a++) {
		for (b = 0; b < 3; b++) {
			t1 = 0.0;
			for (j = 0; j < zi; j++) {
				if (ntypes[j] > 0) {
					t1 += xij[j][a] * xij[j][b] * Rhoia(rij[j], Betaj[j][2], R0j[j]);
				}
			}
			t0 += t1 * t1;
		}
	}
	t2 = 0.0;
	for (j = 0; j < zi; j++) {
		if (ntypes[j] > 0) {
			t2 += Rhoia(rij[j], Betaj[j][2], R0j[j]);
		}
	}
	rhoi2[2] = t0 - t2 * t2 / 3.0;
	// rhoi3
	t0 = 0.0;
	for (a = 0; a < 3; a++) {
		for (b = 0; b < 3; b++) {
			for (c = 0; c < 3; c++) {
				t1 = 0.0;
				for (j = 0; j < zi; j++) {
					if (ntypes[j] > 0) {
						t1 += xij[j][a] * xij[j][b] * xij[j][c] * Rhoia(rij[j], Betaj[j][3], R0j[j]);
					}
				}
				t0 += t1 * t1;
			}
		}
	}
	rhoi2[3] = t0;
	// rho_i
	t0 = 0.0;
	for (l = 0; l < 4; l++) {
		t0 += Ti[l] * rhoi2[l];
	}
	return sqrt(t0);
}

static double Fi(double rho, double Ai, double E0i)
{
	return Ai * E0i * rho * log(rho);
}

static double Eui(double R, double E0i, double R0i, double Alphai)
{
	double aa;

	aa = Alphai*(R / R0i - 1.0);
	return -E0i*(1.0 + aa)*exp(-aa);
}

static double Rho_0i(double rij, double R0i, double Betai[], double Ti[], double Si[])
{
	int l;
	double t0 = 0.0;

	for (l = 0; l < 4; l++) {
		t0 += Ti[l] * Si[l] * pow(Rhoia(rij, Betai[l], R0i), 2.0);
	}
	return sqrt(t0);
}

double MP_MEAMEnergy(MP_MEAM *meam, MP_KMCData *data, short types[])
{
	int j;
	int zi;
	double Si[4];
	MP_MEAMParm p;
	short ntypes[MP_KMC_NCLUSTER_MAX];
	double rij[MP_KMC_NCLUSTER_MAX];
	double xij[MP_KMC_NCLUSTER_MAX][3];
	double t1, t2, t3;
	double rho_i, rho_0i;

	p = GetMEAMParm(meam, types[0]);
	if (p.type == -1) {
		fprintf(stderr, "Error : can't find MEAM parameter for type %d (MP_MEAMEnergy)\n", types[0]);
		return 0.0;
	}
	zi = RijXij(data, types, ntypes, rij, xij);
	// 1st term
	t1 = 0.0;
	for (j = 0; j < zi; j++) {
		t1 += Eui(rij[j], p.E0i, p.R0i, p.Alphai);
	}
	t1 = t1 / zi;
	// 2nd term
	rho_i = Rho_i(meam, zi, ntypes, rij, xij, p.Ti);
	t2 = Fi(rho_i / zi, p.Ai, p.E0i);
	// 3rd term
	Si[0] = meam->Zd * meam->Zd;
	Si[1] = meam->S1, Si[2] = meam->S2, Si[3] = meam->S3;
	t3 = 0.0;
	for (j = 0; j < zi; j++) {
		rho_0i = Rho_0i(rij[j], p.R0i, p.Betai, p.Ti, Si);
		t3 += Fi(rho_0i / zi, p.Ai, p.E0i);
	}
	t3 = t3 / zi;
	//fprintf(stderr, "%.10e %.10e %.10e %.10e\n", t1, t2, t3, rho_i);
	return t1+t2-t3;
}

/**********************************************************
* for Python
**********************************************************/
#ifdef MP_PYTHON_LIB

static void PyDealloc(MP_MEAM *self)
{
#ifndef PY3
	self->ob_type->tp_free((PyObject*)self);
#endif
}

static PyObject *PyNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	MP_MEAM *self;

	self = (MP_MEAM *)type->tp_alloc(type, 0);
	if (self != NULL) MP_MEAMInit(self);
	return (PyObject *)self;
}

static PyMemberDef PyMembers[] = {
	{ "nparm", T_INT, offsetof(MP_MEAM, nparm), 1, "number of parameters" },
	{ "Zd", T_INT, offsetof(MP_MEAM, Zd), 0, "parameter of reference structure" },
	{ "S1", T_DOUBLE, offsetof(MP_MEAM, S1), 0, "parameter of reference structure" },
	{ "S2", T_DOUBLE, offsetof(MP_MEAM, S2), 0, "parameter of reference structure" },
	{ "S3", T_DOUBLE, offsetof(MP_MEAM, S3), 0, "parameter of reference structure" },
	{ NULL }  /* Sentinel */
};

static PyObject *PyEnergy(MP_MEAM *self, PyObject *args, PyObject *kwds)
{
	MP_KMCData *data;
	PyObject *types;
	static char *kwlist[] = { "kmc", "types", NULL };
	int i;
	short stypes[MP_KMC_NCLUSTER_MAX];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", kwlist, &data, &types)) {
		return NULL;
	}
	for (i = 0; i < data->ncluster; i++) {
		stypes[i] = (short)PyInt_AsLong(PyTuple_GetItem(types, (Py_ssize_t)i));
	}
	return Py_BuildValue("d", MP_MEAMEnergy(self, data, stypes));
}

static PyObject *PyGetParm(MP_MEAM *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };
	MP_MEAMParm p;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id >= 0 && id < self->nparm) {
		p = self->parm[id];
		return Py_BuildValue("idddd(dddd)(dddd)", p.type,
			p.E0i, p.R0i, p.Alphai, p.Ai,
			p.Betai[0], p.Betai[1], p.Betai[2], p.Betai[3],
			p.Ti[0], p.Ti[1], p.Ti[2], p.Ti[3]);
	}
	else return NULL;
}

static PyObject *PySetParm(MP_MEAM *self, PyObject *args, PyObject *kwds)
{
	PyObject *parm;
	static char *kwlist[] = { "parm", NULL };
	MP_MEAMParm p;
	PyObject *tp;
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &parm)) {
		return NULL;
	}
	if (PyTuple_Size(parm) != 7) return NULL;
	p.type = (short)PyInt_AsLong(PyTuple_GetItem(parm, (Py_ssize_t)0));
	p.E0i = (double)PyFloat_AsDouble(PyTuple_GetItem(parm, (Py_ssize_t)1));
	p.R0i = (double)PyFloat_AsDouble(PyTuple_GetItem(parm, (Py_ssize_t)2));
	p.Alphai = (double)PyFloat_AsDouble(PyTuple_GetItem(parm, (Py_ssize_t)3));
	p.Ai = (double)PyFloat_AsDouble(PyTuple_GetItem(parm, (Py_ssize_t)4));
	tp = PyTuple_GetItem(parm, (Py_ssize_t)5);
	if (PyTuple_Size(tp) != 4) return NULL;
	for (i = 0; i < 4; i++) {
		p.Betai[i] = (double)PyFloat_AsDouble(PyTuple_GetItem(tp, (Py_ssize_t)i));
	}
	tp = PyTuple_GetItem(parm, (Py_ssize_t)6);
	if (PyTuple_Size(tp) != 4) return NULL;
	for (i = 0; i < 4; i++) {
		p.Ti[i] = (double)PyFloat_AsDouble(PyTuple_GetItem(tp, (Py_ssize_t)i));
	}
	return Py_BuildValue("i", MP_MEAMSetParm(self, p));
}

static PyMethodDef PyMethods[] = {
	{ "energy", (PyCFunction)PyEnergy, METH_VARARGS | METH_KEYWORDS,
		"energy(kmc, types) : calculate cluster energy" },
	{ "get_parm", (PyCFunction)PyGetParm, METH_VARARGS | METH_KEYWORDS,
		"get_parm(id) : return i-th parameter" },
	{ "set_parm", (PyCFunction)PySetParm, METH_VARARGS | METH_KEYWORDS,
		"set_parm(parm) : set parameter, return index in the parameter table" },
	{ NULL }  /* Sentinel */
};

static PyGetSetDef PyGetSet[] = {
	{ NULL }  /* Sentinel */
};

PyTypeObject MP_MEAMPyType = {
	PyObject_HEAD_INIT(NULL)
#ifndef PY3
	0,							/*ob_size*/
#endif
	"MPKMC.meam",				/*tp_name*/
	sizeof(MP_MEAM),			/*tp_basicsize*/
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
	"meam()",					/* tp_doc */
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