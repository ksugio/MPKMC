#include "MPKMC.h"

int MP_KMCAlloc(MP_KMCData *data, int nuc, int nx, int ny, int nz, int ncluster,
	int nsolute_max, int ntable_step, int nevent_step)
{
	int i;
	double init_pv[][3] = {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};

	if (nuc > MP_KMC_NUC_MAX) return FALSE;
	if (ncluster > MP_KMC_NCLUSTER_MAX) return FALSE;
	data->nuc = nuc;
	data->size[0] = nx, data->size[1] = ny, data->size[2] = nz;
	data->ntot = nuc * nx * ny * nz;
	data->ncluster = ncluster;
	data->grid = (MP_KMCGridItem *)malloc(data->ntot*sizeof(MP_KMCGridItem));
	data->table = (MP_KMCTableItem *)malloc(ntable_step*sizeof(MP_KMCTableItem));
	data->table_types = (short *)malloc(ncluster*ntable_step*sizeof(short));
	data->clusterid = (int *)malloc(nuc*ncluster*4*sizeof(int));
	data->rotid = (int *)malloc(MP_KMC_NROT_MAX*ncluster*sizeof(int));
	data->solute = (MP_KMCSoluteItem *)malloc(nsolute_max*sizeof(MP_KMCSoluteItem));
	data->event = (MP_KMCEventItem *)malloc(nevent_step*sizeof(MP_KMCEventItem));
	if (data->grid == NULL || data->table == NULL || data->table_types == NULL
		|| data->clusterid == NULL || data->rotid == NULL || data->solute == NULL
		|| data->event == NULL) return FALSE;
	for (i = 0; i < nuc; i++) {
		data->uc[i][0] = 0.0, data->uc[i][1] = 0.0, data->uc[i][2] = 0.0;
		data->uc_types[i] = -1;
	}
	for (i = 0; i < 3; i++) {
		data->pv[i][0] = init_pv[i][0], data->pv[i][1] = init_pv[i][1], data->pv[i][2] = init_pv[i][2];
	}
	for (i = 0; i < data->ntot; i++) {
		data->grid[i].type = 0;
		data->grid[i].energy = 0.0;
	}
	for (i = 0; i < ncluster; i++) {
		data->cluster[i][0] = 0.0, data->cluster[i][1] = 0.0, data->cluster[i][2] = 0.0;
		data->jcluster[i] = FALSE;
	}
	for (i = 0; i < ntable_step; i++) {
		data->table[i].types = data->table_types + i*ncluster;
	}
	data->nrot = 0;
	data->table_use = TRUE;
	data->ntable = 0;
	data->ntable_max = data->ntable_step = ntable_step;
	data->htable[0] = '\0';
	data->nsolute = 0;
	data->nsolute_max = nsolute_max;
	data->nevent = 0;
	data->nevent_max = data->nevent_step = nevent_step;
	data->rand_seed = 12061969;
	data->step = 0;
	data->tote = 0.0;
	return TRUE;
}

void MP_KMCFree(MP_KMCData *data)
{
	free(data->grid);
	free(data->table);
	free(data->table_types);
	free(data->clusterid);
	free(data->rotid);
	free(data->solute);
	free(data->event);
}

void MP_KMCSetUnitCell(MP_KMCData *data, double uc[][3], short types[], double pv[][3])
{
	int p, x, y, z;
	int id;

	for (p = 0; p < data->nuc; p++) {
		data->uc[p][0] = uc[p][0];
		data->uc[p][1] = uc[p][1];
		data->uc[p][2] = uc[p][2];
		data->uc_types[p] = types[p];
	}
	for (z = 0; z < data->size[2]; z++) {
		for (y = 0; y < data->size[1]; y++) {
			for (x = 0; x < data->size[0]; x++) {
				for (p = 0; p < data->nuc; p++) {
					id = MP_KMCGrid2Index(data, p, x, y, z);
					data->grid[id].type = types[p];
				}
			}
		}
	}
	for (p = 0; p < 3; p++) {
		data->pv[p][0] = pv[p][0];
		data->pv[p][1] = pv[p][1];
		data->pv[p][2] = pv[p][2];
	}
}

