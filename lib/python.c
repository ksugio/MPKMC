#ifndef _DEBUG

#include "MPKMC.h"
#include <numpy/arrayobject.h>

static void PyKMCDealloc(MP_KMCData* self)
{
	MP_KMCFree(self);
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *PyKMCNewNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	int nuc, nx, ny, nz, ncluster, nsolute_max, ntable_step, nevent_step;
	static char *kwlist[] = { "nuc", "nx", "ny", "nz", "ncluster", "nsolute_max", "ntable_step", "nevent_step", NULL };
	MP_KMCData *self;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "iiiiiiii", kwlist, &nuc, &nx, &ny, &nz, &ncluster, &nsolute_max, &ntable_step, &nevent_step)) {
		return NULL;
	}
	self = (MP_KMCData *)type->tp_alloc(type, 0);
	if (self != NULL) {
		if (!MP_KMCAlloc(self, nuc, nx, ny, nz, ncluster, nsolute_max, ntable_step, nevent_step)) {
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
	static char *kwlist[] = { "fname", NULL };
	MP_KMCData *self;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &fname)) {
		return NULL;
	}
	self = (MP_KMCData *)type->tp_alloc(type, 0);
	if (self != NULL) {
		if (!MP_KMCRead(self, fname)) {
			Py_DECREF(self);
			return NULL;
		}
	}
	return (PyObject *)self;
}

static PyMemberDef PyKMCMembers[] = {
	{ "nuc", T_INT, offsetof(MP_KMCData, nuc), 1, "number of atoms in unit cell" },
	{ "ntot", T_INT, offsetof(MP_KMCData, ntot), 1, "total number of allocated memory" },
	{ "ncluster", T_INT, offsetof(MP_KMCData, ncluster), 1, "number of atoms in cluster" },
	{ "nrot", T_INT, offsetof(MP_KMCData, nrot), 1, "number of rotation index" },
	{ "table_use", T_INT, offsetof(MP_KMCData, table_use), 0, "flag for using table" },
	{ "ntable", T_INT, offsetof(MP_KMCData, ntable), 1, "number of table" },
	{ "nsolute", T_INT, offsetof(MP_KMCData, nsolute), 1, "number of solute atoms" },
	{ "nsolute_max", T_INT, offsetof(MP_KMCData, nsolute_max), 1, "maximum number of solute atoms" },
	{ "nevent", T_INT, offsetof(MP_KMCData, nevent), 1, "number of events" },
	{ "step", T_INT, offsetof(MP_KMCData, step), 1, "step" },
	{ "rand_seed", T_LONG, offsetof(MP_KMCData, rand_seed), 0, "seed of random number" },
	{ "tote", T_DOUBLE, offsetof(MP_KMCData, tote), 1, "total energy" },
	{ NULL }  /* Sentinel */
};

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
	static char *kwlist[] = { "cluster", NULL };
	PyObject *tp;
	double dcluster[MP_KMC_NCLUSTER_MAX][3];
	int i, j;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &cluster)) {
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
	MP_KMCSetCluster(self, dcluster);
	Py_RETURN_NONE;
}

static PyObject *PyKMCRealPos(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *cp;
	static char *kwlist[] = { "cp", NULL };
	double dcp[3], drp[3];
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &cp)) {
		return NULL;
	}
	if (PyTuple_Size(cp) != 3) {
		return NULL;
	}
	for (i = 0; i < 3; i++) {
		dcp[i] = (double)PyFloat_AsDouble(PyTuple_GetItem(cp, (Py_ssize_t)i));
	}
	MP_KMCRealPos(self, dcp, drp);
	return Py_BuildValue("ddd", drp[0], drp[1], drp[2]);
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

static PyObject *PyKMCCalcEnergy(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	PyObject *func;
	static char *kwlist[] = { "id", "func", NULL };
	double ene;
	int update;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "iO", kwlist, &id, &func)) {
		return NULL;
	}
	if (!PyCallable_Check(func)) {
		PyErr_SetString(PyExc_TypeError, "func must be callable");
		return NULL;
	}
	self->pyfunc = func;
	ene = MP_KMCCalcEnergy(self, id, calcEnergy, &update);
	return Py_BuildValue("di", ene, update);
}

static PyObject *PyKMCTotalEnergy(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *func;
	static char *kwlist[] = { "func", NULL };
	double ene;
	int update;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &func)) {
		return NULL;
	}
	if (!PyCallable_Check(func)) {
		PyErr_SetString(PyExc_TypeError, "func must be callable");
		return NULL;
	}
	self->pyfunc = func;
	ene = MP_KMCTotalEnergy(self, calcEnergy, &update);
	return Py_BuildValue("di", ene, update);
}

