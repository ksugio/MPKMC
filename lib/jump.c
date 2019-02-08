#include "MPKMC.h"

static int UniteIndexes(int ncluster, int ids0[], int ids1[], int ids2[])
{
	int i, j;
	int count = ncluster;

	for (i = 0; i < ncluster; i++) {
		ids2[i] = ids0[i];
	}
	count = i;
	for (j = 0; j < ncluster; j++) {
		for (i = 0; i < ncluster; i++) {
			if (ids1[j] == ids0[i]) break;
		}
		if (i == ncluster) {
			ids2[count++] = ids1[j];
		}
	}
	return count;
}

static void SwapType(MP_KMCData *data, int id0, int id1)
{
	int type0;

	type0 = data->grid[id0].type;
	data->grid[id0].type = data->grid[id1].type;
	data->grid[id1].type = type0;
}

static double GridClusterEnergy(MP_KMCData *data, int nncluster, int ids[])
{
	int i;
	double cle = 0.0;

	for (i = 0; i < nncluster; i++) {
		cle += data->grid[ids[i]].energy;
	}
	return cle;
}

static int CompareTypes(MP_KMCData *data, short types0[], short types1[])
{
	int j, k;
	int count;
	int sp;

	if (types0[0] == types1[0]) {
		for (j = 0; j < data->nrot; j++) {
			count = 0;
			sp = j * data->ncluster;
			for (k = 1; k < data->ncluster; k++) {
				if (types0[k] == types1[data->rotid[sp + k]]) count++;
			}
			if (count == data->ncluster - 1) return TRUE;
		}
	}
	return FALSE;
}

static void CalcClusterEnergies(MP_KMCData *data, double(*func)(MP_KMCData *, short *),
	int ncluster, int ids[], double energy[], int *update)
{
	int j, k;
	int ttid;
	int tid[MP_KMC_NCLUSTER_MAX * 2];
	short types[MP_KMC_NCLUSTER_MAX * 2][MP_KMC_NCLUSTER_MAX];

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (j = 0; j < ncluster; j++) {
		if (data->grid[ids[j]].type > 0) {
			MP_KMCClusterTypes(data, ids[j], types[j]);
			if (data->table_use) {
				tid[j] = MP_KMCSearchCluster(data, types[j]);
				if (tid[j] >= 0) {
					energy[j] = data->table[tid[j]].energy;
				}
				else {
					if (func != NULL) {
						energy[j] = (func)(data, types[j]);
					}
					else {
						energy[j] = 0.0;
					}
				}
			}
			else {
				energy[j] = (func)(data, types[j]);
			}
		}
		else if (data->grid[ids[j]].type == 0) {
			tid[j] = -99;
			energy[j] = 0.0;
		}
	}
	if (data->table_use) {
		for (j = 0; j < ncluster; j++) {
			if (tid[j] >= 0) {
				data->table[tid[j]].refcount += 1;
			}
			else if (tid[j] == -1) {
				ttid = MP_KMCAddCluster(data, types[j], energy[j], 0);
				for (k = j + 1; k < ncluster; k++) {
					if (tid[k] == -1 && CompareTypes(data, types[j], types[k])) {
						tid[k] = ttid;
					}
				}
				*update = TRUE;
			}
		}
	}
}

static double ClusterEnergy(MP_KMCData *data, double(*func)(MP_KMCData *, short *),
	int ncluster, int ids[], double energy[], int *update)
{
	int j;
	double cle = 0.0;

	CalcClusterEnergies(data, func, ncluster, ids, energy, update);
	for (j = 0; j < ncluster; j++) {
		cle += energy[j];
	}
	return cle;
}

int KMCAddEvent(MP_KMCData *data, int dp, int id0, int id1, double de, int dmcs)
{
	int nevent_max;
	int eid;

	if (data->nevent >= data->nevent_max) {
		nevent_max = data->nevent_max + data->nevent_step;
		data->event = (MP_KMCEventItem *)realloc(data->event, nevent_max * sizeof(MP_KMCEventItem));
		if (data->event == NULL) {
			fprintf(stderr, "Error : allocation failure (KMCAddEvent)\n");
			return  MP_KMC_MEM_ERR;
		}
		data->nevent_max = nevent_max;
	}
	eid = data->nevent;
	data->event[eid].dp = dp;
	data->event[eid].id0 = id0;
	data->event[eid].id1 = id1;
	data->event[eid].de = de;
	data->event[eid].dmcs = dmcs;
	data->nevent++;
	data->step = data->nevent;
	return eid;
}

double MP_KMCTotalEnergy(MP_KMCData *data, double(*func)(MP_KMCData *, short *), int *update)
{
	int j;
	int id = 0;
	int ncluster = 0;
	int step = MP_KMC_NCLUSTER_MAX * 2;
	int ids[MP_KMC_NCLUSTER_MAX * 2];
	double energy[MP_KMC_NCLUSTER_MAX * 2];

	while (TRUE) {
		ids[ncluster++] = id++;
		if (ncluster >= step || id >= data->ntot) {
			CalcClusterEnergies(data, func, ncluster, ids, energy, update);
			for (j = 0; j < ncluster; j++) {
				data->grid[ids[j]].energy = energy[j];
				data->tote += energy[j];
			}
			ncluster = 0;
			if (id >= data->ntot) break;
		}
	}
	return data->tote;
}

