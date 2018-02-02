#include "MPKMC.h"

int MP_KMCAlloc(MP_KMCData *data, int lat_type, int nx, int ny, int nz,
	short solvent, int nsolute_max, int ntable_step, int nevent_step)
{
	int i;
	int x, y, z;
	static int fcc_pos[][3] = {
		{ 0, 0, 0 },{ 1, 1, 0 },{ 0, 1, -1 },{ -1, 0, -1 },{ -1, 1, 0 },
		{ 0, 1, 1 },{ 1, 0, 1 },{ 1, -1, 0 },{ 0, -1, 1 },
		{ -1, 0, 1 },{ -1, -1, 0 },{ 0, -1, -1 },{ 1, 0, -1 } };

	data->lat_type = lat_type;
	if (lat_type == MP_KMCFCC) {
		if (nx % 2 == 1 || ny % 2 == 1 || nz % 2 == 1) {
			fprintf(stderr, "Error : nx, ny, nz (%d, %d, %d) is odd number.(MP_KMCAlloc)\n", nx, ny, nz);
			return FALSE;
		}
		data->ncol = 13;
	}
	else {
		return FALSE;
	}
	data->size[0] = nx, data->size[1] = ny, data->size[2] = nz;
	data->ntot = nx * ny * nz;
	data->grid = (MP_KMCGridItem *)malloc(data->ntot*sizeof(MP_KMCGridItem));
	data->table = (MP_KMCTableItem *)malloc(ntable_step*sizeof(MP_KMCTableItem));
	data->solute = (MP_KMCSoluteItem *)malloc(nsolute_max*sizeof(MP_KMCSoluteItem));
	data->event = (MP_KMCEventItem *)malloc(nevent_step*sizeof(MP_KMCEventItem));
	data->cluster_pos = (int **)malloc(data->ncol*sizeof(int *));
	if (data->grid == NULL || data->table == NULL || data->solute == NULL
		|| data->event == NULL || data->cluster_pos == NULL) return FALSE;
	if (lat_type == MP_KMCFCC) {
		for (i = 0; i < data->ncol; i++) {
			data->cluster_pos[i] = fcc_pos[i];
		}
		for (i = 0; i < data->ntot; i++) {
			MP_KMCIndex2Grid(data, i, &x, &y, &z);
			if ((x + y + z) % 2 == 0) {
				data->grid[i].type = solvent;
			}
			else {
				data->grid[i].type = -1;
			}
			data->grid[i].energy = 0.0;
		}
	}
	data->ntable = 0;
	data->ntable_max = data->ntable_step = ntable_step;
	data->htable[0] = '\0';
	data->nsolute = 0;
	data->nsolute_max = nsolute_max;
	data->types[0] = solvent;
	data->ntypes = 1;
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
	free(data->solute);
	free(data->event);
	free(data->cluster_pos);
}

void MP_KMCIndex2Grid(MP_KMCData *data, int id, int *x, int *y, int *z)
{
	int a, b;

	a = data->size[0] * data->size[1];
	*z = id / a;
	b = id - *z * a;
	*y = b / data->size[0];
	*x = b - *y * data->size[0];
}

int MP_KMCGrid2Index(MP_KMCData *data, int x, int y, int z)
{
	return x + y*data->size[0] + z*data->size[0] * data->size[1];
}

void MP_KMCClusterIndexes(MP_KMCData *data, int id, int ids[])
{
	int i;
	int x, y, z;
	int nx, ny, nz;

	MP_KMCIndex2Grid(data, id, &x, &y, &z);
	ids[0] = id;
	for (i = 1; i < data->ncol; i++) {
		nx = x + data->cluster_pos[i][0];
		ny = y + data->cluster_pos[i][1];
		nz = z + data->cluster_pos[i][2];
		if (nx < 0) {
			nx += data->size[0];
		}
		if (nx >= data->size[0]) {
			nx -= data->size[0];
		}
		if (ny < 0) {
			ny += data->size[1];
		}
		if (ny >= data->size[1]) {
			ny -= data->size[1];
		}
		if (nz < 0) {
			nz += data->size[2];
		}
		if (nz >= data->size[2]) {
			nz -= data->size[2];
		}
		ids[i] = MP_KMCGrid2Index(data, nx, ny, nz);
	}
}

