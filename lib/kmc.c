#include "MPKMC.h"

int MP_KMCAlloc(MP_KMCData *data, int nuc, int nx, int ny, int nz, int ncluster,
	int nsolute_max, int ntable_step, int nevent_step)
{
	int i;

	if (nuc > MP_KMC_NUC_MAX) return FALSE;
	if (ncluster > MP_KMC_NCLUSTER_MAX) return FALSE;
	data->nuc = nuc;
	data->size[0] = nx, data->size[1] = ny, data->size[2] = nz;
	data->ntot = nuc * nx * ny * nz;
	data->ncluster = ncluster;
	data->grid = (MP_KMCGridItem *)malloc(data->ntot*sizeof(MP_KMCGridItem));
	data->table = (MP_KMCTableItem *)malloc(ntable_step*sizeof(MP_KMCTableItem));
	data->table_types = (short *)malloc(ncluster*ntable_step*sizeof(short));
	data->rot_index = (int **)malloc(MP_KMC_NROT_MAX*sizeof(int *));
	data->rot_index_et = (int *)malloc(ncluster*MP_KMC_NROT_MAX*sizeof(int));
	data->solute = (MP_KMCSoluteItem *)malloc(nsolute_max*sizeof(MP_KMCSoluteItem));
	data->event = (MP_KMCEventItem *)malloc(nevent_step*sizeof(MP_KMCEventItem));
	if (data->grid == NULL || data->table == NULL || data->table_types == NULL
		|| data->rot_index == NULL || data->rot_index_et == NULL || data->solute == NULL
		|| data->event == NULL) return FALSE;
	for (i = 0; i < nuc; i++) {
		data->uc[i][0] = 0.0, data->uc[i][1] = 0.0, data->uc[i][2] = 0.0;
	}
	for (i = 0; i < data->ntot; i++) {
		data->grid[i].type = 0;
		data->grid[i].energy = 0.0;
	}	
	for (i = 0; i < MP_KMC_NROT_MAX; i++) {
		data->rot_index[i] = data->rot_index_et + i*ncluster;
	}
	for (i = 0; i < ntable_step; i++) {
		data->table[i].types = data->table_types + i*ncluster;
	}
	data->nrot = 0;
	data->ntable = 0;
	data->ntable_max = data->ntable_step = ntable_step;
	data->htable[0] = '\0';
	data->solvent = -1;
	data->nsolute = 0;
	data->nsolute_max = nsolute_max;
	data->ntypes = 0;
	data->nevent = 0;
	data->nevent_max = data->nevent_step = nevent_step;
	data->rand_seed = 12061969;
	data->step = 0;
	return TRUE;
}

void MP_KMCFree(MP_KMCData *data)
{
	free(data->grid);
	free(data->table);
	free(data->table_types);
	free(data->solute);
	free(data->event);
}

void MP_KMCSetUnitCell(MP_KMCData *data, double uc[][3])
{
	int i;

	for (i = 0; i < data->nuc; i++) {
		data->uc[i][0] = uc[i][0];
		data->uc[i][1] = uc[i][1];
		data->uc[i][2] = uc[i][2];
	}
}

void MP_KMCSetCluster(MP_KMCData *data, double cluster[][3])
{
	int i;

	for (i = 0; i < data->ncluster; i++) {
		data->cluster[i][0] = cluster[i][0];
		data->cluster[i][1] = cluster[i][1];
		data->cluster[i][2] = cluster[i][2];
	}
}

