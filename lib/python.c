#ifdef MP_PYTHON_LIB

#include "MPKMC.h"
#include <numpy/arrayobject.h>

#define SOLUTE_GROUP_MAX 10000
#define SOLUTE_TYPES_MAX 128

static void PyKMCDealloc(MP_KMCData* self)
{
	MP_KMCFree(self);
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *PyKMCNewNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	int nuc, nx, ny, nz, ncluster;
	int nsolute_step = 1000;
	int ntable_step = 1000;
	int nevent_step = 100000;
	int nresult_step = 100;
	static char *kwlist[] = { "nuc", "nx", "ny", "nz", "ncluster", "nsolute_step", "ntable_step", "nevent_step", "nresult_step", NULL };
	MP_KMCData *self;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "iiiii|iiii", kwlist, &nuc, &nx, &ny, &nz, &ncluster,
		&nsolute_step, &ntable_step, &nevent_step, &nresult_step)) {
		return NULL;
	}
	self = (MP_KMCData *)type->tp_alloc(type, 0);
	if (self != NULL) {
		if (!MP_KMCAlloc(self, nuc, nx, ny, nz, ncluster, nsolute_step, ntable_step, nevent_step, nresult_step)) {
			Py_DECREF(self);
			return NULL;
		}
	}
	return (PyObject *)self;
}

static PyObject *PyKMCReadNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	char *fname;
	int version = 1;
	static char *kwlist[] = { "fname", "version", NULL };
	MP_KMCData *self;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|i", kwlist, &fname, &version)) {
		return NULL;
	}
	self = (MP_KMCData *)type->tp_alloc(type, 0);
	if (self != NULL) {
		if (!MP_KMCRead(self, fname, version)) {
			Py_DECREF(self);
			return NULL;
		}
	}
	return (PyObject *)self;
}

static PyMemberDef PyKMCMembers[] = {
	{ "nuc", T_INT, offsetof(MP_KMCData, nuc), 1, "number of atoms in unit cell" },
	{ "ntot", T_INT, offsetof(MP_KMCData, ntot), 1, "total number of atom sites" },
	{ "ncluster", T_INT, offsetof(MP_KMCData, ncluster), 1, "number of atoms in cluster" },
	{ "nrot", T_INT, offsetof(MP_KMCData, nrot), 1, "number of rotation index" },
	{ "table_use", T_INT, offsetof(MP_KMCData, table_use), 0, "flag for using table" },
	{ "ntable", T_INT, offsetof(MP_KMCData, ntable), 1, "number of table" },
	{ "nsolute", T_INT, offsetof(MP_KMCData, nsolute), 1, "number of solute atoms" },
	{ "ngroup", T_INT, offsetof(MP_KMCData, ngroup), 1, "number of solute groups" },
    { "event_record", T_INT, offsetof(MP_KMCData, event_record), 0, "flag for recording event" },
    { "nevent", T_INT, offsetof(MP_KMCData, nevent), 1, "number of events" },
    { "nhistory", T_INT, offsetof(MP_KMCData, nhistory), 1, "number of history" },
	{ "event_pt", T_INT, offsetof(MP_KMCData, event_pt), 1, "current event point" },
	{ "rand_seed", T_LONG, offsetof(MP_KMCData, rand_seed), 0, "seed of random number" },
	{ "totmcs", T_INT, offsetof(MP_KMCData, totmcs), 1, "total Monte Carlo step" },
	{ "mcs", T_INT, offsetof(MP_KMCData, mcs), 1, "current Monte Carlo step" },
	{ "tote", T_DOUBLE, offsetof(MP_KMCData, tote), 1, "total energy" },
    { "kb", T_DOUBLE, offsetof(MP_KMCData, kb), 0, "Boltzmann constant" },
	{ NULL }  /* Sentinel */
};

/*--------------------------------------------------
* kmc functions
*/
static PyObject *PyKMCSetUnitCell(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *uc, *types, *pv;
	static char *kwlist[] = { "uc", "types", "pv", NULL };
	PyObject *tp;
	double duc[MP_KMC_NUC_MAX][3];
	short stypes[MP_KMC_NUC_MAX];
	double dpv[3][3];
	int i, j;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO", kwlist, &uc, &types, &pv)) {
		return NULL;
	}
	if (PyTuple_Size(uc) != self->nuc || PyTuple_Size(types) != self->nuc || PyTuple_Size(pv) != 3) {
		return NULL;
	}
	for (i = 0; i < self->nuc; i++) {
		tp = PyTuple_GetItem(uc, (Py_ssize_t)i);
		for (j = 0; j < 3; j++) {
			duc[i][j] = (double)PyFloat_AsDouble(PyTuple_GetItem(tp, (Py_ssize_t)j));
		}
		stypes[i] = (short)PyInt_AsLong(PyTuple_GetItem(types, (Py_ssize_t)i));
	}
	for (i = 0; i < 3; i++) {
		tp = PyTuple_GetItem(pv, (Py_ssize_t)i);
		for (j = 0; j < 3; j++) {
			dpv[i][j] = (double)PyFloat_AsDouble(PyTuple_GetItem(tp, (Py_ssize_t)j));
		}
	}
	MP_KMCSetUnitCell(self, duc, stypes, dpv);
	Py_RETURN_NONE;
}