static int SearchClusterIndex(MP_KMCData *data, int p, double cluster[], int *np, int *dx, int *dy, int *dz)
{
	int i, j;
	double x, y, z;
	int neigh[27][3] = {
		{ 0, 0, 0 },{ 1, 0, 0 },{ -1, 0, 0 },{ 0, 1, 0 },{ 0, -1, 0 },{ 1, 1, 0 },
		{ 1, -1, 0 },{ -1, 1, 0 },{ -1, -1, 0 },{ 0, 0, 1 },{ 1, 0, 1 },{ -1, 0, 1 },
		{ 0, 1, 1 },{ 0, -1, 1 },{ 1, 1, 1 },{ 1, -1, 1 },{ -1, 1, 1 },{ -1, -1, 1 },
		{ 0, 0, -1 },{ 1, 0, -1 },{ -1, 0, -1 },{ 0, 1, -1 },{ 0, -1, -1 },{ 1, 1, -1 },
		{ 1, -1, -1 },{ -1, 1, -1 },{ -1, -1, -1 } };

	for (i = 0; i < 27; i++) {
		for (j = 0; j < data->nuc; j++) {
			x = neigh[i][0] + data->uc[j][0] - data->uc[p][0];
			y = neigh[i][1] + data->uc[j][1] - data->uc[p][1];
			z = neigh[i][2] + data->uc[j][2] - data->uc[p][2];
			if (x == cluster[0] && y == cluster[1] && z == cluster[2]) {
				*np = j;
				*dx = neigh[i][0];
				*dy = neigh[i][1];
				*dz = neigh[i][2];
				return TRUE;
			}
		}
	}
	return FALSE;
}

int MP_KMCSetCluster(MP_KMCData *data, double cluster[][3], short jcluster[])
{
	int i, j;
	int np, dx, dy, dz;
	int pt;
	double rp[3];

	for (i = 0; i < data->ncluster; i++) {
		data->cluster[i][0] = cluster[i][0];
		data->cluster[i][1] = cluster[i][1];
		data->cluster[i][2] = cluster[i][2];
		data->jcluster[i] = jcluster[i];
		MP_KMCRealPos(data, cluster[i], rp);
		data->rcluster[i][0] = rp[0];
		data->rcluster[i][1] = rp[1];
		data->rcluster[i][2] = rp[2];
	}
	for (i = 0; i < data->nuc; i++) {
		for (j = 0; j < data->ncluster; j++) {
			if (SearchClusterIndex(data, i, cluster[j], &np, &dx, &dy, &dz)) {
				pt = i * (data->ncluster * 4) + j * 4;
				data->clusterid[pt++] = np;
				data->clusterid[pt++] = dx;
				data->clusterid[pt++] = dy;
				data->clusterid[pt] = dz;
			}
			else {
				fprintf(stderr, "Error : Cluster position not match to Unitcell position.\n");
				return FALSE;
			}
		}
	}
	return TRUE;
}

void MP_KMCRealPos(MP_KMCData *data, double pos[], double rpos[])
{
	rpos[0] = data->pv[0][0] * pos[0] + data->pv[1][0] * pos[1] + data->pv[2][0] * pos[2];
	rpos[1] = data->pv[0][1] * pos[0] + data->pv[1][1] * pos[1] + data->pv[2][1] * pos[2];
	rpos[2] = data->pv[0][2] * pos[0] + data->pv[1][2] * pos[1] + data->pv[2][2] * pos[2];
}

void MP_KMCIndex2Grid(MP_KMCData *data, int id, int *p, int *x, int *y, int *z)
{
	int a, b, c, d;

	a = data->nuc * data->size[0] * data->size[1];
	*z = id / a;
	b = id - *z * a;
	c = data->nuc * data->size[0];
	*y = b / c;
	d = b - *y * c;
	*x = d / data->nuc;
	*p = d - *x * data->nuc;
}

int MP_KMCGrid2Index(MP_KMCData *data, int p, int x, int y, int z)
{
	return p + x*data->nuc + y*(data->nuc*data->size[0]) + z*(data->nuc*data->size[0]*data->size[1]);
}

void MP_KMCIndex2Pos(MP_KMCData *data, int id, double pos[])
{
	int p, x, y, z;

	MP_KMCIndex2Grid(data, id, &p, &x, &y, &z);
	pos[0] = data->uc[p][0] + x;
	pos[1] = data->uc[p][1] + y;
	pos[2] = data->uc[p][2] + z;
}

