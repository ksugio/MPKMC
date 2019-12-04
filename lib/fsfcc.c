#include "MPKMC.h"

static struct MP_FSFCCParm FSFCCParmTable[] =
{
	// Copper
	{ 29, 3.615,
	{ 29.059214, -140.05681, 130.07331, -17.48135, 31.82546, 71.58749 },
	{ 1.2247449, 1.0 },
	{ 9.806694, 16.774638 },
	{ 1.2247449, 1.1547054, 1.1180065, 1.0, 0.8660254, 0.7071068 } },
	// Silver
	{ 47, 4.086,
	{ 20.368404, -102.36075, 94.31277, -6.220051, 31.08088, 175.56047 },
	{ 1.2247449, 1.0 },
	{ 1.458761, 42.946555 },
	{ 1.2247449, 1.1547054, 1.1180065, 1.0, 0.8660254, 0.7071068 } },
	// Gold
	{ 79, 4.078,
	{ 29.059066, -153.14779, 148.17881, -22.20508, 72.71465, 199.26269 },
	{ 1.1180065, 0.8660254 },
	{ 21.930125, 284.99631 },
	{ 1.2247449, 1.1547054, 1.1180065, 1.0, 0.8660254, 0.7071068 } },
	// Nickel
	{ 28, 3.524,
	{ 29.057085, -76.04625, 48.08920, -25.96604, 79.15121,  0.0 },
	{ 1.2247449, 1.1180065 },
	{ 60.537985, -80.102414 },
	{ 1.2247449, 1.1547054, 1.1180065, 1.0, 0.8660254, 0.7071068 } }
};

void MP_FSFCCInit(MP_FSFCC *fsfcc)
{
	int i;
	int ntable = sizeof(FSFCCParmTable) / sizeof(FSFCCParmTable[0]);

	fsfcc->nparm = 0;
	for (i = 0; i < ntable; i++) {
		if (MP_FSFCCSetParm(fsfcc, FSFCCParmTable[i]) < 0) break;
	}
}

int MP_FSFCCSetParm(MP_FSFCC *fsfcc, MP_FSFCCParm parm)
{
	int i;

	if (fsfcc->nparm >= MP_FSFCC_NPARM_MAX) {
		fprintf(stderr, "Error : can't add FSFCC parameter (MP_FSFCCSetParm)\n");
		return -1;
	}
	for (i = 0; i < fsfcc->nparm; i++) {
		if (fsfcc->parm[i].type == parm.type) {
			fsfcc->parm[i] = parm;
			return i;
		}
	}
	fsfcc->parm[fsfcc->nparm] = parm;
	return fsfcc->nparm++;
}

static MP_FSFCCParm GetFSFCCParm(MP_FSFCC *fsfcc, short type)
{
	int i;
	MP_FSFCCParm err = { -1, 0.0, { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, { 0.0, 0.0 }, {0.0, 0.0}, { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };

	for (i = 0; i < fsfcc->nparm; i++) {
		if (fsfcc->parm[i].type == type) return fsfcc->parm[i];
	}
	return err;
}

double MP_FSFCCEnergy(MP_FSFCC *fsfcc, MP_KMCData *data, short types[])
{
	int i, j;
	MP_FSFCCParm p;
	double r, Ui;
	double rho, sV;
	double phi, V;

	p = GetFSFCCParm(fsfcc, types[0]);
	if (p.type == -1) {
		fprintf(stderr, "Error : can't find FSFCC parameter for type %d (MP_FSFCCEnergy)\n", types[0]);
		return 0.0;
	}
	for (i = 1, rho = 0.0, sV = 0.0; i < data->ncluster; i++) {
		if (types[i] == p.type) {
			r = sqrt(data->rcluster[i][0]*data->rcluster[i][0]
				+ data->rcluster[i][1]*data->rcluster[i][1]
				+ data->rcluster[i][2]*data->rcluster[i][2]) / p.lc;
			for (j = 0, phi = 0.0; j < 2; j++) {
				if (p.R[j] >= r) {
					phi += p.A[j] * pow(p.R[j] - r, 3.0);
				}
			}
			rho += phi;
			for (j = 0, V = 0.0; j < 6; j++) {
				if (p.r[j] >= r) {
					V += p.a[j] * pow(p.r[j] - r, 3.0);
				}
			}
			sV += V;
		}
	}
	Ui = -sqrt(rho) + 0.5*sV;
	return Ui;
}

/**********************************************************
* for Python
**********************************************************/
#ifdef MP_PYTHON_LIB

static void PyDealloc(MP_FSFCC *self)
{
#ifndef PY3
	self->ob_type->tp_free((PyObject*)self);
#endif
}

static PyObject *PyNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	MP_FSFCC *self;

	self = (MP_FSFCC *)type->tp_alloc(type, 0);
	if (self != NULL) MP_FSFCCInit(self);
	return (PyObject *)self;
}

static PyMemberDef PyMembers[] = {
	{ "nparm", T_INT, offsetof(MP_FSFCC, nparm), 1, "number of parameters" },
	{ NULL }  /* Sentinel */
};

static PyObject *PyEnergy(MP_FSFCC *self, PyObject *args, PyObject *kwds)
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
	return Py_BuildValue("d", MP_FSFCCEnergy(self, data, stypes));
}