static PyObject *PyKMCSetCluster(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *cluster;
	int jpmax;
	static char *kwlist[] = { "cluster", "jpmax", NULL };
	PyObject *tp;
	double dcluster[MP_KMC_NCLUSTER_MAX][3];
	int i, j;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oi", kwlist, &cluster, &jpmax)) {
		return NULL;
	}
	if (PyTuple_Size(cluster) != self->ncluster) {
		return NULL;
	}
	for (i = 0; i < self->ncluster; i++) {
		tp = PyTuple_GetItem(cluster, (Py_ssize_t)i);
		for (j = 0; j < 3; j++) {
			dcluster[i][j] = (double)PyFloat_AsDouble(PyTuple_GetItem(tp, (Py_ssize_t)j));
		}
	}
	return Py_BuildValue("i", MP_KMCSetCluster(self, dcluster, jpmax));
}

static PyObject *PyKMCRealPos(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *pos;
	static char *kwlist[] = { "pos", NULL };
	double dpos[3], drpos[3];
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &pos)) {
		return NULL;
	}
	if (PyTuple_Size(pos) != 3) {
		return NULL;
	}
	for (i = 0; i < 3; i++) {
		dpos[i] = (double)PyFloat_AsDouble(PyTuple_GetItem(pos, (Py_ssize_t)i));
	}
	MP_KMCRealPos(self, dpos, drpos);
	return Py_BuildValue("ddd", drpos[0], drpos[1], drpos[2]);
}

static PyObject *PyKMCIndex2Grid(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };
	int p, x, y, z;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	MP_KMCIndex2Grid(self, id, &p, &x, &y, &z);
	return Py_BuildValue("iiii", p, x, y, z);
}

static PyObject *PyKMCGrid2Index(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int p, x, y, z;
	static char *kwlist[] = { "p", "x", "y", "z", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "iiii", kwlist, &p, &x, &y, &z)) {
		return NULL;
	}
	return Py_BuildValue("i", MP_KMCGrid2Index(self, p, x, y, z));
}

static PyObject *PyKMCIndex2Pos(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };
	double pos[3];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	MP_KMCIndex2Pos(self, id, pos);
	return Py_BuildValue("ddd", pos[0], pos[1], pos[2]);
}

static PyObject *PyKMCClusterIndexes(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };
	int ids[MP_KMC_NCLUSTER_MAX];
	PyObject *pyids;
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	MP_KMCClusterIndexes(self, id, ids);
	pyids = PyTuple_New((Py_ssize_t)self->ncluster);
	for (i = 0; i < self->ncluster; i++) {
		PyTuple_SetItem(pyids, (Py_ssize_t)i, PyInt_FromLong(ids[i]));
	}
	return pyids;
}

static PyObject *PyKMCClusterTypes(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };
	short types[MP_KMC_NCLUSTER_MAX];
	PyObject *pytypes;
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	MP_KMCClusterTypes(self, id, types);
	pytypes = PyTuple_New((Py_ssize_t)self->ncluster);
	for (i = 0; i < self->ncluster; i++) {
		PyTuple_SetItem(pytypes, (Py_ssize_t)i, PyInt_FromLong(types[i]));
	}
	return pytypes;
}

static PyObject *PyKMCSearchCluster(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *types;
	static char *kwlist[] = { "types", NULL };
	int i;
	short stypes[MP_KMC_NCLUSTER_MAX];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &types)) {
		return NULL;
	}
	if (PyTuple_Size(types) != self->ncluster) {
		return NULL;
	}
	for (i = 0; i < self->ncluster; i++) {
		stypes[i] = (short)PyInt_AsLong(PyTuple_GetItem(types, (Py_ssize_t)i));
	}
	return Py_BuildValue("i", MP_KMCSearchCluster(self, stypes));
}

static PyObject *PyKMCSearchClusterIDs(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *ids;
	static char *kwlist[] = { "ids", NULL };
	int i;
	int iids[MP_KMC_NCLUSTER_MAX];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &ids)) {
		return NULL;
	}
	if (PyTuple_Size(ids) != self->ncluster) {
		return NULL;
	}
	for (i = 0; i < self->ncluster; i++) {
		iids[i] = (int)PyInt_AsLong(PyTuple_GetItem(ids, (Py_ssize_t)i));
	}
	return Py_BuildValue("i", MP_KMCSearchClusterIDs(self, iids));
}