static int SearchClusterFCC(MP_KMCData *data, short types[])
{
	int i, j, k;
	int count;
	int rot_index[][13] = {
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
		{0, 6, 5, 4, 9, 8, 7, 12, 11, 10, 3, 2, 1},
		{0, 7, 8, 9, 10, 11, 12, 1, 2, 3, 4, 5, 6},
		{0, 12, 11, 10, 3, 2, 1, 6, 5, 4, 9, 8, 7},
		{0, 4, 3, 11, 10, 9, 5, 1, 6, 8, 7, 12, 2},
		{0, 9, 4, 2, 3, 10, 8, 6, 7, 11, 12, 1, 5},
		{0, 10, 9, 5, 4, 3, 11, 7, 12, 2, 1, 6, 8},
		{0, 3, 10, 8, 9, 4, 2, 12, 1, 5, 6, 7, 11},
		{0, 10, 11, 12, 7, 8, 9, 4, 5, 6, 1, 2, 3},
		{0, 3, 2, 1, 12, 11, 10, 9, 8, 7, 6, 5, 4},
		{0, 4, 5, 6, 1, 2, 3, 10, 11, 12, 7, 8, 9},
		{0, 9, 8, 7, 6, 5, 4, 3, 2, 1, 12, 11, 10},
		{0, 7, 12, 2, 1, 6, 8, 10, 9, 5, 4, 3, 11},
		{0, 12, 1, 5, 6, 7, 11, 3, 10, 8, 9, 4, 2},
		{0, 1, 6, 8, 7, 12, 2, 4, 3, 11, 10, 9, 5},
		{0, 6, 7, 11, 12, 1, 5, 9, 4, 2, 3, 10, 8},
		{0, 5, 1, 12, 2, 4, 9, 8, 10, 3, 11, 7, 6},
		{0, 8, 6, 1, 5, 9, 10, 11, 3, 4, 2, 12, 7},
		{0, 11, 7, 6, 8, 10, 3, 2, 4, 9, 5, 1, 12},
		{0, 2, 12, 7, 11, 3, 4, 5, 9, 10, 8, 6, 1},
		{0, 2, 4, 9, 5, 1, 12, 11, 7, 6, 8, 10, 3},
		{0, 5, 9, 10, 8, 6, 1, 2, 12, 7, 11, 3, 4},
		{0, 8, 10, 3, 11, 7, 6, 5, 1, 12, 2, 4, 9},
		{0, 11, 3, 4, 2, 12, 7, 8, 6, 1, 5, 9, 10} };

	for (i = 0; i < data->ntable; i++) {
		if (data->table[i].types[0] == types[0]) {
			for (j = 0; j < 24; j++) {
				count = 0;
				for (k = 1; k < 13; k++) {
					if (data->table[i].types[k] == types[rot_index[j][k]]) count++;
				}
				if (count == 12) return i;
			}
		}
	}
	return -1;
}

int MP_KMCSearchCluster(MP_KMCData *data, short types[])
{
	if (data->lat_type == MP_KMCFCC) {
		return SearchClusterFCC(data, types);
	}
	return -1;
}

