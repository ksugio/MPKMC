#include "MPKMC.h"

struct FSFCCParm {
	short type;
	double a[6];
	double R[2];
	double A[2];
	double r[6];
};

static struct FSFCCParm Copper = 
{
	29,
	{ 29.059214, -140.05681, 130.07331, -17.48135, 31.82546, 71.58749 },
	{ 1.2247449, 1.0 },
	{ 9.806694, 16.774638 },
	{ 1.2247449, 1.1547054, 1.1180065, 1.0, 0.8660254, 0.7071068 }
};

static struct FSFCCParm Silver =
{
	47,
	{ 20.368404, -102.36075, 94.31277, -6.220051, 31.08088, 175.56047 },
	{ 1.2247449, 1.0 },
	{ 1.458761, 42.946555 },
	{ 1.2247449, 1.1547054, 1.1180065, 1.0, 0.8660254, 0.7071068 }
};

static struct FSFCCParm Gold =
{
	79,
	{ 29.059066, -153.14779, 148.17881, -22.20508, 72.71465, 199.26269 },
	{ 1.1180065, 0.8660254 },
	{ 21.930125, 284.99631 },
	{ 1.2247449, 1.1547054, 1.1180065, 1.0, 0.8660254, 0.7071068 }
};

static struct FSFCCParm Nickel =
{
	28,
	{ 29.057085, -76.04625, 48.08920, -25.96604, 79.15121,  0.0 },
	{ 1.2247449, 1.1180065 },
	{ 60.537985, -80.102414 },
	{ 1.2247449, 1.1547054, 1.1180065, 1.0, 0.8660254, 0.7071068 }
};

static void FSFCCCopyParm(MP_FSFCCParm *parm, struct FSFCCParm p)
{
	int i;

	parm->type = p.type;
	for (i = 0; i < 2; i++) {
		parm->R[i] = p.R[i];
		parm->A[i] = p.A[i];
	}
	for (i = 0; i < 6; i++) {
		parm->a[i] = p.a[i];
		parm->r[i] = p.r[i];
	}
}

int MP_FSFCCSetParm(MP_FSFCCParm *parm, short type)
{
	if (type == 29) {
		FSFCCCopyParm(parm, Copper);
		return TRUE;
	}
	else if (type == 47) {
		FSFCCCopyParm(parm, Silver);
		return TRUE;
	}
	else if (type == 79) {
		FSFCCCopyParm(parm, Gold);
		return TRUE;
	}
	else if (type == 28) {
		FSFCCCopyParm(parm, Nickel);
		return TRUE;
	}
	return FALSE;
}

double MP_FSFCCEnergy(MP_FSFCCParm *parm, MP_KMCData *data, short types[])
{
	int i, j;
	double r, Ui;
	double rho, sV;
	double phi, V;

	if (types[0] != parm->type) return 0.0;
	for (i = 1, rho = 0.0, sV = 0.0; i < data->ncluster; i++) {
		if (types[i] == parm->type) {
			r = sqrt(data->cluster[i][0] * data->cluster[i][0]
				+ data->cluster[i][1] * data->cluster[i][1]
				+ data->cluster[i][2] * data->cluster[i][2]);
			for (j = 0, phi = 0.0; j < 2; j++) {
				if (parm->R[j] >= r) {
					phi += parm->A[j] * pow(parm->R[j] - r, 3.0);
				}
			}
			rho += phi;
			for (j = 0, V = 0.0; j < 6; j++) {
				if (parm->r[j] >= r) {
					V += parm->a[j] * pow(parm->r[j] - r, 3.0);
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
#ifndef _DEBUG

static void PyDealloc(MP_FSFCCParm* self)
{
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *PyNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	MP_FSFCCParm *self;

	self = (MP_FSFCCParm *)type->tp_alloc(type, 0);
	return (PyObject *)self;
}

static PyMemberDef PyMembers[] = {
	{ "type", T_SHORT, offsetof(MP_FSFCCParm, type), 1, "type" },
	{ NULL }  /* Sentinel */
};

static PyObject *PySetParm(MP_FSFCCParm *self, PyObject *args, PyObject *kwds)
{
	short type;
	static char *kwlist[] = { "type", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "h", kwlist, &type)) {
		return NULL;
	}
	return Py_BuildValue("i", MP_FSFCCSetParm(self, type));
}

static PyObject *PyEnergy(MP_FSFCCParm *self, PyObject *args, PyObject *kwds)
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

static PyMethodDef PyMethods[] = {
	{ "set_parm", (PyCFunction)PySetParm, METH_VARARGS | METH_KEYWORDS,
	"set_parm(type) : set parameter" },
	{ "energy", (PyCFunction)PyEnergy, METH_VARARGS | METH_KEYWORDS,
	"energy(kmc, types) : calculate cluster energy" },
	{ NULL }  /* Sentinel */
};

static PyGetSetDef PyGetSet[] = {
	{ NULL }  /* Sentinel */
};

PyTypeObject MP_FSFCCPyType = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"MPKMC.fsfcc",				/*tp_name*/
	sizeof(MP_FSFCCParm),		/*tp_basicsize*/
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

#endif /* _DEBUG */