static PyObject *PyKMCAddCluster(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *types;
	double energy;
	long refcount;
	static char *kwlist[] = { "types", "energy", "refcount", NULL };
	int i;
	short stypes[MP_KMC_NCLUSTER_MAX];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "Odl", kwlist, &types, &energy, &refcount)) {
		return NULL;
	}
	if (PyTuple_Size(types) != self->ncluster) {
		return NULL;
	}
	for (i = 0; i < self->ncluster; i++) {
		stypes[i] = (short)PyInt_AsLong(PyTuple_GetItem(types, (Py_ssize_t)i));
	}
	return Py_BuildValue("i", MP_KMCAddCluster(self, stypes, energy, refcount));
}

static PyObject *PyKMCAddClusterIDs(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *ids;
	double energy;
	long refcount;
	static char *kwlist[] = { "ids", "energy", "refcount", NULL };
	int i;
	int iids[MP_KMC_NCLUSTER_MAX];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "Odl", kwlist, &ids, &energy, &refcount)) {
		return NULL;
	}
	if (PyTuple_Size(ids) != self->ncluster) {
		return NULL;
	}
	for (i = 0; i < self->ncluster; i++) {
		iids[i] = (int)PyInt_AsLong(PyTuple_GetItem(ids, (Py_ssize_t)i));
	}
	return Py_BuildValue("i", MP_KMCAddClusterIDs(self, iids, energy, refcount));
}

static PyObject *PyKMCCountType(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	short type;
	static char *kwlist[] = { "type", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "h", kwlist, &type)) {
		return NULL;
	}
	return Py_BuildValue("i", MP_KMCCountType(self, type));
}

static PyObject *PyKMCSortTable(MP_KMCData *self, PyObject *args)
{
	MP_KMCSortTable(self);
	Py_RETURN_NONE;
}

static PyObject *PyKMCResetTable(MP_KMCData *self, PyObject *args)
{
	MP_KMCResetTable(self);
	Py_RETURN_NONE;
}

static PyObject *TableItem(int ncluster, MP_KMCTableItem item)
{
	int i;
	PyObject *tps;

	tps = PyTuple_New((Py_ssize_t)ncluster);
	for (i = 0; i < ncluster; i++) {
		PyTuple_SetItem(tps, (Py_ssize_t)i, PyInt_FromLong(item.types[i]));
	}
	return Py_BuildValue("Odl", tps, item.energy, item.refcount);
}

static PyObject *PyKMCSearchTable(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	char *ss;
	static char *kwlist[] = { "ss", NULL };
	int nlist;
	MP_KMCTableItem list[1024];
	PyObject *tb;
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &ss)) {
		return NULL;
	}
	nlist = MP_KMCSearchTable(self, ss, list, 1024);
	tb = PyTuple_New((Py_ssize_t)nlist);
	for (i = 0; i < nlist; i++) {
		PyTuple_SetItem(tb, (Py_ssize_t)i, TableItem(self->ncluster, list[i]));
	}
	return Py_BuildValue("iO", nlist, tb);
}

/*--------------------------------------------------
* solute functions
*/
static PyObject *PyKMCAddSolute(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	short type;
	short jump;
	static char *kwlist[] = { "id", "type", "jump", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ihh", kwlist, &id, &type, &jump)) {
		return NULL;
	}
	return Py_BuildValue("i", MP_KMCAddSolute(self, id, type, jump));
}

static PyObject *PyKMCAddSoluteRandom(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int num;
	short type;
	short jump;
	static char *kwlist[] = { "num", "type", "jump", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ihh", kwlist, &num, &type, &jump)) {
		return NULL;
	}
	MP_KMCAddSoluteRandom(self, num, type, jump);
	Py_RETURN_NONE;
}

static PyObject *PyKMCCheckSolute(MP_KMCData *self, PyObject *args)
{
	return Py_BuildValue("i", MP_KMCCheckSolute(self));
}

static PyObject *PyKMCSoluteTypes(MP_KMCData *self, PyObject *args)
{
	int ntypes;
	short types[SOLUTE_TYPES_MAX];
	PyObject *pytypes;
	int i;

	ntypes = MP_KMCSoluteTypes(self, SOLUTE_TYPES_MAX, types);
	pytypes = PyTuple_New((Py_ssize_t)ntypes);
	for (i = 0; i < ntypes; i++) {
		PyTuple_SetItem(pytypes, (Py_ssize_t)i, PyInt_FromLong(types[i]));
	}
	return pytypes;
}

static PyObject *PyKMCFindSoluteGroup(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	double rcut;
	static char *kwlist[] = { "rcut", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "d", kwlist, &rcut)) {
		return NULL;
	}
	return Py_BuildValue("i", MP_KMCFindSoluteGroup(self, rcut));
}

static PyObject *PyKMCSoluteGroupIndexes(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int group;
	static char *kwlist[] = { "group", NULL };
	int nsolute;
	int ids[SOLUTE_GROUP_MAX];
	PyObject *pyids;
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &group)) {
		return NULL;
	}
	nsolute = MP_KMCSoluteGroupIndexes(self, group, SOLUTE_GROUP_MAX, ids);
	pyids = PyTuple_New((Py_ssize_t)nsolute);
	for (i = 0; i < nsolute; i++) {
		PyTuple_SetItem(pyids, (Py_ssize_t)i, PyInt_FromLong(ids[i]));
	}
	return pyids;
}