void MP_KMCClusterIndexes(MP_KMCData *data, int id, int ids[])
{
	int i;
	int p, x, y, z;
	int np, nx, ny, nz;
	int pt;

	MP_KMCIndex2Grid(data, id, &p, &x, &y, &z);
	for (i = 0; i < data->ncluster; i++) {
		pt = p * (data->ncluster * 4) + i * 4;
		np = data->clusterid[pt++];
		nx = x + data->clusterid[pt++];
		ny = y + data->clusterid[pt++];
		nz = z + data->clusterid[pt];
		if (nx < 0) {
			nx += data->size[0];
		}
		else if (nx >= data->size[0]) {
			nx -= data->size[0];
		}
		if (ny < 0) {
			ny += data->size[1];
		}
		else if (ny >= data->size[1]) {
			ny -= data->size[1];
		}
		if (nz < 0) {
			nz += data->size[2];
		}
		else if (nz >= data->size[2]) {
			nz -= data->size[2];
		}
		ids[i] = MP_KMCGrid2Index(data, np, nx, ny, nz);
	}
}

int MP_KMCSearchCluster(MP_KMCData *data, short types[])
{
	int i, j, k;
	int count;
	int sp;

	for (i = 0; i < data->ntable; i++) {
		if (data->table[i].types[0] == types[0]) {
			for (j = 0; j < data->nrot; j++) {
				count = 0;
				sp = j * data->ncluster;
				for (k = 1; k < data->ncluster; k++) {
					if (data->table[i].types[k] == types[data->rotid[sp+k]]) count++;
				}
				if (count == data->ncluster - 1) return i;
			}
		}
	}
	return -1;
}

int MP_KMCSearchClusterIDs(MP_KMCData *data, int ids[])
{
	int i;
	short types[MP_KMC_NCLUSTER_MAX];

	for (i = 0; i < data->ncluster; i++) {
		types[i] = data->grid[ids[i]].type;
	}
	return MP_KMCSearchCluster(data, types);
}

int MP_KMCAddCluster(MP_KMCData *data, short types[], double energy, long refcount)
{
	int i;
	int ntable_max;
	int tid;

	if (data->ntable >= data->ntable_max) {
		ntable_max = data->ntable_max + data->ntable_step;
		data->table = (MP_KMCTableItem *)realloc(data->table, ntable_max*sizeof(MP_KMCTableItem));
		data->table_types = (short *)realloc(data->table_types, data->ncluster*ntable_max*sizeof(short));
		if (data->table == NULL || data->table_types == NULL) {
			fprintf(stderr, "Error : allocation failure (MP_KMCAddCluster)\n");
			return -99;
		}
		for (i = 0; i < ntable_max; i++) {
			data->table[i].types = data->table_types + i*data->ncluster;
		}
		data->ntable_max = ntable_max;
	}
	tid = data->ntable;
	for (i = 0; i < data->ncluster; i++) {
		data->table[tid].types[i] = types[i];
	}
	data->table[tid].energy = energy;
	data->table[tid].refcount = refcount;
	data->ntable++;
	return tid;
}

int MP_KMCAddClusterIDs(MP_KMCData *data, int ids[], double energy, long refcount)
{
	int i;
	short types[MP_KMC_NCLUSTER_MAX];

	for (i = 0; i < data->ncluster; i++) {
		types[i] = data->grid[ids[i]].type;
	}
	return MP_KMCAddCluster(data, types, energy, refcount);
}

int MP_KMCAddSolute(MP_KMCData *data, int id, short type, short jump)
{
	int i;

	if (id < 0 || id >= data->ntot) {
		fprintf(stderr, "Error : invalid index %d.\n", id);
		return FALSE;
	}
	if (data->nsolute >= data->nsolute_max) {
		fprintf(stderr, "Error : maximum solute atoms is %d.\n", data->nsolute_max);
		return FALSE;
	}
	for (i = 0; i < data->nsolute; i++) {
		if (data->solute[i].id == id) return FALSE;
	}
	data->grid[id].type = type;
	data->solute[data->nsolute].id = id;
	data->solute[data->nsolute].type = type;
	data->solute[data->nsolute].jump = jump;
	data->solute[data->nsolute].njump = 0;
	data->nsolute++;
	return TRUE;
}

void MP_KMCAddSoluteRandom(MP_KMCData *data, int num, short type, short jump)
{
	int count = 0;
	int id;

	while (count < num) {
		id = (int)(MP_Rand(&(data->rand_seed)) * data->ntot);
		if (MP_KMCAddSolute(data, id, type, jump)) count++;
	}
}