static PyObject *PyGetParm(MP_FSFCC *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };
	MP_FSFCCParm p;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id >= 0 && id < self->nparm) {
		p = self->parm[id];
		return Py_BuildValue("id(dddddd)(dd)(dd)(dddddd)", p.type, p.lc,
			p.a[0], p.a[1], p.a[2], p.a[3], p.a[4], p.a[5],
			p.R[0], p.R[1], p.A[0], p.A[1],
			p.r[0], p.r[1], p.r[2], p.r[3], p.r[4], p.r[5]);
	}
	else return NULL;
}

static PyObject *PySetParm(MP_FSFCC *self, PyObject *args, PyObject *kwds)
{
	PyObject *parm;
	static char *kwlist[] = { "parm", NULL };
	MP_FSFCCParm p;
	PyObject *tp;
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &parm)) {
		return NULL;
	}
	if (PyTuple_Size(parm) != 6) return NULL;
	p.type = (short)PyInt_AsLong(PyTuple_GetItem(parm, (Py_ssize_t)0));
	p.lc = (double)PyFloat_AsDouble(PyTuple_GetItem(parm, (Py_ssize_t)1));
	tp = PyTuple_GetItem(parm, (Py_ssize_t)2);
	if (PyTuple_Size(tp) != 6) return NULL;
	for (i = 0; i < 6; i++) {
		p.a[i] = (double)PyFloat_AsDouble(PyTuple_GetItem(tp, (Py_ssize_t)i));
	}
	tp = PyTuple_GetItem(parm, (Py_ssize_t)3);
	if (PyTuple_Size(tp) != 2) return NULL;
	for (i = 0; i < 2; i++) {
		p.R[i] = (double)PyFloat_AsDouble(PyTuple_GetItem(tp, (Py_ssize_t)i));
	}
	tp = PyTuple_GetItem(parm, (Py_ssize_t)4);
	if (PyTuple_Size(tp) != 2) return NULL;
	for (i = 0; i < 2; i++) {
		p.A[i] = (double)PyFloat_AsDouble(PyTuple_GetItem(tp, (Py_ssize_t)i));
	}
	tp = PyTuple_GetItem(parm, (Py_ssize_t)5);
	if (PyTuple_Size(tp) != 6) return NULL;
	for (i = 0; i < 6; i++) {
		p.r[i] = (double)PyFloat_AsDouble(PyTuple_GetItem(tp, (Py_ssize_t)i));
	}
	return Py_BuildValue("i", MP_FSFCCSetParm(self, p));
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

PyTypeObject MP_FSFCCPyType = {
	PyObject_HEAD_INIT(NULL)
#ifndef PY3
	0,							/*ob_size*/
#endif
	"MPKMC.fsfcc",				/*tp_name*/
	sizeof(MP_FSFCC),			/*tp_basicsize*/
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
	"fsfcc()",					/* tp_doc */
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