static PyObject *PyKMCSoluteGroupTypes(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int group;
	static char *kwlist[] = { "group", NULL };
	int nsolute;
	short types[SOLUTE_GROUP_MAX];
	PyObject *pytypes;
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &group)) {
		return NULL;
	}
	nsolute = MP_KMCSoluteGroupTypes(self, group, SOLUTE_GROUP_MAX, types);
	pytypes = PyTuple_New((Py_ssize_t)nsolute);
	for (i = 0; i < nsolute; i++) {
		PyTuple_SetItem(pytypes, (Py_ssize_t)i, PyInt_FromLong(types[i]));
	}
	return pytypes;
}

/*--------------------------------------------------
* jump functions
*/
static double calcEnergy(MP_KMCData *data, short types[])
{
	int i;
	PyObject *tps;
	PyObject *arg;
	PyObject *res;
	double energy;

	tps = PyTuple_New((Py_ssize_t)data->ncluster);
	for (i = 0; i < data->ncluster; i++) {
		PyTuple_SetItem(tps, (Py_ssize_t)i, PyInt_FromLong(types[i]));
	}
	arg = Py_BuildValue("OO", (PyObject *)data, tps);
	res = PyObject_CallObject(data->pyfunc, arg);
	Py_DECREF(tps);
	Py_DECREF(arg);
	if (res == NULL) {
		return 0.0;
	}
	energy = PyFloat_AsDouble(res);
	Py_DECREF(res);
	return energy;
}

static PyObject *PyKMCGridEnergy(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *func;
	static char *kwlist[] = { "func", NULL };
	int update;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &func)) {
		return NULL;
	}
	if (func == Py_None) {
		update = MP_KMCGridEnergy(self, NULL);
	}
	else {
		if (!PyCallable_Check(func)) {
			PyErr_SetString(PyExc_TypeError, "func must be callable");
			return NULL;
		}
		self->pyfunc = func;
		update = MP_KMCGridEnergy(self, calcEnergy);
	}
	return Py_BuildValue("i", update);
}

static PyObject *PyKMCJump(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int ntry;
	double temp;
	PyObject *func;
	static char *kwlist[] = { "ntry", "temp", "func", NULL };
	MP_KMCHistoryItem ret;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "idO", kwlist, &ntry, &temp, &func)) {
		return NULL;
	}
	if (func == Py_None) {
		ret = MP_KMCJump(self, ntry, temp, NULL);
	}
	else {
		if (!PyCallable_Check(func)) {
			PyErr_SetString(PyExc_TypeError, "func must be callable");
			return NULL;
		}
		self->pyfunc = func;
		ret = MP_KMCJump(self, ntry, temp, calcEnergy);
	}
	return Py_BuildValue("ldiiiidd", ret.totmcs, ret.temp, ret.ntry, ret.njump, 
		ret.table_update, ret.ntable, ret.tote, ret.time);
}

/*--------------------------------------------------
* event functions
*/
static PyObject *PyKMCEventForward(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int count;
	static char *kwlist[] = { "count", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &count)) {
		return NULL;
	}
	MP_KMCEventForward(self, count);
	Py_RETURN_NONE;
}

static PyObject *PyKMCEventBackward(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int count;
	static char *kwlist[] = { "count", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &count)) {
		return NULL;
	}
	MP_KMCEventBackward(self, count);
	Py_RETURN_NONE;
}

static PyObject *PyKMCEventGo(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int event_pt;
	static char *kwlist[] = { "event_pt", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &event_pt)) {
		return NULL;
	}
	MP_KMCEventGo(self, event_pt);
	Py_RETURN_NONE;
}

static PyObject *PyKMCEventPt2MCS(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int event_pt;
	static char *kwlist[] = { "event_pt", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &event_pt)) {
		return NULL;
	}
	return Py_BuildValue("l", MP_KMCEventPt2MCS(self, event_pt));
}

static PyObject *PyKMCMCS2EventPt(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	long mcs;
	static char *kwlist[] = { "mcs", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "l", kwlist, &mcs)) {
		return NULL;
	}
	return Py_BuildValue("i", MP_KMCMCS2EventPt(self, mcs));
}

static PyObject *PyKMCEventMCS(MP_KMCData *self, PyObject *args)
{
	PyObject *mcs_obj;
	npy_intp dims[1];

	dims[0] = self->nevent + 1;
	mcs_obj = PyArray_SimpleNew(1, dims, NPY_FLOAT64);
	if (mcs_obj == NULL) return NULL;
	MP_KMCEventMCS(self, dims[0], (double *)PyArray_DATA(mcs_obj));
	return mcs_obj;
}