int MP_KMCJump(MP_KMCData *data, int ntry, double temp, double(*func)(MP_KMCData *, short *), int *update)
{
	int j, c;
	int dp, cp;
	int id0, id1;
	int ids0[MP_KMC_NCLUSTER_MAX];
	int ids1[MP_KMC_NCLUSTER_MAX];
	int ids2[MP_KMC_NCLUSTER_MAX * 2];
	int nncluster;
	double kt;
	double energy[MP_KMC_NCLUSTER_MAX * 2];
	double cle0, cle1, clde;
	int ntried = 0;
	int njump = 0;
	int dmcs;
	int ret;

	*update = FALSE;
	if (data->step != data->nevent) {
		fprintf(stderr, "Error : invalid step, step must be the last step (MP_KMCJump)\n");
		return -1;
	}
	for (j = 0, c = 0; j < data->nsolute; j++) {
		if (data->solute[j].jump) c++;
	}
	if (c < 1) {
		fprintf(stderr, "Error : no solute that can jump (MP_KMCJump)\n");
		return -1;
	}
	kt = data->kb*temp;
	dmcs = data->totmcs - data->mcs;
	while (ntried < ntry) {
		dp = (int)(MP_Rand(&(data->rand_seed)) * data->dpmax);
		cp = (int)(MP_Rand(&(data->rand_seed)) * (data->cpmax - 1)) + 1;
		if (dp < data->dpmax && cp < data->cpmax) {
			data->totmcs++, dmcs++;
			id0 = data->solute[dp].id;
			MP_KMCClusterIndexes(data, id0, ids0);
			id1 = ids0[cp];
			if (data->grid[id0].type != data->grid[id1].type) {
				MP_KMCClusterIndexes(data, id1, ids1);
				nncluster = UniteIndexes(data->ncluster, ids0, ids1, ids2);
				cle0 = GridClusterEnergy(data, nncluster, ids2);
				SwapType(data, id0, id1);
				cle1 = ClusterEnergy(data, func, nncluster, ids2, energy, update);
				clde = cle1 - cle0;
				if (clde < 0.0 || MP_Rand(&(data->rand_seed)) < exp(-clde / kt)) {
					for (j = 0; j < nncluster; j++) {
						data->grid[ids2[j]].energy = energy[j];
					}
					for (j = 0; j < data->nsolute; j++) {
						if (j != dp && data->solute[j].id == id1) break;
					}
					if (j < data->nsolute) {
						data->solute[j].id = id0;
						data->solute[j].njump++;
					}
					data->solute[dp].id = id1;
					data->solute[dp].njump++;
					data->tote += clde;
					if (data->event_record) {
						ret = KMCAddEvent(data, dp, id0, id1, clde, dmcs);
						if (ret < 0) return ret;
						dmcs = 0;
					}
					data->mcs = data->totmcs;
					njump++;
				}
				else {
					SwapType(data, id0, id1);
				}
			}
			ntried++;
		}
	}
	return njump;
}

static void UpdateGridEnergy(MP_KMCData *data, int id0, int id1)
{
	int j;
	int ids0[MP_KMC_NCLUSTER_MAX];
	int ids1[MP_KMC_NCLUSTER_MAX];
	int ids2[MP_KMC_NCLUSTER_MAX * 2];
	short types[MP_KMC_NCLUSTER_MAX];
	int nncluster;
	int tid;

	if (!data->table_use) return;
	MP_KMCClusterIndexes(data, id0, ids0);
	MP_KMCClusterIndexes(data, id1, ids1);
	nncluster = UniteIndexes(data->ncluster, ids0, ids1, ids2);
	for (j = 0; j < nncluster; j++) {
		if (data->grid[ids2[j]].type > 0) {
			MP_KMCClusterTypes(data, ids2[j], types);
			tid = MP_KMCSearchCluster(data, types);
			if (tid >= 0) {
				data->grid[ids2[j]].energy = data->table[tid].energy;
			}
			else {
				data->grid[ids2[j]].energy = 0.0;
			}
		}
		else if (data->grid[ids2[j]].type == 0) {
			data->grid[ids2[j]].energy = 0.0;
		}
	}
}

void MP_KMCStepForward(MP_KMCData *data, int count)
{
	int i;
	int dp;
	int id0, id1;

	for (i = 0; i < count; i++) {
		if (data->step >= data->nevent) return;
		dp = data->event[data->step].dp;
		id0 = data->event[data->step].id0;
		id1 = data->event[data->step].id1;
		SwapType(data, id0, id1);
		UpdateGridEnergy(data, id0, id1);
		data->solute[dp].id = id1;
		data->mcs += data->event[data->step].dmcs;
		data->tote += data->event[data->step].de;
		data->step++;
	}
}