static PyObject *PyKMCJump(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int ntry;
	double kt;
	PyObject *func;
	static char *kwlist[] = { "ntry", "kt", "func", NULL };
	int njump;
	int update;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "idO", kwlist, &ntry, &kt, &func)) {
		return NULL;
	}
	if (!PyCallable_Check(func)) {
		PyErr_SetString(PyExc_TypeError, "func must be callable");
		return NULL;
	}
	self->pyfunc = func;
	njump = MP_KMCJump(self, ntry, kt, calcEnergy, &update);
	return Py_BuildValue("ii", njump, update);
}

static PyObject *PyKMCStepForward(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int count;
	static char *kwlist[] = { "count", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &count)) {
		return NULL;
	}
	MP_KMCStepForward(self, count);
	Py_RETURN_NONE;
}

static PyObject *PyKMCStepBackward(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int count;
	static char *kwlist[] = { "count", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &count)) {
		return NULL;
	}
	MP_KMCStepBackward(self, count);
	Py_RETURN_NONE;
}

static PyObject *PyKMCStepGo(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int step;
	static char *kwlist[] = { "step", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &step)) {
		return NULL;
	}
	MP_KMCStepGo(self, step);
	Py_RETURN_NONE;
}

static PyObject *PyKMCEnergyHistory(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	PyObject *ehist_obj;
	static char *kwlist[] = { "ehist", NULL };
	PyArrayObject *ehist_arr;
	npy_intp nhist;
	double *ehist;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwlist, &PyArray_Type, &ehist_obj)) {
		return NULL;
	}
	ehist_arr = (PyArrayObject *)PyArray_FROM_OTF(ehist_obj, NPY_DOUBLE, NPY_INOUT_ARRAY);
	if (ehist_arr == NULL) return NULL;
	if (PyArray_NDIM(ehist_arr) != 1) {
		Py_XDECREF(ehist_arr);
		PyErr_SetString(PyExc_ValueError, "invalid ehist data, ndim must be 1");
		return NULL;
	}
	nhist = PyArray_DIM(ehist_arr, 0);
	ehist = (double *)PyArray_DATA(ehist_arr);
	MP_KMCEnergyHistory(self, nhist, ehist);
	Py_RETURN_NONE;
}

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

static PyObject *PyKMCTableItem(MP_KMCData *self, PyObject *args, PyObject *kwds)
{
	int id;
	static char *kwlist[] = { "id", NULL };
	PyObject *tps;
	int i;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &id)) {
		return NULL;
	}
	if (id >= 0 && id < self->ntable) {
		tps = PyTuple_New((Py_ssize_t)self->ncluster);
		for (i = 0; i < self->ncluster; i++) {
			PyTuple_SetItem(tps, (Py_ssize_t)i, PyInt_FromLong(self->table[id].types[i]));
		}
		return Py_BuildValue("Odl", tps, self->table[id].energy, self->table[id].refcount);
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
		return Py_BuildValue("ihh", self->solute[id].id, self->solute[id].type, self->solute[id].jump);
	}
	else return NULL;
}

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