static PyObject *PyKMCEventEnergy(MP_KMCData *self, PyObject *args)
{
	PyObject *ene_obj;
	npy_intp dims[1];

	dims[0] = self->nevent + 1;
	ene_obj = PyArray_SimpleNew(1, dims, NPY_FLOAT64);
	if (ene_obj == NULL) return NULL;
	MP_KMCEventEnergy(self, dims[0], (double *)PyArray_DATA(ene_obj));
	return ene_obj;
}

static PyObject *PyKMCEventMSD(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	short type;
	int event_pt;
	static char *kwlist[] = { "type", "event_pt", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "hi", kwlist, &type, &event_pt)) {
		return NULL;
	}
	return Py_BuildValue("d", MP_KMCEventMSD(self, type, event_pt));
}

/*--------------------------------------------------
* rw functions
*/
static PyObject *PyKMCWriteTable(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	char *filename;
	static char *kwlist[] = { "filename", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &filename)) {
		return NULL;
	}
	return Py_BuildValue("i", MP_KMCWriteTable(self, filename));
}

static PyObject *PyKMCReadTable(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	char *filename;
	static char *kwlist[] = { "filename", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &filename)) {
		return NULL;
	}
	return Py_BuildValue("i", MP_KMCReadTable(self, filename));
}

static PyObject *PyKMCWrite(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	char *filename;
	int comp;
	static char *kwlist[] = { "filename", "comp", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "si", kwlist, &filename, &comp)) {
		return NULL;
	}
	return Py_BuildValue("i", MP_KMCWrite(self, filename, comp));
}

/*--------------------------------------------------
* rotindex functions
*/
static PyObject *PyKMCAddRotIndex(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *ids;
	static char *kwlist[] = { "ids", NULL };
	int i;
	int iids[MP_KMC_NCLUSTER_MAX];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &ids)) {
		return NULL;
	}
	if (PyTuple_Size(ids) != self->ncluster) {
		return NULL;
	}
	for (i = 0; i < self->ncluster; i++) {
		iids[i] = (int)PyInt_AsLong(PyTuple_GetItem(ids, (Py_ssize_t)i));
	}
	return Py_BuildValue("i", MP_KMCAddRotIndex(self, iids));
}

static PyObject *PyKMCCalcRotIndex(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	double step;
	double tol;
	static char *kwlist[] = { "step", "tol", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "dd", kwlist, &step, &tol)) {
		return NULL;
	}
	return Py_BuildValue("i", MP_KMCCalcRotIndex(self, step, tol));
}

/*--------------------------------------------------
* items
*/
static PyObject *PyKMCTableItem(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id >= 0 && id < self->ntable) {
		return TableItem(self->ncluster, self->table[id]);
	}
	else return NULL;
}

static PyObject *PyKMCGridItem(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id >= 0 && id < self->ntot) {
		return Py_BuildValue("hd", self->grid[id].type, self->grid[id].energy);
	}
	else return NULL;
}

static PyObject *PyKMCSoluteItem(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id >= 0 && id < self->nsolute) {
		return Py_BuildValue("ihhii", self->solute[id].id, self->solute[id].type, self->solute[id].jump,
			self->solute[id].njump, self->solute[id].group);
	}
	else return NULL;
}

static PyObject *PyKMCHistoryItem(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id >= 0 && id < self->nhistory) {
		return Py_BuildValue("ldiiiidd", self->history[id].totmcs, self->history[id].temp, self->history[id].ntry,
			self->history[id].njump, self->history[id].table_update, self->history[id].ntable,
			self->history[id].tote, self->history[id].time);
	}
	else return NULL;
}