void MP_KMCStepBackward(MP_KMCData *data, int count)
{
	int i;
	int dp;
	int id0, id1;

	for (i = 0; i < count; i++) {
		if (data->step <= 0) return;
		data->step--;
		dp = data->event[data->step].dp;
		id0 = data->event[data->step].id0;
		id1 = data->event[data->step].id1;
		SwapType(data, id0, id1);
		UpdateGridEnergy(data, id0, id1);
		data->solute[dp].id = id0;
		data->mcs -= data->event[data->step].dmcs;
		data->tote -= data->event[data->step].de;
	}
}

void MP_KMCStepGo(MP_KMCData *data, int step)
{
	int count;

	count = step - data->step;
	if (count > 0) MP_KMCStepForward(data, count);
	else if (count < 0) MP_KMCStepBackward(data, -count);
}

long MP_KMCStep2MCS(MP_KMCData *data, int step)
{
	int i;
	long st;

	st = data->mcs;
	for (i = data->step - 1; i >= 0; i--) {
		st -= data->event[i].dmcs;
		if (i == step) return st;
	}
	st = data->mcs;
	for (i = data->step; i <= data->nevent; i++) {
		if (i == step) return st;
		st += data->event[i].dmcs;
	}
	return st;
}

int MP_KMCMCS2Step(MP_KMCData *data, long mcs)
{
	int i;
	long st;

	st = data->mcs;
	for (i = data->step - 1; i >= 0; i--) {
		if (mcs >= st - data->event[i].dmcs && mcs < st) {
			return i;
		}
		st -= data->event[i].dmcs;
	}
	st = data->mcs;
	for (i = data->step; i <= data->nevent; i++) {
		if (mcs >= st && mcs < st + data->event[i].dmcs) {
			return i;
		}
		st += data->event[i].dmcs;
	}
	return data->nevent;
}

void MP_KMCMCSHistory(MP_KMCData *data, int num, double mcs[])
{
	int i;
	long st;

	st = data->mcs;
	for (i = data->step - 1; i >= 0; i--) {
		st -= data->event[i].dmcs;
		if (i < num) mcs[i] = st;
	}
	st = data->mcs;
	for (i = data->step; i <= data->nevent; i++) {
		if (i < num) mcs[i] = st;
		st += data->event[i].dmcs;
	}
}

void MP_KMCEnergyHistory(MP_KMCData *data, int num, double ene[])
{
	int i;
	double te;

	te = data->tote;
	for (i = data->step - 1; i >= 0; i--) {
		te -= data->event[i].de;
		if (i < num) ene[i] = te;
	}
	te = data->tote;
	for (i = data->step; i <= data->nevent; i++) {
		if (i < num) ene[i] = te;
		te += data->event[i].de;
	}
}

/*double MP_KMCSoluteSD(MP_KMCData *data, int sid, int step)
{
	int i;
	int count = 0;
	int id0, id1;
	int p0, x0, y0, z0;
	int p1, x1, y1, z1;
	int p2, x2, y2, z2;
	int ox, oy, oz;
	double dx, dy, dz;

	ox = 0, oy = 0, oz = 0;
	for (i = 0; i < step; i++) {
		if (data->event[i].dp == sid) {
			id0 = data->event[i].id0;
			id1 = data->event[i].id1;
			MP_KMCIndex2Grid(data, id0, &p0, &x0, &y0, &z0);
			MP_KMCIndex2Grid(data, id1, &p1, &x1, &y1, &z1);
			if (count == 0) {
				p2 = p0, x2 = x0, y2 = y0, z2 = z0;
			}
			if (x1 - x0 >= data->size[0] - 1) ox -= data->size[0];
			else if (x1 - x0 <= -data->size[0] + 1) ox += data->size[0];
			if (y1 - y0 >= data->size[1] - 1) oy -= data->size[1];
			else if (y1 - y0 <= -data->size[1] + 1) oy += data->size[1];
			if (z1 - z0 >= data->size[2] - 1) oz -= data->size[2];
			else if (z1 - z0 <= -data->size[2] + 1) oz += data->size[2];
			count++;
		}
	}
	if (count > 0) {
		dx = (x1 + data->uc[p1][0] + ox) - (x2 + data->uc[p2][0]);
		dy = (y1 + data->uc[p1][1] + oy) - (y2 + data->uc[p2][1]);
		dz = (z1 + data->uc[p1][2] + oz) - (z2 + data->uc[p2][2]);
		return dx * dx + dy * dy + dz * dz;
	}
	else return 0.0;
}

double MP_KMCSoluteMSD(MP_KMCData *data, int step)
{
	int i;
	double tsd = 0.0;

	for (i = 0; i < data->nsolute; i++) {
		tsd += MP_KMCSoluteSD(data, i, step);
	}
	return tsd / data->nsolute;
}

double MP_KMCSoluteTypeMSD(MP_KMCData *data, short type, int step)
{
	int i;
	int nsolute = 0;
	double tsd = 0.0;

	for (i = 0; i < data->nsolute; i++) {
		if (data->solute[i].type == type) {
			tsd += MP_KMCSoluteSD(data, i, step);
			nsolute++;
		}
	}
	return tsd / nsolute;
}*/

