#include "MPKMC.h"

int KMCUniteIndexes(int ncluster, int ids0[], int ids1[], int ids2[]);
void KMCSwapType(MP_KMCData *data, int id0, int id1);

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
	nncluster = KMCUniteIndexes(data->ncluster, ids0, ids1, ids2);
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

void MP_KMCEventForward(MP_KMCData *data, int count)
{
	int i;
	int dp;
	int id0, id1;

	for (i = 0; i < count; i++) {
		if (data->event_pt >= data->nevent) return;
		dp = data->event[data->event_pt].dp;
		id0 = data->event[data->event_pt].id0;
		id1 = data->event[data->event_pt].id1;
		KMCSwapType(data, id0, id1);
		UpdateGridEnergy(data, id0, id1);
		data->solute[dp].id = id1;
		data->mcs += data->event[data->event_pt].dmcs;
		data->tote += data->event[data->event_pt].de;
		data->event_pt++;
	}
}

void MP_KMCEventBackward(MP_KMCData *data, int count)
{
	int i;
	int dp;
	int id0, id1;

	for (i = 0; i < count; i++) {
		if (data->event_pt <= 0) return;
		data->event_pt--;
		dp = data->event[data->event_pt].dp;
		id0 = data->event[data->event_pt].id0;
		id1 = data->event[data->event_pt].id1;
		KMCSwapType(data, id0, id1);
		UpdateGridEnergy(data, id0, id1);
		data->solute[dp].id = id0;
		data->mcs -= data->event[data->event_pt].dmcs;
		data->tote -= data->event[data->event_pt].de;
	}
}

void MP_KMCEventGo(MP_KMCData *data, int event_pt)
{
	int count;

	count = event_pt - data->event_pt;
	if (count > 0) MP_KMCEventForward(data, count);
	else if (count < 0) MP_KMCEventBackward(data, -count);
}

long MP_KMCEventPt2MCS(MP_KMCData *data, int event_pt)
{
	int i;
	long st;

	st = data->mcs;
	for (i = data->event_pt - 1; i >= 0; i--) {
		st -= data->event[i].dmcs;
		if (i == event_pt) return st;
	}
	st = data->mcs;
	for (i = data->event_pt; i <= data->nevent; i++) {
		if (i == event_pt) return st;
		st += data->event[i].dmcs;
	}
	return st;
}

int MP_KMCMCS2EventPt(MP_KMCData *data, long mcs)
{
	int i;
	long st;

	st = data->mcs;
	for (i = data->event_pt - 1; i >= 0; i--) {
		if (mcs >= st - data->event[i].dmcs && mcs < st) {
			return i;
		}
		st -= data->event[i].dmcs;
	}
	st = data->mcs;
	for (i = data->event_pt; i <= data->nevent; i++) {
		if (mcs >= st && mcs < st + data->event[i].dmcs) {
			return i;
		}
		st += data->event[i].dmcs;
	}
	return data->nevent;
}

void MP_KMCEventMCS(MP_KMCData *data, int num, double mcs[])
{
	int i;
	long st;

	st = data->mcs;
	for (i = data->event_pt - 1; i >= 0; i--) {
		st -= data->event[i].dmcs;
		if (i < num) mcs[i] = st;
	}
	st = data->mcs;
	for (i = data->event_pt; i <= data->nevent; i++) {
		if (i < num) mcs[i] = st;
		st += data->event[i].dmcs;
	}
}

void MP_KMCEventEnergy(MP_KMCData *data, int num, double ene[])
{
	int i;
	double te;

	te = data->tote;
	for (i = data->event_pt - 1; i >= 0; i--) {
		te -= data->event[i].de;
		if (i < num) ene[i] = te;
	}
	te = data->tote;
	for (i = data->event_pt; i <= data->nevent; i++) {
		if (i < num) ene[i] = te;
		te += data->event[i].de;
	}
}

static double EventSD(MP_KMCData *data, int sid, int event_pt)
{
	int i;
	int count = 0;
	int p0, x0, y0, z0;
	int p1, x1, y1, z1;
	int p2, x2, y2, z2;
	int ox, oy, oz;
	double dx, dy, dz;

	ox = 0, oy = 0, oz = 0;
	for (i = 0; i < event_pt; i++) {
		if (data->event[i].dp == sid || data->event[i].dpp == sid) {
			if (data->event[i].dp == sid) {
				MP_KMCIndex2Grid(data, data->event[i].id0, &p0, &x0, &y0, &z0);
				MP_KMCIndex2Grid(data, data->event[i].id1, &p1, &x1, &y1, &z1);
			}
			else if (data->event[i].dpp == sid) {
				MP_KMCIndex2Grid(data, data->event[i].id1, &p0, &x0, &y0, &z0);
				MP_KMCIndex2Grid(data, data->event[i].id0, &p1, &x1, &y1, &z1);
			}
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

double MP_KMCEventMSD(MP_KMCData *data, short type, int event_pt)
{
	int i;
	int nsolute = 0;
	double tsd = 0.0;

	for (i = 0; i < data->nsolute; i++) {
		if (data->solute[i].type == type) {
			tsd += EventSD(data, i, event_pt);
			nsolute++;
		}
	}
	return tsd / nsolute;
}