static PyMethodDef PyKMCMethods[] = {
	// kmc
	{ "set_unitcell", (PyCFunction)PyKMCSetUnitCell, METH_VARARGS | METH_KEYWORDS,
		"set_unitcell(uc, types, pv) : set atom positons, types and primitive vector of unit cell" },
	{ "set_cluster", (PyCFunction)PyKMCSetCluster, METH_VARARGS | METH_KEYWORDS,
		"set_cluster(cluster, jpmax) : set atom positions of cluster and maximum jump pointer, return true if it succeeds" },
	{ "real_pos", (PyCFunction)PyKMCRealPos, METH_VARARGS | METH_KEYWORDS,
		"real_pos(cp) : return real position from unit cell position" },
	{ "index2grid", (PyCFunction)PyKMCIndex2Grid, METH_VARARGS | METH_KEYWORDS,
		"index2grid(id) : return grid position of i-th atom" },
	{ "grid2index", (PyCFunction)PyKMCGrid2Index, METH_VARARGS | METH_KEYWORDS,
		"grid2index(p, x, y, z) : return index from grid position" },
	{ "index2pos", (PyCFunction)PyKMCIndex2Pos, METH_VARARGS | METH_KEYWORDS,
		"index2pos(id) : return atom position of i-th atom" },
	{ "cluster_indexes", (PyCFunction)PyKMCClusterIndexes, METH_VARARGS | METH_KEYWORDS,
		"cluster_indexes(id) : return indexes around i-th atom" },
    { "cluster_types", (PyCFunction)PyKMCClusterTypes, METH_VARARGS | METH_KEYWORDS,
		"cluster_types(id) : return types around i-th atom" },
	{ "search_cluster", (PyCFunction)PyKMCSearchCluster, METH_VARARGS | METH_KEYWORDS,
		"search_cluster(types) : search cluster in table and return table index" },
	{ "search_cluster_ids", (PyCFunction)PyKMCSearchClusterIDs, METH_VARARGS | METH_KEYWORDS,
		"search_cluster_ids(ids) : search cluster in table from atom indexes and return table index" },
	{ "add_cluster", (PyCFunction)PyKMCAddCluster, METH_VARARGS | METH_KEYWORDS,
		"add_cluster(types) : add cluster in table and return table index" },
	{ "add_cluster_ids", (PyCFunction)PyKMCAddClusterIDs, METH_VARARGS | METH_KEYWORDS,
		"add_cluster_ids(ids) : add cluster in table by atom indexes and return table index" },
	{ "count_type", (PyCFunction)PyKMCCountType, METH_VARARGS | METH_KEYWORDS,
		"count_type(type) : count type in grid" },
	{ "sort_table", (PyCFunction)PyKMCSortTable, METH_NOARGS,
		"sort_table() : sort energy table by reference count" },
	{ "reset_table", (PyCFunction)PyKMCResetTable, METH_NOARGS,
		"reset_table() : reset reference count of energy table" },
	{ "search_table", (PyCFunction)PyKMCSearchTable, METH_VARARGS | METH_KEYWORDS,
		"search_table(ss) : search cluster with search string, ex. p0t14,t14n3" },
	// solute
	{ "add_solute", (PyCFunction)PyKMCAddSolute, METH_VARARGS | METH_KEYWORDS,
		"add_solute(id, type, jump) : add a solute atom by an atom index" },
	{ "add_solute_random", (PyCFunction)PyKMCAddSoluteRandom, METH_VARARGS | METH_KEYWORDS,
		"add_solute_random(num, type, jump) : add solute atoms randomly" },
	{ "check_solute", (PyCFunction)PyKMCCheckSolute, METH_NOARGS,
		"check_solute() : check solute table" },
	{ "solute_types", (PyCFunction)PyKMCSoluteTypes, METH_NOARGS,
		"solute_types() : return solute types" },
    { "find_solute_group", (PyCFunction)PyKMCFindSoluteGroup, METH_VARARGS | METH_KEYWORDS,
		"find_solute_group(rcut) : find solute group, rcut is cutoff radius" },
	{ "solute_group_indexes", (PyCFunction)PyKMCSoluteGroupIndexes, METH_VARARGS | METH_KEYWORDS,
		"solute_group_indexes(group) : return solute group indexes" },
	{ "solute_group_types", (PyCFunction)PyKMCSoluteGroupTypes, METH_VARARGS | METH_KEYWORDS,
		"solute_group_types(group) : return solute group types" },
	// jump
	{ "grid_energy", (PyCFunction)PyKMCGridEnergy, METH_VARARGS | METH_KEYWORDS,
		"grid_energy(func) : calculate energies of grids, return table update flag" },
	{ "jump", (PyCFunction)PyKMCJump, METH_VARARGS | METH_KEYWORDS,
		"jump(ntry, temp, func) : jump solute atoms by KMC method" },
	// event
	{ "event_forward", (PyCFunction)PyKMCEventForward, METH_VARARGS | METH_KEYWORDS,
		"event_forward(count) : take event point forward" },
	{ "event_backward", (PyCFunction)PyKMCEventBackward, METH_VARARGS | METH_KEYWORDS,
		"event_backward(count) : take event point backward" },
	{ "event_go", (PyCFunction)PyKMCEventGo, METH_VARARGS | METH_KEYWORDS,
		"event_go(event_pt) : go to event point" },
    { "eventpt2mcs", (PyCFunction)PyKMCEventPt2MCS, METH_VARARGS | METH_KEYWORDS,
		"eventpt2mcs(mcs) : event point to Monte Carlo step" },
    { "mcs2eventpt", (PyCFunction)PyKMCMCS2EventPt, METH_VARARGS | METH_KEYWORDS,
		"mcs2eventpt(mcs) : Monte Carlo step to event point" },
    { "event_mcs", (PyCFunction)PyKMCEventMCS, METH_NOARGS,
		"event_mcs() : return MCSs at each event" },
	{ "event_energy", (PyCFunction)PyKMCEventEnergy, METH_NOARGS,
		"event_energy() : return energies at each event" },
	{ "event_msd", (PyCFunction)PyKMCEventMSD, METH_NOARGS,
		"event_msd(type, event_pt) : return mean square displacement" },
	// rw
	{ "write_table", (PyCFunction)PyKMCWriteTable, METH_VARARGS | METH_KEYWORDS,
		"write_table(filename) : write energy table" },
	{ "read_table", (PyCFunction)PyKMCReadTable, METH_VARARGS | METH_KEYWORDS,
		"read_table(filename) : read energy table" },
	{ "write", (PyCFunction)PyKMCWrite, METH_VARARGS | METH_KEYWORDS,
		"write(filename, comp) : write kmc data" },
	// rotindex
	{ "add_rot_index", (PyCFunction)PyKMCAddRotIndex, METH_VARARGS | METH_KEYWORDS,
		"add_rot_index(ids) : add rotation index" },
	{ "calc_rot_index", (PyCFunction)PyKMCCalcRotIndex, METH_VARARGS | METH_KEYWORDS,
		"calc_rot_index(step, tol) : calculate rotation index" },
	// items
	{ "table_item", (PyCFunction)PyKMCTableItem, METH_VARARGS | METH_KEYWORDS,
		"table_item(id) : return i-th item in energy table" },
	{ "grid_item", (PyCFunction)PyKMCGridItem, METH_VARARGS | METH_KEYWORDS,
		"grid_item(id) : return grid item of i-th atom" },
	{ "solute_item", (PyCFunction)PyKMCSoluteItem, METH_VARARGS | METH_KEYWORDS,
		"solute_item(id) : return i-th item in solute table" },
    { "history_item", (PyCFunction)PyKMCHistoryItem, METH_VARARGS | METH_KEYWORDS,
		"history_item(id) : return i-th item in history table" },
	{ NULL }  /* Sentinel */
};

