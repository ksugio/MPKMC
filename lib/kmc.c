#include "MPKMC.h"

int MP_KMCAlloc(MP_KMCData *data, int nuc, int nx, int ny, int nz, int ncluster,
	int nsolute_step, int ntable_step, int nevent_step, int nhistory_step)
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
	data->clusterid = (int *)malloc(nuc*ncluster*4*sizeof(int));
	data->rotid = (int *)malloc(MP_KMC_NROT_MAX*ncluster*sizeof(int));
	data->solute = (MP_KMCSoluteItem *)malloc(nsolute_step*sizeof(MP_KMCSoluteItem));
	data->event = (MP_KMCEventItem *)malloc(nevent_step*sizeof(MP_KMCEventItem));
	data->history = (MP_KMCHistoryItem *)malloc(nhistory_step*sizeof(MP_KMCHistoryItem));
	if (data->grid == NULL || data->table == NULL || data->clusterid == NULL
		|| data->rotid == NULL || data->solute == NULL || data->event == NULL
		|| data->history == NULL) return FALSE;
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
	}
	data->save_grid = FALSE;
	data->jpmax = 0;
	data->nrot = 0;
	data->table_use = TRUE;
	data->ntable = 0;
	data->ntable_max = data->ntable_step = ntable_step;
	data->htable[0] = '\0';
	data->nsolute = 0;
	data->nsolute_max = data->nsolute_step = nsolute_step;
	data->dpmax = 0;
	data->ngroup = 0;
	data->event_record = TRUE;
	data->nevent = 0;
	data->nevent_max = data->nevent_step = nevent_step;
	data->event_pt = 0;
	data->nhistory = 0;
	data->nhistory_max = data->nhistory_step = nhistory_step;
	data->rand_seed = 12061969;
	data->totmcs = 0;
	data->mcs = 0;
	data->tote = 0.0;
	data->kb = 86.1735e-6; // ev/K
	return TRUE;
}