double MP_KMCCalcEnergy(MP_KMCData *data, int id, double(*func)(MP_KMCData *, short *), int *update)
{
	int i;
	int ids[MP_KMC_NCLUSTER_MAX];
	short types[MP_KMC_NCLUSTER_MAX];
	int tid;

	*update = FALSE;
	if (data->grid[id].type > 0) {
		MP_KMCClusterIndexes(data, id, ids);
		for (i = 0; i < data->ncluster; i++) {
			types[i] = data->grid[ids[i]].type;
		}
		if (data->table_use) {
			tid = MP_KMCSearchCluster(data, types);
			if (tid >= 0) {
				data->table[tid].refcount += 1;
				data->grid[id].energy = data->table[tid].energy;
			}
			else {
				if (func != NULL) {
					data->grid[id].energy = (func)(data, types);
					MP_KMCAddCluster(data, types, data->grid[id].energy, 0);
					*update = TRUE;
				}
				else {
					data->grid[id].energy = 0.0;
				}
			}
		}
		else {
			data->grid[id].energy = (func)(data, types);
		}
	}
	else if (data->grid[id].type == 0) {
		data->grid[id].energy = 0.0;
	}
	return data->grid[id].energy;
}

double MP_KMCTotalEnergy(MP_KMCData *data, double(*func)(MP_KMCData *, short *), int *update)
{
	int i;
	int tupdate;

	*update = FALSE;
	data->tote = 0.0;
	for (i = 0; i < data->ntot; i++) {
		data->tote += MP_KMCCalcEnergy(data, i, func, &tupdate);
		if (tupdate) *update = TRUE;
	}
	return data->tote;
}

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

static void AddEvent(MP_KMCData *data, int dp, int id0, int id1, double de)
{
	int nevent_max;
	int eid;

	if (data->nevent >= data->nevent_max) {
		nevent_max = data->nevent_max + data->nevent_step;
		data->event = (MP_KMCEventItem *)realloc(data->event, nevent_max*sizeof(MP_KMCEventItem));
		if (data->event == NULL) {
			fprintf(stderr, "Error : allocation failure (AddEvent)\n");
			return;
		}
		data->nevent_max = nevent_max;
	}
	eid = data->nevent;
	data->event[eid].dp = dp;
	data->event[eid].id0 = id0;
	data->event[eid].id1 = id1;
	data->event[eid].de = de;
	data->nevent++;
}