static PyMethodDef PyKMCMethods[] = {
	{ "set_unitcell", (PyCFunction)PyKMCSetUnitCell, METH_VARARGS | METH_KEYWORDS,
	"set_unitcell(uc, types, pv) : set unit cell" },
	{ "set_cluster", (PyCFunction)PyKMCSetCluster, METH_VARARGS | METH_KEYWORDS,
	"set_cluster(cluster) : set atom position of cluster" },
	{ "real_pos", (PyCFunction)PyKMCRealPos, METH_VARARGS | METH_KEYWORDS,
	"real_pos(cp) : return real position" },
	{ "index2grid", (PyCFunction)PyKMCIndex2Grid, METH_VARARGS | METH_KEYWORDS,
	"index2grid(id) : return grid position from index" },
	{ "grid2index", (PyCFunction)PyKMCGrid2Index, METH_VARARGS | METH_KEYWORDS,
	"grid2index(x, y, z) : return index from grid position" },
	{ "cluster_indexes", (PyCFunction)PyKMCClusterIndexes, METH_VARARGS | METH_KEYWORDS,
	"cluster_indexes(id) : return cluster indexes" },
	{ "search_cluster", (PyCFunction)PyKMCSearchCluster, METH_VARARGS | METH_KEYWORDS,
	"search_cluster(types) : search cluster in table and return table index" },
	{ "search_cluster_ids", (PyCFunction)PyKMCSearchClusterIDs, METH_VARARGS | METH_KEYWORDS,
	"search_cluster_ids(ids) : search cluster in table and return table index" },
	{ "add_cluster", (PyCFunction)PyKMCAddCluster, METH_VARARGS | METH_KEYWORDS,
	"add_cluster(types) : add cluster in table and return table index" },
	{ "add_cluster_ids", (PyCFunction)PyKMCAddClusterIDs, METH_VARARGS | METH_KEYWORDS,
	"add_cluster_ids(ids) : add cluster in table and return table index" },
	{ "add_solute", (PyCFunction)PyKMCAddSolute, METH_VARARGS | METH_KEYWORDS,
	"add_solute(id, type, jump) : add a solute atom by id" },
	{ "add_solute_random", (PyCFunction)PyKMCAddSoluteRandom, METH_VARARGS | METH_KEYWORDS,
	"add_solute_random(num, type, jump) : add solute atoms randomly" },
	{ "calc_energy", (PyCFunction)PyKMCCalcEnergy, METH_VARARGS | METH_KEYWORDS,
	"calc_energy(id, func) : calculate energy" },
	{ "total_energy", (PyCFunction)PyKMCTotalEnergy, METH_VARARGS | METH_KEYWORDS,
	"total_energy(func) : calculate total energy" },
	{ "jump", (PyCFunction)PyKMCJump, METH_VARARGS | METH_KEYWORDS,
	"jump(ntry, kt, func) : jump diffusions by KMC method" },
	{ "step_forward", (PyCFunction)PyKMCStepForward, METH_VARARGS | METH_KEYWORDS,
	"step_forward(count) : take a step forward" },
	{ "step_backward", (PyCFunction)PyKMCStepBackward, METH_VARARGS | METH_KEYWORDS,
	"step_backward(count) : take a step backward" },
	{ "step_go", (PyCFunction)PyKMCStepGo, METH_VARARGS | METH_KEYWORDS,
	"step_go(step) : go to step" },
	{ "energy_history", (PyCFunction)PyKMCEnergyHistory, METH_VARARGS | METH_KEYWORDS,
	"energy_history(ehist) : setup energy history" },
	{ "write_table", (PyCFunction)PyKMCWriteTable, METH_VARARGS | METH_KEYWORDS,
	"write_table(filename) : write energy table" },
	{ "read_table", (PyCFunction)PyKMCReadTable, METH_VARARGS | METH_KEYWORDS,
	"read_table(filename) : read energy table" },
	{ "sort_table", (PyCFunction)PyKMCSortTable, METH_NOARGS,
	"sort_table() : sort energy table by reference count" },
	{ "reset_table", (PyCFunction)PyKMCResetTable, METH_NOARGS,
	"reset_table() : reset reference count of energy table" },
	{ "write", (PyCFunction)PyKMCWrite, METH_VARARGS | METH_KEYWORDS,
	"write(filename, comp) : write kmc data" },
	{ "table_item", (PyCFunction)PyKMCTableItem, METH_VARARGS | METH_KEYWORDS,
	"table_item(id) : return energy table item" },
	{ "grid_item", (PyCFunction)PyKMCGridItem, METH_VARARGS | METH_KEYWORDS,
	"grid_item(id) : return grid item" },
	{ "solute_item", (PyCFunction)PyKMCSoluteItem, METH_VARARGS | METH_KEYWORDS,
	"solute_item(id) : return solute item" },
	{ "add_rot_index", (PyCFunction)PyKMCAddRotIndex, METH_VARARGS | METH_KEYWORDS,
	"add_rot_index(ids) : add rotation index" },
	{ "calc_rot_index", (PyCFunction)PyKMCCalcRotIndex, METH_VARARGS | METH_KEYWORDS,
	"calc_rot_index(step, tol) : calculate rotation index" },
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
	{ "size", (getter)PyKMCGetSize, NULL, "size of grid", NULL },
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
	"new(lat_type, nx, ny, nz, solvent, ndiffuse_max, ntable_step)", 	/* tp_doc */
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
	"read(fname)",				/* tp_doc */
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

static PyMethodDef MPKMCPyMethods[] = {
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
	m = Py_InitModule3("MPKMC", MPKMCPyMethods, "MPKMC extention");
	if (m == NULL) return;
	import_array();
	Py_INCREF(&PyKMCNewType);
	PyModule_AddObject(m, "new", (PyObject *)&PyKMCNewType);
	Py_INCREF(&PyKMCReadType);
	PyModule_AddObject(m, "read", (PyObject *)&PyKMCReadType);
	Py_INCREF(&MP_FSFCCPyType);
	PyModule_AddObject(m, "fsfcc", (PyObject *)&MP_FSFCCPyType);
}

#endif /* _DEBUG */
