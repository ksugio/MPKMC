#include "MPKMC.h"

int KMCUniteIndexes(int ncluster, int ids0[], int ids1[], int ids2[])
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

void KMCSwapType(MP_KMCData *data, int id0, int id1)
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
	int ncluster, int ids[], double energy[], int *table_update)
{
	int j, k;
	int ttid;
	int tid[MP_KMC_NCLUSTER_MAX * 2];
	short types[MP_KMC_NCLUSTER_MAX * 2][MP_KMC_NCLUSTER_MAX];

	if (data->table_use) {
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for (j = 0; j < ncluster; j++) {
			if (data->grid[ids[j]].type > 0) {
				MP_KMCClusterTypes(data, ids[j], types[j]);
				tid[j] = MP_KMCSearchCluster(data, types[j]);
				if (tid[j] >= 0) {
					energy[j] = data->table[tid[j]].energy;
				}
				else {
					if (func != NULL) energy[j] = (func)(data, types[j]);
					else energy[j] = 0.0;
				}
			}
			else if (data->grid[ids[j]].type == 0) {
				tid[j] = -99;
				energy[j] = 0.0;
			}
		}
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
				*table_update = TRUE;
			}
		}
	}
	else {
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for (j = 0; j < ncluster; j++) {
			if (data->grid[ids[j]].type > 0) {
				MP_KMCClusterTypes(data, ids[j], types[j]);
				if (func != NULL) energy[j] = (func)(data, types[j]);
				else energy[j] = 0.0;
			}
			else if (data->grid[ids[j]].type == 0) {
				energy[j] = 0.0;
			}
		}
	}
}

static double ClusterEnergy(MP_KMCData *data, double(*func)(MP_KMCData *, short *),
	int ncluster, int ids[], double energy[], int *table_update)
{
	int j;
	double cle = 0.0;

	CalcClusterEnergies(data, func, ncluster, ids, energy, table_update);
	for (j = 0; j < ncluster; j++) {
		cle += energy[j];
	}
	return cle;
}

int KMCAddEvent(MP_KMCData *data, int dp, int dpp, int id0, int id1, double de, int dmcs)
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
	data->event[eid].dpp = dpp;
	data->event[eid].id0 = id0;
	data->event[eid].id1 = id1;
	data->event[eid].de = de;
	data->event[eid].dmcs = dmcs;
	data->nevent++;
	data->event_pt = data->nevent;
	return eid;
}

int KMCAddHistory(MP_KMCData *data, long totmcs, double temp, int ntry, int njump, int table_update, int ntable, double tote, double time)
{
	int nhistory_max;
	int hid;

	if (data->nhistory >= data->nhistory_max) {
		nhistory_max = data->nhistory_max + data->nhistory_step;
		data->history = (MP_KMCHistoryItem *)realloc(data->history, nhistory_max * sizeof(MP_KMCHistoryItem));
		if (data->history == NULL) {
			fprintf(stderr, "Error : allocation failure (KMCAddHistory)\n");
			return  MP_KMC_MEM_ERR;
		}
		data->nhistory_max = nhistory_max;
	}
	hid = data->nhistory;
	data->history[hid].totmcs = totmcs;
	data->history[hid].temp = temp;
	data->history[hid].ntry = ntry;
	data->history[hid].njump = njump;
	data->history[hid].table_update = table_update;
	data->history[hid].ntable = ntable;
	data->history[hid].tote = tote;
	data->history[hid].time = time;
	data->nhistory++;
	return hid;
}

int MP_KMCGridEnergy(MP_KMCData *data, double(*func)(MP_KMCData *, short *))
{
	int j;
	int id = 0;
	int ncluster = 0;
	int step = MP_KMC_NCLUSTER_MAX;
	int ids[MP_KMC_NCLUSTER_MAX];
	double energy[MP_KMC_NCLUSTER_MAX];
	int table_update = FALSE;

	while (TRUE) {
		ids[ncluster++] = id++;
		if (ncluster >= step || id >= data->ntot) {
			CalcClusterEnergies(data, func, ncluster, ids, energy, &table_update);
			for (j = 0; j < ncluster; j++) {
				data->grid[ids[j]].energy = energy[j];
				data->tote += energy[j];
			}
			ncluster = 0;
			if (id >= data->ntot) break;
		}
	}
	return table_update;
}

MP_KMCHistoryItem MP_KMCJump(MP_KMCData *data, int ntry, double temp, double(*func)(MP_KMCData *, short *))
{
	int j, c;
	int dp, jp, dpp;
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
	int table_update = FALSE;
	int dmcs;
	int hid;
	MP_KMCHistoryItem err = { 0, 0.0, 0, 0, 0, 0, 0.0, 0.0 };
	clock_t start = clock();

	if (data->event_pt != data->nevent) {
		fprintf(stderr, "Error : invalid current event point (MP_KMCJump)\n");
		return err;
	}
	for (j = 0, c = 0; j < data->nsolute; j++) {
		if (data->solute[j].jump) c++;
	}
	if (c < 1) {
		fprintf(stderr, "Error : no solute that can jump (MP_KMCJump)\n");
		return err;
	}
	kt = data->kb*temp;
	dmcs = data->totmcs - data->mcs;
	while (ntried < ntry) {
		dp = (int)(MP_Rand(&(data->rand_seed)) * data->dpmax);
		jp = (int)(MP_Rand(&(data->rand_seed)) * (data->jpmax - 1)) + 1;
		if (dp < data->dpmax && jp < data->jpmax) {
			data->totmcs++, dmcs++;
			id0 = data->solute[dp].id;
			MP_KMCClusterIndexes(data, id0, ids0);
			id1 = ids0[jp];
			if (data->grid[id0].type != data->grid[id1].type) {
				MP_KMCClusterIndexes(data, id1, ids1);
				nncluster = KMCUniteIndexes(data->ncluster, ids0, ids1, ids2);
				cle0 = GridClusterEnergy(data, nncluster, ids2);
				KMCSwapType(data, id0, id1);
				cle1 = ClusterEnergy(data, func, nncluster, ids2, energy, &table_update);
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
						dpp = j;
					}
					else dpp = -1;
					data->solute[dp].id = id1;
					data->solute[dp].njump++;
					data->tote += clde;
					if (data->event_record) {
						if (KMCAddEvent(data, dp, dpp, id0, id1, clde, dmcs) == MP_KMC_MEM_ERR) return err;
						dmcs = 0;
					}
					data->mcs = data->totmcs;
					njump++;
				}
				else {
					KMCSwapType(data, id0, id1);
				}
			}
			ntried++;
		}
	}
	hid = KMCAddHistory(data, data->totmcs, temp, ntry, njump, table_update, data->ntable, data->tote, 
		(double)(clock() - start) / CLOCKS_PER_SEC);
	if (hid == MP_KMC_MEM_ERR) return err;
	else return data->history[hid];
}