static PyObject *PyKMCGetUC(MP_KMCData *self, void *closure)
{
	PyObject *tps;
	int i;

	tps = PyTuple_New((Py_ssize_t)self->nuc);
	for (i = 0; i < self->nuc; i++) {
		PyTuple_SetItem(tps, (Py_ssize_t)i,
			Py_BuildValue("ddd", self->uc[i][0], self->uc[i][1], self->uc[i][2]));
	}
	return tps;
}

static PyObject *PyKMCGetUCTypes(MP_KMCData *self, void *closure)
{
	PyObject *tps;
	int i;

	tps = PyTuple_New((Py_ssize_t)self->nuc);
	for (i = 0; i < self->nuc; i++) {
		PyTuple_SetItem(tps, (Py_ssize_t)i, PyInt_FromLong(self->uc_types[i]));
	}
	return tps;
}

static PyObject *PyKMCGetPV(MP_KMCData *self, void *closure)
{
	return Py_BuildValue("(ddd)(ddd)(ddd)",
		self->pv[0][0], self->pv[0][1], self->pv[0][2],
		self->pv[1][0], self->pv[1][1], self->pv[1][2], 
		self->pv[2][0], self->pv[2][1], self->pv[2][2]);
}

static PyObject *PyKMCGetCluster(MP_KMCData *self, void *closure)
{
	PyObject *tps;
	int i;

	tps = PyTuple_New((Py_ssize_t)self->ncluster);
	for (i = 0; i < self->ncluster; i++) {
		PyTuple_SetItem(tps, (Py_ssize_t)i,
			Py_BuildValue("ddd", self->cluster[i][0], self->cluster[i][1], self->cluster[i][2]));
	}
	return tps;
}

static PyObject *PyKMCGetRCluster(MP_KMCData *self, void *closure)
{
	PyObject *tps;
	int i;

	tps = PyTuple_New((Py_ssize_t)self->ncluster);
	for (i = 0; i < self->ncluster; i++) {
		PyTuple_SetItem(tps, (Py_ssize_t)i,
			Py_BuildValue("ddd", self->rcluster[i][0], self->rcluster[i][1], self->rcluster[i][2]));
	}
	return tps;
}

static PyObject *PyKMCGetSize(MP_KMCData *self, void *closure)
{
	return Py_BuildValue("iii", self->size[0], self->size[1], self->size[2]);
}

static PyObject *PyKMCGetHTable(MP_KMCData *self, void *closure)
{
	return Py_BuildValue("s", self->htable);
}

static int PyKMCSetHTable(MP_KMCData *self, PyObject *value, void *closure)
{
	char *htable = PyString_AsString(value);

	if (htable == NULL) return -1;
	else {
		strcpy(self->htable, htable);
		return 0;
	}
}

static PyGetSetDef PyKMCGetSet[] = {
	{ "uc", (getter)PyKMCGetUC, NULL, "atom positions of unit cell", NULL },
	{ "uc_types", (getter)PyKMCGetUCTypes, NULL, "atom types of unit cell", NULL },
	{ "pv", (getter)PyKMCGetPV, NULL, "primitive vector", NULL },
	{ "cluster", (getter)PyKMCGetCluster, NULL, "atom positions of cluster", NULL },
	{ "rcluster", (getter)PyKMCGetRCluster, NULL, "real atom positions of cluster", NULL },
	{ "size", (getter)PyKMCGetSize, NULL, "size of simulation cell", NULL },
	{ "htable", (getter)PyKMCGetHTable, (setter)PyKMCSetHTable, "header of table", NULL },
	{ NULL }  /* Sentinel */
};