void MP_KMCFree(MP_KMCData *data)
{
	free(data->grid);
	free(data->table);
	free(data->clusterid);
	free(data->rotid);
	free(data->solute);
	free(data->event);
	free(data->history);
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
	double tol = 1.0e-9;
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
			if (fabs(x - cluster[0]) < tol && fabs(y - cluster[1]) < tol && fabs(z - cluster[2]) < tol) {
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

int MP_KMCSetCluster(MP_KMCData *data, double cluster[][3], int jpmax)
{
	int i, j;
	int np, dx, dy, dz;
	int pt;
	double rp[3];

	if (jpmax > data->ncluster || jpmax < 0) {
		fprintf(stderr, "Error : Invalid maximum jump pointer, jpmax (MP_KMCSetCluster).\n");
		return FALSE;
	}
	else data->jpmax = jpmax;
	for (i = 0; i < data->ncluster; i++) {
		data->cluster[i][0] = cluster[i][0];
		data->cluster[i][1] = cluster[i][1];
		data->cluster[i][2] = cluster[i][2];
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
				fprintf(stderr, "Error : Cluster position not match to Unitcell position (MP_KMCSetCluster).\n");
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

void MP_KMCClusterTypes(MP_KMCData *data, int id, short types[])
{
	int i;
	int ids[MP_KMC_NCLUSTER_MAX];

	MP_KMCClusterIndexes(data, id, ids);
	for (i = 0; i < data->ncluster; i++) {
		types[i] = data->grid[ids[i]].type;
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
		if (data->table == NULL) {
			fprintf(stderr, "Error : allocation failure (MP_KMCAddCluster)\n");
			return MP_KMC_MEM_ERR;
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

int MP_KMCCountType(MP_KMCData *data, short type)
{
	int i;
	int count = 0;

	for (i = 0; i < data->ntot; i++) {
		if (data->grid[i].type == type) count++;
	}
	return count;
}

/*double MP_KMCCalcEnergy(MP_KMCData *data, int id, double(*func)(MP_KMCData *, short *), int *update)
{
	short types[MP_KMC_NCLUSTER_MAX];
	int tid;

	*update = FALSE;
	if (data->grid[id].type > 0) {
		MP_KMCClusterTypes(data, id, types);
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
}*/

static int SortCompRefcount(const void *c1, const void *c2)
{
	MP_KMCTableItem *t1 = (MP_KMCTableItem *)c1;
	MP_KMCTableItem *t2 = (MP_KMCTableItem *)c2;

	if (t1->refcount < t2->refcount) return 1;
	else if (t1->refcount == t2->refcount) return 0;
	else return -1;
}

static int SortCompEnergy(const void *c1, const void *c2)
{
	MP_KMCTableItem *t1 = *(MP_KMCTableItem **)c1;
	MP_KMCTableItem *t2 = *(MP_KMCTableItem **)c2;

	if (t1->energy > t2->energy) return 1;
	else if (t1->energy == t2->energy) return 0;
	else return -1;
}

void MP_KMCSortTable(MP_KMCData *data)
{
	qsort(data->table, data->ntable, sizeof(MP_KMCTableItem), SortCompRefcount);
}

void MP_KMCResetTable(MP_KMCData *data)
{
	int i;

	for (i = 0; i < data->ntable; i++) {
		data->table[i].refcount = 0;
	}
}

struct KMCTableCond {
	int pos;
	short type;
	int num;
};

static int KMCSearchTable(MP_KMCData *data, int ncond, struct KMCTableCond cond[], MP_KMCTableItem list[], int list_max)
{
	int i, j, k;
	int c, cm;
	int nlist = 0;
	MP_KMCTableItem **table;

	table = (MP_KMCTableItem **)malloc(data->ntable*sizeof(MP_KMCTableItem *));
	for (i = 0; i < data->ntable; i++) {
		table[i] = &(data->table[i]);
	}
	qsort(table, data->ntable, sizeof(MP_KMCTableItem *), SortCompEnergy);
	for (i = 0; i < data->ntable; i++) {
		for (j = 0, cm = 0; j < ncond; j++) {
			if (cond[j].pos >= 0 && cond[j].pos < data->ncluster) {
				if (cond[j].type == table[i]->types[cond[j].pos]) cm++;
			}
			else {
				for (k = 0, c = 0; k < data->ncluster; k++) {
					if (cond[j].type == table[i]->types[k]) c++;
				}
				if (c == cond[j].num) cm++;
			}
		}
		if (cm == ncond) {
			list[nlist++] = *(table[i]);
			if (nlist >= list_max) break;
		}
	}
	free(table);
	return nlist;
}

static int GetValue(char *p)
{
	char s[256];
	int c = 0;

	while (TRUE) {
		if (*p == 'p' || *p == 't' || *p == 'n'
			|| *p == ',' || *p == '\0') {
			s[c] = '\0';
			return atoi(s);
		}
		s[c++] = *p++;
	}
}

int MP_KMCSearchTable(MP_KMCData *data, char ss[], MP_KMCTableItem list[], int list_max)
{
	char *p = ss;
	int ncond = 0;
	struct KMCTableCond cond[64];

	cond[ncond].pos = -1;
	cond[ncond].type = -1;
	cond[ncond].num = -1;
	while (*p != '\0') {
		if (*p == 'p') {
			cond[ncond].pos = GetValue(++p);
		}
		else if (*p == 't') {
			cond[ncond].type = (short)GetValue(++p);
		}
		else if (*p == 'n') {
			cond[ncond].num = GetValue(++p);
		}
		else ++p;
		if (*p == ',' || *p == '\0') {
			ncond++;
			if (ncond >= 64) break;
			cond[ncond].pos = -1;
			cond[ncond].type = -1;
			cond[ncond].num = -1;
		}
	}
	return KMCSearchTable(data, ncond, cond, list, list_max);
}