int MP_KMCJump(MP_KMCData *data, int ntry, double kt, double(*func)(MP_KMCData *, short *), int *update)
{
	int j, c;
	int dp, cp;
	int id0, id1;
	int ids0[MP_KMC_NCLUSTER_MAX];
	int ids1[MP_KMC_NCLUSTER_MAX];
	int ids2[MP_KMC_NCLUSTER_MAX * 2];
	int nncluster;
	double energy[MP_KMC_NCLUSTER_MAX * 2];
	double cle0, cle1, clde;
	int ntried = 0;
	int njump = 0;

	*update = FALSE;
	for (j = 0, c = 0; j < data->nsolute; j++) {
		if (data->solute[j].jump) c++;
	}
	if (c < 1) return -1;
	for (j = 0, c = 0; j < data->ncluster; j++) {
		if (data->jcluster[j]) c++;
	}
	if (c < 2) return -1;
	while (ntried < ntry) {
		dp = (int)(MP_Rand(&(data->rand_seed)) * data->nsolute);
		cp = (int)(MP_Rand(&(data->rand_seed)) * (data->ncluster - 1)) + 1;
		if (dp < data->nsolute && cp < data->ncluster && data->solute[dp].jump && data->jcluster[cp]) {
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
					data->solute[dp].id = id1;
					data->solute[dp].njump++;
					AddEvent(data, dp, id0, id1, clde);
					data->tote += clde;
					data->step++;
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

void MP_KMCEnergyHistory(MP_KMCData *data, int nhist, double ehist[])
{
	int i;
	double te;

	te = data->tote;
	for (i = data->step - 1; i >= 0; i--) {
		te -= data->event[i].de;
		if (i < nhist) ehist[i] = te;
	}
	te = data->tote;
	for (i = data->step; i <= data->nevent; i++) {
		if (i < nhist) ehist[i] = te;
		te += data->event[i].de;
	}
}

int MP_KMCWriteTable(MP_KMCData *data, char *filename)
{
	int i, j;
	FILE *fp;

	if ((fp = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCWriteTable)\n", filename);
		return FALSE;
	}
	fprintf(fp, "%s\n", data->htable);
	fprintf(fp, "ncluster %d\n", data->ncluster);
	for (i = 0; i < data->ncluster; i++) {
		fprintf(fp, "%.15e %.15e %.15e\n", data->cluster[i][0], data->cluster[i][1], data->cluster[i][2]);
	}
	fprintf(fp, "ntable %d\n", data->ntable);
	for (i = 0; i < data->ntable; i++) {
		for (j = 0; j < data->ncluster; j++) {
			fprintf(fp, "%d ", data->table[i].types[j]);
		}
		fprintf(fp, "%.15e %d\n", data->table[i].energy, data->table[i].refcount);
	}
	fclose(fp);
	return TRUE;
}

static void ScanTable(char buf[], int ncluster, short types[], double *energy, long *refcount)
{
	int i;
	char *tok;

	tok = strtok(buf, " ");
	for (i = 0; i < ncluster; i++) {
		types[i] = atoi(tok);
		tok = strtok(NULL, " ");
	}
	*energy = atof(tok);
	tok = strtok(NULL, " ");
	*refcount = atol(tok);
}

int MP_KMCReadTable(MP_KMCData *data, char *filename)
{
	FILE *fp;
	int ncluster, ntable;
	double cluster[3];
	short types[MP_KMC_NCLUSTER_MAX];
	double energy;
	long refcount;
	char *p;
	char buf[256];
	int i;

	if ((fp = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCReadTable)\n", filename);
		return FALSE;
	}
	fgets(data->htable, 256, fp);
	p = strchr(data->htable, '\n');
	if (p != NULL) *p = '\0';
	fscanf(fp, "%s %d", buf, &ncluster);
	if (ncluster != data->ncluster) {
		fprintf(stderr, "Error : incompatible number of cluster, %s.(MP_KMCReadTable)\n", filename);
		return FALSE;
	}
	for (i = 0; i < ncluster; i++) {
		fscanf(fp, "%le %le %le", &(cluster[0]), &(cluster[1]), &(cluster[2]));
		if (cluster[0] != data->cluster[i][0] || cluster[1] != data->cluster[i][1]
			|| cluster[2] != data->cluster[i][2]) {
			fprintf(stderr, "Error : incompatible position of cluster, %s.(MP_KMCReadTable)\n", filename);
			return FALSE;
		}
	}
	fscanf(fp, "%s %d\n", buf, &ntable);
	for (i = 0; i < ntable;i++) {
		fgets(buf, 256, fp);
		ScanTable(buf, ncluster, types, &energy, &refcount);
		MP_KMCAddCluster(data, types, energy, refcount);
	}
	return TRUE;
}

static int SortComp(const void *c1, const void *c2)
{
	MP_KMCTableItem *t1 = (MP_KMCTableItem *)c1;
	MP_KMCTableItem *t2 = (MP_KMCTableItem *)c2;

	if (t1->refcount < t2->refcount) return 1;
	else if (t1->refcount == t2->refcount) return 0;
	else return -1;
}

void MP_KMCSortTable(MP_KMCData *data)
{
	qsort(data->table, data->ntable, sizeof(MP_KMCTableItem), SortComp);
}

void MP_KMCResetTable(MP_KMCData *data)
{
	int i;

	for (i = 0; i < data->ntable; i++) {
		data->table[i].refcount = 0;
	}
}

int MP_KMCWrite(MP_KMCData *data, char *filename, int comp)
{
	int i, j;
	int sp;
	gzFile gfp;
	char mode[32];

	sprintf(mode, "wb%df", comp);
	if ((gfp = gzopen(filename, mode)) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCWrite)\n", filename);
		return FALSE;
	}
	gzprintf(gfp, "nuc %d\n", data->nuc);
	for (i = 0; i < data->nuc; i++) {
		gzprintf(gfp, "%.15e %.15e %.15e %d\n", data->uc[i][0], data->uc[i][1], data->uc[i][2], data->uc_types[i]);
	}
	gzprintf(gfp, "pv\n");
	for (i = 0; i < 3; i++) {
		gzprintf(gfp, "%.15e %.15e %.15e\n", data->pv[i][0], data->pv[i][1], data->pv[i][2]);
	}
	gzprintf(gfp, "size %d %d %d\n", data->size[0], data->size[1], data->size[2]);
	gzprintf(gfp, "ncluster %d\n", data->ncluster);
	for (i = 0; i < data->ncluster; i++) {
		gzprintf(gfp, "%.15e %.15e %.15e %d\n", data->cluster[i][0], data->cluster[i][1], data->cluster[i][2], data->jcluster[i]);
	}
	gzprintf(gfp, "nsolute_max %d\n", data->nsolute_max);
	gzprintf(gfp, "ntable_step %d\n", data->ntable_step);
	gzprintf(gfp, "nevent_step %d\n", data->nevent_step);
	gzprintf(gfp, "nrot %d\n", data->nrot);
	for (i = 0; i < data->nrot; i++) {
		sp = i * data->ncluster;
		for (j = 0; j < data->ncluster; j++) {
			gzprintf(gfp, "%d ", data->rotid[sp+j]);
		}
		gzprintf(gfp, "\n");
	}
	gzprintf(gfp, "rand_seed %d\n", data->rand_seed);
	gzprintf(gfp, "step %d\n", data->step);
	gzprintf(gfp, "tote %.15e\n", data->tote);
	gzprintf(gfp, "table_use %d\n", data->table_use);
	gzprintf(gfp, "%s\n", data->htable);
	gzprintf(gfp, "ntable %d\n", data->ntable);
	for (i = 0; i < data->ntable; i++) {
		for (j = 0; j < data->ncluster; j++) {
			gzprintf(gfp, "%d ", data->table[i].types[j]);
		}
		gzprintf(gfp, "%.15e %d\n", data->table[i].energy, data->table[i].refcount);
	}
	gzprintf(gfp, "nsolute %d\n", data->nsolute);
	for (i = 0; i < data->nsolute; i++) {
		gzprintf(gfp, "%d %d %d %d\n", data->solute[i].id, data->solute[i].type, data->solute[i].jump, data->solute[i].njump);
	}
	gzprintf(gfp, "nevent %d\n", data->nevent);
	for (i = 0; i < data->nevent; i++) {
		gzprintf(gfp, "%d %d %d %.15e\n", data->event[i].dp, data->event[i].id0, data->event[i].id1, data->event[i].de);
	}
	gzclose(gfp);
	return TRUE;
}

static void ScanRotIndex(char buf[], int ncluster, int ids[])
{
	int i;
	char *tok;

	tok = strtok(buf, " ");
	for (i = 0; i < ncluster; i++) {
		ids[i] = atoi(tok);
		tok = strtok(NULL, " ");
	}
}

static int KMCRead1(MP_KMCData *data, char *filename)
{	
	int i;
	gzFile gfp;
	char buf[256], dum[256];
	char *p;
	int nuc;
	double uc[MP_KMC_NUC_MAX][3];
	short uc_types[MP_KMC_NUC_MAX];
	double pv[3][3];
	int nx, ny, nz;
	int ncluster;
	double cluster[MP_KMC_NCLUSTER_MAX][3];
	short jcluster[MP_KMC_NCLUSTER_MAX];
	int nsolute_max;
	int ntable_step;
	int nevent_step;
	int nrot;
	int ids[MP_KMC_NCLUSTER_MAX];
	int ntable;
	short types[MP_KMC_NCLUSTER_MAX];
	double energy;
	long refcount;
	int nsolute;
	int nevent;
	int id;
	short type, jump;
	int njump;
	int dp, id0, id1;
	double de;

	if ((gfp = gzopen(filename, "rb")) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCRead)\n", filename);
		return FALSE;
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nuc);
	for (i = 0; i < nuc; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%le %le %le %hd", &(uc[i][0]), &(uc[i][1]), &(uc[i][2]), &(uc_types[i]));
	}
	gzgets(gfp, buf, 256);
	for (i = 0; i < 3; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%le %le %le", &(pv[i][0]), &(pv[i][1]), &(pv[i][2]));
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d %d %d", dum, &nx, &ny, &nz);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &ncluster);
	for (i = 0; i < ncluster; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%le %le %le %hd", &(cluster[i][0]), &(cluster[i][1]), &(cluster[i][2]), &(jcluster[i]));
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nsolute_max);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &ntable_step);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nevent_step);
	if (!MP_KMCAlloc(data, nuc, nx, ny, nz, ncluster, nsolute_max, ntable_step, nevent_step)) return FALSE;
	MP_KMCSetUnitCell(data, uc, uc_types, pv);
	MP_KMCSetCluster(data, cluster, jcluster);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nrot);
	for (i = 0; i < nrot; i++) {
		gzgets(gfp, buf, 256);
		ScanRotIndex(buf, ncluster, ids);
		MP_KMCAddRotIndex(data, ids);
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %ld", dum, &(data->rand_seed));
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %ld", dum, &(data->step));
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %le", dum, &(data->tote));
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &(data->table_use));
	gzgets(gfp, data->htable, 256);
	p = strchr(data->htable, '\n');
	if (p != NULL) *p = '\0';
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &ntable);
	for (i = 0; i < ntable; i++) {
		gzgets(gfp, buf, 256);
		ScanTable(buf, ncluster, types, &energy, &refcount);
		MP_KMCAddCluster(data, types, energy, refcount);
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nsolute);
	for (i = 0; i < nsolute; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%d %hd %hd %d", &id, &type, &jump, &njump);
		MP_KMCAddSolute(data, id, type, jump);
		data->solute[data->nsolute-1].njump = njump;
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nevent);
	for (i = 0; i < nevent; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%d %d %d %le", &dp, &id0, &id1, &de);
		AddEvent(data, dp, id0, id1, de);
	}
	gzclose(gfp);
	return TRUE;
}

