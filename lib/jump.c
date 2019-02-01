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

static double ClusterEnergy(MP_KMCData *data, double(*func)(MP_KMCData *, short *),
	int nncluster, int ids[], double energy[], int *update)
{
	int j, k;
	int tid;
	double cle;
	int eids[MP_KMC_NCLUSTER_MAX];
	short types[MP_KMC_NCLUSTER_MAX];

	for (j = 0; j < nncluster; j++) {
		if (data->grid[ids[j]].type > 0) {
			MP_KMCClusterIndexes(data, ids[j], eids);
			for (k = 0; k < data->ncluster; k++) {
				types[k] = data->grid[eids[k]].type;
			}
			if (data->table_use) {
				tid = MP_KMCSearchCluster(data, types);
				if (tid >= 0) {
					data->table[tid].refcount += 1;
					energy[j] = data->table[tid].energy;
				}
				else {
					if (func != NULL) {
						energy[j] = (func)(data, types);
						MP_KMCAddCluster(data, types, energy[j], 0);
						*update = TRUE;
					}
					else {
						energy[j] = 0.0;
					}
				}
			}
			else {
				energy[j] = (func)(data, types);
			}
		}
		else if (data->grid[ids[j]].type == 0) {
			energy[j] = 0.0;
		}
	}
	cle = 0.0;
	for (j = 0; j < nncluster; j++) {
		cle += energy[j];
	}
	return cle;
}

int KMCAddResult(MP_KMCData *data, long totmcs, double temp, int ntry, int njump, double fjump, double tote)
{
	int nresult_max;
	int tid;

	if (data->nresult >= data->nresult_max) {
		nresult_max = data->nresult_max + data->nresult_step;
		data->result = (MP_KMCResultItem *)realloc(data->result, nresult_max*sizeof(MP_KMCResultItem));
		if (data->result == NULL) {
			fprintf(stderr, "Error : allocation failure (KMCAddResult)\n");
			return  MP_KMC_MEM_ERR;
		}
		data->nresult_max = nresult_max;
	}
	tid = data->nresult;
	data->result[tid].totmcs = totmcs;
	data->result[tid].temp = temp;
	data->result[tid].ntry = ntry;
	data->result[tid].njump = njump;
	data->result[tid].fjump = fjump;
	data->result[tid].tote = tote;
	data->nresult++;
	return tid;
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

	*update = FALSE;
	if (data->step != data->nevent) {
		fprintf(stderr, "Error : invalid step, step must be the last step (MP_KMCJump)\n");
		return -1;
	}
	for (j = 0, c = 0; j < data->nsolute; j++) {
		if (data->solute[j].jump) c++;
	}
	if (c < 1) return -1;
	for (j = 0, c = 0; j < data->ncluster; j++) {
		if (data->jcluster[j]) c++;
	}
	if (c < 2) return -1;
	kt = data->kb*temp;
	dmcs = data->totmcs - data->mcs;
	while (ntried < ntry) {
		dp = (int)(MP_Rand(&(data->rand_seed)) * data->nsolute);
		cp = (int)(MP_Rand(&(data->rand_seed)) * (data->ncluster - 1)) + 1;
		if (dp < data->nsolute && cp < data->ncluster && data->solute[dp].jump && data->jcluster[cp]) {
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
					if (j < data->nsolute) data->solute[j].id = id0;
					data->solute[dp].id = id1;
					data->solute[dp].njump++;
					data->tote += clde;
					if (KMCAddEvent(data, dp, id0, id1, clde, dmcs) < 0) return njump;
					dmcs = 0;
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
	KMCAddResult(data, data->totmcs, temp, ntry, njump, (double)njump/ntry, data->tote);
	return njump;
}

static void UpdateGridEnergy(MP_KMCData *data, int id0, int id1)
{
	int j, k;
	int ids0[MP_KMC_NCLUSTER_MAX];
	int ids1[MP_KMC_NCLUSTER_MAX];
	int ids2[MP_KMC_NCLUSTER_MAX * 2];
	int ids3[MP_KMC_NCLUSTER_MAX];
	short types3[MP_KMC_NCLUSTER_MAX];
	int nncluster;
	int tid;

	if (!data->table_use) return;
	MP_KMCClusterIndexes(data, id0, ids0);
	MP_KMCClusterIndexes(data, id1, ids1);
	nncluster = UniteIndexes(data->ncluster, ids0, ids1, ids2);
	for (j = 0; j < nncluster; j++) {
		if (data->grid[ids2[j]].type > 0) {
			MP_KMCClusterIndexes(data, ids2[j], ids3);
			for (k = 0; k < data->ncluster; k++) {
				types3[k] = data->grid[ids3[k]].type;
			}
			tid = MP_KMCSearchCluster(data, types3);
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

void MP_KMCMCSHistory(MP_KMCData *data, int num, double mcs[])
{
	int i;
	long st = 0;

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