int MP_KMCSearchClusterIDs(MP_KMCData *data, int ids[])
{
	int i;
	short types[13];

	for (i = 0; i < data->ncol; i++) {
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
			return -99;
		}
		data->ntable_max = ntable_max;
	}
	tid = data->ntable;
	for (i = 0; i < data->ncol; i++) {
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
	short types[13];

	for (i = 0; i < data->ncol; i++) {
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
	if (data->grid[id].type == data->types[0]) {
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
	int ids[13];
	short types[13];
	int tid;

	*update = FALSE;
	if (data->grid[id].type > 0) {
		MP_KMCClusterIndexes(data, id, ids);
		for (i = 0; i < data->ncol; i++) {
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
	int ids0[13], ids1[13], ids2[26], ids3[13];
	short types3[13];
	int nncol;
	int tid;
	double energy[26];
	double cle0, cle1, clde;

	dp = (int)(MP_Rand(&(data->rand_seed)) * data->nsolute);
	cp = (int)(MP_Rand(&(data->rand_seed)) * (data->ncol - 1)) + 1;
	*update = FALSE;
	if (dp >= data->nsolute || cp >= data->ncol) return FALSE;
	if (!data->solute[dp].jump) return FALSE;
	id0 = data->solute[dp].id;
	MP_KMCClusterIndexes(data, id0, ids0);
	id1 = ids0[cp];
	if (data->grid[id0].type == data->grid[id1].type) {
		return FALSE;
	}
	MP_KMCClusterIndexes(data, id1, ids1);
	nncol = UniteIndexes(data->ncol, ids0, ids1, ids2);
	cle0 = GridClusterEnergy(data, nncol, ids2);
	SwapType(data, id0, id1);
	for (j = 0; j < nncol; j++) {
		if (data->grid[ids2[j]].type > 0) {
			MP_KMCClusterIndexes(data, ids2[j], ids3);
			for (k = 0; k < data->ncol; k++) {
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
	fprintf(fp, "%d %d\n", data->ntable, data->ncol);
	for (i = 0; i < data->ntable; i++) {
		for (j = 0; j < data->ncol; j++) {
			fprintf(fp, "%d ", data->table[i].types[j]);
		}
		fprintf(fp, "%.15e %d\n", data->table[i].energy, data->table[i].refcount);
	}
	fclose(fp);
}

void MP_KMCReadTable(MP_KMCData *data, char *filename)
{
	FILE *fp;
	int ntable, ncol;
	short types[13];
	double energy;
	long refcount;
	char *p;
	int i;

	if ((fp = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCReadTable)\n", filename);
		return;
	}
	fgets(data->htable, 256, fp);
	p = strchr(data->htable, '\n');
	if (p != NULL) *p = '\0';
	fscanf(fp, "%d %d\n", &ntable, &ncol);
	if (ncol != data->ncol) {
		fprintf(stderr, "Error : incompatible format, %s.(MP_KMCReadTable)\n", filename);
	}
	if (ncol == 13) {
		for (i = 0; i < ntable;i++) {
			fscanf(fp, "%hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %le %ld",
			&(types[0]), &(types[1]), &(types[2]), &(types[3]), &(types[4]),
			&(types[5]), &(types[6]), &(types[7]), &(types[8]), &(types[9]),
			&(types[10]), &(types[11]), &(types[12]), &energy, &refcount);
			MP_KMCAddCluster(data, types, energy, refcount);
		}
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
	gzprintf(gfp, "lat_type %d\n", data->lat_type);
	gzprintf(gfp, "size %d %d %d\n", data->size[0], data->size[1], data->size[2]);
	gzprintf(gfp, "solvent %d\n", data->types[0]);
	gzprintf(gfp, "nsolute_max %d\n", data->nsolute_max);
	gzprintf(gfp, "ntable_step %d\n", data->ntable_step);
	gzprintf(gfp, "nevent_step %d\n", data->nevent_step);
	gzprintf(gfp, "rand_seed %d\n", data->rand_seed);
	gzprintf(gfp, "step %d\n", data->step);
	gzprintf(gfp, "%s\n", data->htable);
	gzprintf(gfp, "ntable %d\n", data->ntable);
	for (i = 0; i < data->ntable; i++) {
		for (j = 0; j < data->ncol; j++) {
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

int MP_KMCRead(MP_KMCData *data, char *filename)
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

	if ((gfp = gzopen(filename, "rb")) == NULL) {
		fprintf(stderr, "Error : can't open %s.(MP_KMCRead)\n", filename);
		return FALSE;
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &lat_type);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d %d %d", dum, &nx, &ny, &nz);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %hd", dum, &solvent);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nsolute_max);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &ntable_step);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nevent_step);
	if (!MP_KMCAlloc(data, lat_type, nx, ny, nz, solvent, nsolute_max, ntable_step, nevent_step)) return FALSE;
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