void MP_KMCSetSolvent(MP_KMCData *data, short type)
{
	int i;

	for (i = 0; i < data->ntot; i++) {
		data->grid[i].type = type;
	}
	data->solvent = type;
	data->types[data->ntypes++] = type;
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

static int SearchNeigh(MP_KMCData *data, int cp, int p, int x, int y, int z, int *np, int *nx, int *ny, int *nz)
{
	int i, j;
	double dx, dy, dz;
	int neigh[27][3] = {
		{ 0, 0, 0 },{ 1, 0, 0 },{ -1, 0, 0 },{ 0, 1, 0 },{ 0, -1, 0 },{ 1, 1, 0 },
		{ 1, -1, 0 },{ -1, 1, 0 },{ -1, -1, 0 },{ 0, 0, 1 },{ 1, 0, 1 },{ -1, 0, 1 },
		{ 0, 1, 1 },{ 0, -1, 1 },{ 1, 1, 1 },{ 1, -1, 1 },{ -1, 1, 1 },{ -1, -1, 1 },
		{ 0, 0, -1 },{ 1, 0, -1 },{ -1, 0, -1 },{ 0, 1, -1 },{ 0, -1, -1 },{ 1, 1, -1 },
		{ 1, -1, -1 },{ -1, 1, -1 },{ -1, -1, -1 } };

	for (i = 0; i < 27; i++) {
		for (j = 0; j < data->nuc; j++) {
			dx = neigh[i][0] + data->uc[j][0] - data->uc[p][0];
			dy = neigh[i][1] + data->uc[j][1] - data->uc[p][1];
			dz = neigh[i][2] + data->uc[j][2] - data->uc[p][2];
			if (dx == data->cluster[cp][0] && dy == data->cluster[cp][1] && dz == data->cluster[cp][2]) {
				*np = j;
				*nx = x + neigh[i][0];
				*ny = y + neigh[i][1];
				*nz = z + neigh[i][2];
				if (*nx < 0) {
					*nx += data->size[0];
				}
				else if (*nx >= data->size[0]) {
					*nx -= data->size[0];
				}
				if (*ny < 0) {
					*ny += data->size[1];
				}
				else if (*ny >= data->size[1]) {
					*ny -= data->size[1];
				}
				if (*nz < 0) {
					*nz += data->size[2];
				}
				else if (*nz >= data->size[2]) {
					*nz -= data->size[2];
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

int MP_KMCClusterIndexes(MP_KMCData *data, int id, int ids[])
{
	int i;
	int p, x, y, z;
	int np, nx, ny, nz;

	MP_KMCIndex2Grid(data, id, &p, &x, &y, &z);
	for (i = 0; i < data->ncluster; i++) {
		if (SearchNeigh(data, i, p, x, y, z, &np, &nx, &ny, &nz)) {
			ids[i] = MP_KMCGrid2Index(data, np, nx, ny, nz);
		}
		else {
			return FALSE;
		}
	}
	return TRUE;
}

int MP_KMCSearchCluster(MP_KMCData *data, short types[])
{
	int i, j, k;
	int count;

	for (i = 0; i < data->ntable; i++) {
		if (data->table[i].types[0] == types[0]) {
			for (j = 0; j < data->nrot; j++) {
				count = 0;
				for (k = 1; k < data->ncluster; k++) {
					if (data->table[i].types[k] == types[data->rot_index[j][k]]) count++;
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
	if (data->grid[id].type == data->solvent) {
		data->grid[id].type = type;
		data->solute[data->nsolute].id = id;
		data->solute[data->nsolute].type = type;
		data->solute[data->nsolute].jump = jump;
		data->nsolute++;
		for (i = 0; i < data->ntypes; i++) {
			if (data->types[i] == type) break;
		}
		if (i == data->ntypes) {
			data->types[data->ntypes++] = type;
		}
		return TRUE;
	}
	else return FALSE;
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
	else if (data->grid[id].type == 0) {
		data->grid[id].energy = 0.0;
	}
	return data->grid[id].energy;
}

double MP_KMCTotalEnergy(MP_KMCData *data)
{
	int i;
	double tot = 0.0;

	for (i = 0; i < data->ntot; i++) {
		if (data->grid[i].type >= 0) {
			tot += data->grid[i].energy;
		}
	}
	return tot;
}

static int UniteIndexes(int ncol, int ids0[], int ids1[], int ids2[])
{
	int i, j;
	int count = ncol;

	for (i = 0; i < ncol; i++) {
		ids2[i] = ids0[i];
	}
	count = i;
	for (j = 0; j < ncol; j++) {
		for (i = 0; i < ncol; i++) {
			if (ids1[j] == ids0[i]) break;
		}
		if (i == ncol) {
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

static double GridClusterEnergy(MP_KMCData *data, int nncol, int ids[])
{
	int i;
	double cle = 0.0;

	for (i = 0; i < nncol; i++) {
		cle += data->grid[ids[i]].energy;
	}
	return cle;
}

static double ClusterEnergy(MP_KMCData *data, int nncol, double energy[])
{
	int i;
	double cle = 0.0;

	for (i = 0; i < nncol; i++) {
		cle += energy[i];
	}
	return cle;
}

static void AddEvent(MP_KMCData *data, int dp, int id0, int id1)
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
	data->nevent++;
}

int MP_KMCJump(MP_KMCData *data, double kt, double(*func)(MP_KMCData *, short *), int *update)
{
	int j, k;
	int dp, cp;
	int id0, id1;
	int ids0[MP_KMC_NCLUSTER_MAX];
	int ids1[MP_KMC_NCLUSTER_MAX];
	int ids2[MP_KMC_NCLUSTER_MAX * 2];
	int ids3[MP_KMC_NCLUSTER_MAX];
	short types3[MP_KMC_NCLUSTER_MAX];
	int nncol;
	int tid;
	double energy[26];
	double cle0, cle1, clde;

	dp = (int)(MP_Rand(&(data->rand_seed)) * data->nsolute);
	cp = (int)(MP_Rand(&(data->rand_seed)) * (data->ncluster - 1)) + 1;
	*update = FALSE;
	if (dp >= data->nsolute || cp >= data->ncluster) return FALSE;
	if (!data->solute[dp].jump) return FALSE;
	id0 = data->solute[dp].id;
	MP_KMCClusterIndexes(data, id0, ids0);
	id1 = ids0[cp];
	if (data->grid[id0].type == data->grid[id1].type) {
		return FALSE;
	}
	MP_KMCClusterIndexes(data, id1, ids1);
	nncol = UniteIndexes(data->ncluster, ids0, ids1, ids2);
	cle0 = GridClusterEnergy(data, nncol, ids2);
	SwapType(data, id0, id1);
	for (j = 0; j < nncol; j++) {
		if (data->grid[ids2[j]].type > 0) {
			MP_KMCClusterIndexes(data, ids2[j], ids3);
			for (k = 0; k < data->ncluster; k++) {
				types3[k] = data->grid[ids3[k]].type;
			}
			tid = MP_KMCSearchCluster(data, types3);
			if (tid >= 0) {
				data->table[tid].refcount += 1;
				energy[j] = data->table[tid].energy;
			}
			else {
				if (func != NULL) {
					energy[j] = (func)(data, types3);
					MP_KMCAddCluster(data, types3, energy[j], 0);
					*update = TRUE;
				}
				else {
					energy[j] = 0.0;
				}
			}
		}
		else if (data->grid[ids2[j]].type == 0) {
			energy[j] = 0.0;
		}
	}
	cle1 = ClusterEnergy(data, nncol, energy);
	clde = cle1 - cle0;
	if (clde < 0.0 || MP_Rand(&(data->rand_seed)) < exp(-clde / kt)) {
		for (j = 0; j < nncol; j++) {
			data->grid[ids2[j]].energy = energy[j];
		}
		data->solute[dp].id = id1;
		AddEvent(data, dp, id0, id1);
		data->step++;
		return TRUE;
	}
	else {
		SwapType(data, id0, id1);
		return FALSE;
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
		//printf("F %d, %d -> %d\n", id0, data->diffuse[dp].id, id1);
		SwapType(data, id0, id1);
		data->solute[dp].id = id1;
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
		//printf("B %d, %d -> %d\n", id1, data->diffuse[dp].id, id0);
		SwapType(data, id0, id1);
		data->solute[dp].id = id0;
	}
}

void MP_KMCStepGo(MP_KMCData *data, int step)
{
	int count;

	count = step - data->step;
	if (count > 0) MP_KMCStepForward(data, count);
	else if (count < 0) MP_KMCStepBackward(data, -count);

}

void MP_KMCWriteTable(MP_KMCData *data, char *filename)
{
	int i, j;
	FILE *fp;

	if ((fp = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCWriteTable)\n", filename);
		return;
	}
	fprintf(fp, "%s\n", data->htable);
	fprintf(fp, "%d %d\n", data->ntable, data->ncluster);
	for (i = 0; i < data->ntable; i++) {
		for (j = 0; j < data->ncluster; j++) {
			fprintf(fp, "%d ", data->table[i].types[j]);
		}
		fprintf(fp, "%.15e %d\n", data->table[i].energy, data->table[i].refcount);
	}
	fclose(fp);
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

void MP_KMCReadTable(MP_KMCData *data, char *filename)
{
	FILE *fp;
	int ntable, ncluster;
	short types[MP_KMC_NCLUSTER_MAX];
	double energy;
	long refcount;
	char *p;
	char buf[256];
	int i;

	if ((fp = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCReadTable)\n", filename);
		return;
	}
	fgets(data->htable, 256, fp);
	p = strchr(data->htable, '\n');
	if (p != NULL) *p = '\0';
	fscanf(fp, "%d %d\n", &ntable, &ncluster);
	if (ncluster != data->ncluster) {
		fprintf(stderr, "Error : incompatible format, %s.(MP_KMCReadTable)\n", filename);
	}
	for (i = 0; i < ntable;i++) {
		fgets(buf, 256, fp);
		ScanTable(buf, ncluster, types, &energy, &refcount);
		MP_KMCAddCluster(data, types, energy, refcount);
	}
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
	gzFile gfp;
	char mode[32];

	sprintf(mode, "wb%df", comp);
	if ((gfp = gzopen(filename, mode)) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCWrite)\n", filename);
		return FALSE;
	}
	gzprintf(gfp, "nuc %d\n", data->nuc);
	for (i = 0; i < data->nuc; i++) {
		gzprintf(gfp, "%.15e %.15e %.15e\n", data->uc[i][0], data->uc[i][1], data->uc[i][2]);
	}
	gzprintf(gfp, "size %d %d %d\n", data->size[0], data->size[1], data->size[2]);
	gzprintf(gfp, "ncluster %d\n", data->ncluster);
	for (i = 0; i < data->ncluster; i++) {
		gzprintf(gfp, "%.15e %.15e %.15e\n", data->cluster[i][0], data->cluster[i][1], data->cluster[i][2]);
	}
	gzprintf(gfp, "nsolute_max %d\n", data->nsolute_max);
	gzprintf(gfp, "ntable_step %d\n", data->ntable_step);
	gzprintf(gfp, "nevent_step %d\n", data->nevent_step);
	gzprintf(gfp, "solvent %d\n", data->solvent);
	gzprintf(gfp, "nrot %d\n", data->nrot);
	for (i = 0; i < data->nrot; i++) {
		for (j = 0; j < data->ncluster; j++) {
			gzprintf(gfp, "%d ", data->rot_index[i][j]);
		}
		gzprintf(gfp, "\n");
	}
	gzprintf(gfp, "rand_seed %d\n", data->rand_seed);
	gzprintf(gfp, "step %d\n", data->step);
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
		gzprintf(gfp, "%d %d %d\n", data->solute[i].id, data->solute[i].type, data->solute[i].jump);
	}
	gzprintf(gfp, "nevent %d\n", data->nevent);
	for (i = 0; i < data->nevent; i++) {
		gzprintf(gfp, "%d %d %d\n", data->event[i].dp, data->event[i].id0, data->event[i].id1);
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

int MP_KMCRead(MP_KMCData *data, char *filename)
{	
	int i;
	gzFile gfp;
	char buf[256], dum[256];
	char *p;
	int nuc;
	double uc[MP_KMC_NUC_MAX][3];
	int nx, ny, nz;
	int ncluster;
	double cluster[MP_KMC_NCLUSTER_MAX][3];
	int nsolute_max;
	int ntable_step;
	int nevent_step;
	short solvent;
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
	int dp, id0, id1;

	if ((gfp = gzopen(filename, "rb")) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCRead)\n", filename);
		return FALSE;
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nuc);
	for (i = 0; i < nuc; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%le %le %le", &(uc[i][0]), &(uc[i][1]), &(uc[i][2]));
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d %d %d", dum, &nx, &ny, &nz);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &ncluster);
	for (i = 0; i < ncluster; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%le %le %le", &(cluster[i][0]), &(cluster[i][1]), &(cluster[i][2]));
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nsolute_max);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &ntable_step);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nevent_step);
	if (!MP_KMCAlloc(data, nuc, nx, ny, nz, ncluster, nsolute_max, ntable_step, nevent_step)) return FALSE;
	MP_KMCSetUnitCell(data, uc);
	MP_KMCSetCluster(data, cluster);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %hd", dum, &solvent);
	MP_KMCSetSolvent(data, solvent);
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
		sscanf(buf, "%d %hd %hd", &id, &type, &jump);
		MP_KMCAddSolute(data, id, type, jump);
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nevent);
	for (i = 0; i < nevent; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%d %d %d", &dp, &id0, &id1);
		AddEvent(data, dp, id0, id1);
	}
	gzclose(gfp);
	return TRUE;
}