static int Modid0to1(MP_KMCData *data, int nx, int ny, int id)
{
	int i;
	int a, b;
	int x, y, z;
	int uc[][3] = { {0, 0, 0}, {1, 1, 0}, {1, 0, 1}, {0, 1, 1} };

	a = nx * ny;
	z = id / a;
	b = id - z * a;
	y = b / nx;
	x = b - y * nx;
	for (i = 0; i < 4; i++) {
		if (x % 2 == uc[i][0] && y % 2 == uc[i][1] && z % 2 == uc[i][2]) break;
	}
	return MP_KMCGrid2Index(data, i, x / 2, y / 2, z / 2);
}

static int KMCRead0(MP_KMCData *data, char *filename)
{
	gzFile gfp;
	char buf[256], dum[256];
	int lat_type;
	int nx, ny, nz;
	short solvent;
	int nsolute_max;
	int ntable_step;
	int nevent_step;
	int ntable;
	short types[13];
	double energy;
	long refcount;
	int nsolute;
	int nevent;
	int id;
	short type, jump;
	int dp, id0, id1;
	char *p;
	int i;
	int update;
	double cle0, cle1;
	int ids0[MP_KMC_NCLUSTER_MAX];
	int ids1[MP_KMC_NCLUSTER_MAX];
	int ids2[MP_KMC_NCLUSTER_MAX * 2];
	int nncluster;
	double ecluster[MP_KMC_NCLUSTER_MAX * 2];
	double uc[][3] = { { 0.0, 0.0, 0.0 },{ 0.5, 0.5, 0.0 },{ 0.5, 0.0, 0.5 },{ 0.0, 0.5, 0.5 } };
	short uc_types[4];
	double pv[][3] = { { 1.0, 0.0, 0.0 },{ 0.0, 1.0, 0.0 },{ 0.0, 0.0, 1.0 } };
	double cluster[][3] = { { 0, 0, 0 },{ 0.5, 0.5, 0 },{ 0, 0.5, -0.5 },{ -0.5, 0, -0.5 },{ -0.5, 0.5, 0 },
	{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
	{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 } };
	short jcluster[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	int rotid[][13] = { { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }, { 0, 6, 5, 4, 9, 8, 7, 12, 11, 10, 3, 2, 1 },
	{ 0, 7, 8, 9, 10, 11, 12, 1, 2, 3, 4, 5, 6 }, { 0, 12, 11, 10, 3, 2, 1, 6, 5, 4, 9, 8, 7 },
	{ 0, 4, 3, 11, 10, 9, 5, 1, 6, 8, 7, 12, 2 }, { 0, 9, 4, 2, 3, 10, 8, 6, 7, 11, 12, 1, 5 },
	{ 0, 10, 9, 5, 4, 3, 11, 7, 12, 2, 1, 6, 8 }, { 0, 3, 10, 8, 9, 4, 2, 12, 1, 5, 6, 7, 11 },
	{ 0, 10, 11, 12, 7, 8, 9, 4, 5, 6, 1, 2, 3 }, { 0, 3, 2, 1, 12, 11, 10, 9, 8, 7, 6, 5, 4 },
	{ 0, 4, 5, 6, 1, 2, 3, 10, 11, 12, 7, 8, 9 }, { 0, 9, 8, 7, 6, 5, 4, 3, 2, 1, 12, 11, 10 },
	{ 0, 7, 12, 2, 1, 6, 8, 10, 9, 5, 4, 3, 11 }, { 0, 12, 1, 5, 6, 7, 11, 3, 10, 8, 9, 4, 2 },
	{ 0, 1, 6, 8, 7, 12, 2, 4, 3, 11, 10, 9, 5 }, { 0, 6, 7, 11, 12, 1, 5, 9, 4, 2, 3, 10, 8 },
	{ 0, 5, 1, 12, 2, 4, 9, 8, 10, 3, 11, 7, 6 }, { 0, 8, 6, 1, 5, 9, 10, 11, 3, 4, 2, 12, 7 },
	{ 0, 11, 7, 6, 8, 10, 3, 2, 4, 9, 5, 1, 12 }, { 0, 2, 12, 7, 11, 3, 4, 5, 9, 10, 8, 6, 1 },
	{ 0, 2, 4, 9, 5, 1, 12, 11, 7, 6, 8, 10, 3 }, { 0, 5, 9, 10, 8, 6, 1, 2, 12, 7, 11, 3, 4 },
	{ 0, 8, 10, 3, 11, 7, 6, 5, 1, 12, 2, 4, 9 }, { 0, 11, 3, 4, 2, 12, 7, 8, 6, 1, 5, 9, 10 } };

	if ((gfp = gzopen(filename, "rb")) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCRead0)\n", filename);
		return FALSE;
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &lat_type);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d %d %d", dum, &nx, &ny, &nz);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %hd", dum, &solvent);
	for (i = 0; i < 4; i++) uc_types[i] = solvent;
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nsolute_max);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &ntable_step);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nevent_step);
	if (!MP_KMCAlloc(data, 4, nx/2, ny/2, nz/2, 13, nsolute_max, ntable_step, nevent_step)) return FALSE;
	MP_KMCSetUnitCell(data, uc, uc_types, pv);
	MP_KMCSetCluster(data, cluster, jcluster);
	for (i = 0; i < 24; i++) {
		MP_KMCAddRotIndex(data, rotid[i]);
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %ld", dum, &(data->rand_seed));
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %ld", dum, &(data->step));
	gzgets(gfp, data->htable, 256);
	p = strchr(data->htable, '\n');
	if (p != NULL) *p = '\0';
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &ntable);
	if (lat_type == MP_KMCFCC) {
		for (i = 0; i < ntable; i++) {
			gzgets(gfp, buf, 256);
			sscanf(buf, "%hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %le %ld",
				&(types[0]), &(types[1]), &(types[2]), &(types[3]), &(types[4]),
				&(types[5]), &(types[6]), &(types[7]), &(types[8]), &(types[9]),
				&(types[10]), &(types[11]), &(types[12]), &energy, &refcount);
			MP_KMCAddCluster(data, types, energy, refcount);
		}
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nsolute);
	for (i = 0; i < nsolute; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%d %hd %hd", &id, &type, &jump);
		MP_KMCAddSolute(data, Modid0to1(data, nx, ny, id), type, jump);
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nevent);
	for (i = 0; i < nevent; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%d %d %d", &dp, &id0, &id1);
		AddEvent(data, dp, Modid0to1(data, nx, ny, id0), Modid0to1(data, nx, ny, id1), 0.0);
	}
	gzclose(gfp);
	if (nevent > 0) {
		while (data->step > 0) {
			data->step--;
			dp = data->event[data->step].dp;
			id0 = data->event[data->step].id0;
			id1 = data->event[data->step].id1;
			SwapType(data, id0, id1);
			data->solute[dp].id = id0;
		}
		while (data->step < nevent) {
			dp = data->event[data->step].dp;
			id0 = data->event[data->step].id0;
			id1 = data->event[data->step].id1;
			MP_KMCClusterIndexes(data, id0, ids0);
			MP_KMCClusterIndexes(data, id1, ids1);
			nncluster = UniteIndexes(data->ncluster, ids0, ids1, ids2);			
			cle0 = ClusterEnergy(data, NULL, nncluster, ids2, ecluster, &update);
			SwapType(data, id0, id1);
			cle1 = ClusterEnergy(data, NULL, nncluster, ids2, ecluster, &update);
			data->solute[dp].id = id1;
			data->event[data->step].de = cle1 - cle0;
			data->step++;
		}
		MP_KMCTotalEnergy(data, NULL, &update);
	}
	return TRUE;
}

int MP_KMCRead(MP_KMCData *data, char *filename, int version)
{
	switch (version) {
	case 0:
		return KMCRead0(data, filename);
	case 1:
		return KMCRead1(data, filename);
	default:
		fprintf(stderr, "Error : Invalid version, %d(MP_KMCRead)\n", version);
		return FALSE;
	}
}