static PyTypeObject PyKMCNewType = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"MPKMC.new",				/*tp_name*/
	sizeof(MP_KMCData),			/*tp_basicsize*/
	0,							/*tp_itemsize*/
	(destructor)PyKMCDealloc,	/*tp_dealloc*/
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
	"new(nuc, nx, ny, nz, ncluster, [nsolute_max=1000, ntable_step=1000, nevent_step=100000, nresult_step=100])", 	/* tp_doc */
	0,							/* tp_traverse */
	0,							/* tp_clear */
	0,							/* tp_richcompare */
	0,							/* tp_weaklistoffset */
	0,							/* tp_iter */
	0,							/* tp_iternext */
	PyKMCMethods,				/* tp_methods */
	PyKMCMembers,				/* tp_members */
	PyKMCGetSet,				/* tp_getset */
	0,							/* tp_base */
	0,							/* tp_dict */
	0,							/* tp_descr_get */
	0,							/* tp_descr_set */
	0,							/* tp_dictoffset */
	0,							/* tp_init */
	0,							/* tp_alloc */
	PyKMCNewNew,				/* tp_new */
};

static PyTypeObject PyKMCReadType = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"MPKMC.read",				/*tp_name*/
	sizeof(MP_KMCData),		/*tp_basicsize*/
	0,							/*tp_itemsize*/
	(destructor)PyKMCDealloc,	/*tp_dealloc*/
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
	"read(fname, [version=1])",				/* tp_doc */
	0,							/* tp_traverse */
	0,							/* tp_clear */
	0,							/* tp_richcompare */
	0,							/* tp_weaklistoffset */
	0,							/* tp_iter */
	0,							/* tp_iternext */
	PyKMCMethods,				/* tp_methods */
	PyKMCMembers,				/* tp_members */
	PyKMCGetSet,				/* tp_getset */
	0,							/* tp_base */
	0,							/* tp_dict */
	0,							/* tp_descr_get */
	0,							/* tp_descr_set */
	0,							/* tp_dictoffset */
	0,							/* tp_init */
	0,							/* tp_alloc */
	PyKMCReadNew,				/* tp_new */
};

static PyObject *PyKMCTypes2String(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *types;
	static char *kwlist[] = { "types", NULL };
	int i;
	int ncluster;
	short stypes[MP_KMC_NCLUSTER_MAX];
	char str[512];

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &types)) {
		return NULL;
	}
	ncluster = PyTuple_Size(types);
	for (i = 0; i < ncluster; i++) {
		stypes[i] = (short)PyInt_AsLong(PyTuple_GetItem(types, (Py_ssize_t)i));
	}
	MP_KMCTypes2String(ncluster, stypes, str);
	return Py_BuildValue("s", str);
}

static PyObject *PyKMCString2Types(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	char *str;
	static char *kwlist[] = { "str", NULL };
	short types[MP_KMC_NCLUSTER_MAX];
	int count;
	PyObject *tps;
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &str)) {
		return NULL;
	}
	count = MP_KMCString2Types(str, types);
	tps = PyTuple_New((Py_ssize_t)count);
	for (i = 0; i < count; i++) {
		PyTuple_SetItem(tps, (Py_ssize_t)i, PyInt_FromLong(types[i]));
	}
	return tps;
}

static PyMethodDef MPKMCPyMethods[] = {
	{ "types2string", (PyCFunction)PyKMCTypes2String, METH_VARARGS | METH_KEYWORDS,
	"types2string(types) : generate an unique string from types and return" },
	{ "string2types", (PyCFunction)PyKMCString2Types, METH_VARARGS | METH_KEYWORDS,
	"string2types(str) : return types from an unique string" },
	{ NULL }  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC initMPKMC(void)
{
	PyObject *m;

	if (PyType_Ready(&PyKMCNewType) < 0) return;
	if (PyType_Ready(&PyKMCReadType) < 0) return;
	if (PyType_Ready(&MP_FSFCCPyType) < 0) return;
	if (PyType_Ready(&MP_MEAMPyType) < 0) return;
	m = Py_InitModule3("MPKMC", MPKMCPyMethods, "MPKMC extention");
	if (m == NULL) return;
	import_array();
	Py_INCREF(&PyKMCNewType);
	PyModule_AddObject(m, "new", (PyObject *)&PyKMCNewType);
	Py_INCREF(&PyKMCReadType);
	PyModule_AddObject(m, "read", (PyObject *)&PyKMCReadType);
	Py_INCREF(&MP_FSFCCPyType);
	PyModule_AddObject(m, "fsfcc", (PyObject *)&MP_FSFCCPyType);
	Py_INCREF(&MP_MEAMPyType);
	PyModule_AddObject(m, "meam", (PyObject *)&MP_MEAMPyType);
}

#endif /* MP_PYTHON_LIB */
