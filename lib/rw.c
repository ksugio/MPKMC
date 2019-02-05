#include "MPKMC.h"

int KMCAddResult(MP_KMCData *data, long mcs, double temp, int ntry, int njump, double fjump, double tote);
int KMCAddEvent(MP_KMCData *data, int dp, int id0, int id1, double de, int dmcs);

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
	for (i = 0; i < ntable; i++) {
		fgets(buf, 256, fp);
		ScanTable(buf, ncluster, types, &energy, &refcount);
		MP_KMCAddCluster(data, types, energy, refcount);
	}
	return TRUE;
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
		gzprintf(gfp, "%.15e %.15e %.15e\n", data->cluster[i][0], data->cluster[i][1], data->cluster[i][2]);
	}
	gzprintf(gfp, "cpmax %d\n", data->cpmax);
	gzprintf(gfp, "nsolute_step %d\n", data->nsolute_step);
	gzprintf(gfp, "ntable_step %d\n", data->ntable_step);
	gzprintf(gfp, "nevent_step %d\n", data->nevent_step);
	gzprintf(gfp, "nresult_step %d\n", data->nresult_step);
	gzprintf(gfp, "nrot %d\n", data->nrot);
	for (i = 0; i < data->nrot; i++) {
		sp = i * data->ncluster;
		for (j = 0; j < data->ncluster; j++) {
			gzprintf(gfp, "%d ", data->rotid[sp + j]);
		}
		gzprintf(gfp, "\n");
	}
	gzprintf(gfp, "rand_seed %d\n", data->rand_seed);
	gzprintf(gfp, "totmcs %d\n", data->totmcs);
	gzprintf(gfp, "mcs %d\n", data->mcs);
	gzprintf(gfp, "tote %.15e\n", data->tote);
	gzprintf(gfp, "kb %.15e\n", data->kb);
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
		gzprintf(gfp, "%d %d %d %.15e %d\n", data->event[i].dp, data->event[i].id0, data->event[i].id1, data->event[i].de, data->event[i].dmcs);
	}
	gzprintf(gfp, "nresult %d\n", data->nresult);
	for (i = 0; i < data->nresult; i++) {
		gzprintf(gfp, "%d %.15e %d %d %.15e %.15e\n",  data->result[i].totmcs, data->result[i].temp,
			data->result[i].ntry, data->result[i].njump, data->result[i].fjump, data->result[i].tote);
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
	int cpmax;
	int nsolute_step;
	int ntable_step;
	int nevent_step;
	int ntemp_step;
	int nrot;
	int ids[MP_KMC_NCLUSTER_MAX];
	int ntable;
	short types[MP_KMC_NCLUSTER_MAX];
	double energy;
	long refcount;
	int nsolute;
	int nevent;
	int nresult;
	int id;
	short type, jump;
	int sid;
	int njump;
	int dp, id0, id1;
	double de;
	int dmcs;
	double temp, fjump, tote;
	long mcs;
	int ntry;

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
		sscanf(buf, "%le %le %le", &(cluster[i][0]), &(cluster[i][1]), &(cluster[i][2]));
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &cpmax);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nsolute_step);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &ntable_step);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nevent_step);
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &ntemp_step);
	if (!MP_KMCAlloc(data, nuc, nx, ny, nz, ncluster, nsolute_step, ntable_step, nevent_step, ntemp_step)) return FALSE;
	MP_KMCSetUnitCell(data, uc, uc_types, pv);
	MP_KMCSetCluster(data, cluster, cpmax);
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
	sscanf(buf, "%s %ld", dum, &(data->totmcs));
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %ld", dum, &(data->mcs));
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %le", dum, &(data->tote));
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %le", dum, &(data->kb));
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
		if (MP_KMCAddCluster(data, types, energy, refcount) < 0) return FALSE;
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nsolute);
	for (i = 0; i < nsolute; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%d %hd %hd %d", &id, &type, &jump, &njump);
		sid = MP_KMCAddSolute(data, id, type, jump);
		if (sid < 0) {
			fprintf(stderr, "Error : overlapped ID, %d.(MP_KMCRead)\n", id);
			return FALSE;
		}
		data->solute[sid].njump = njump;
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nevent);
	for (i = 0; i < nevent; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%d %d %d %le %d", &dp, &id0, &id1, &de, &dmcs);
		if (KMCAddEvent(data, dp, id0, id1, de, dmcs) < 0) return FALSE;
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nresult);
	for (i = 0; i < nresult; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%ld %le %d %d %le %le", &mcs, &temp, &ntry, &njump, &fjump, &tote);
		if (KMCAddResult(data, mcs, temp, ntry, njump, fjump, tote) < 0) return FALSE;
	}
	gzclose(gfp);
	return TRUE;
}

static int Modid0to1(MP_KMCData *data, int nx, int ny, int id)
{
	int i;
	int a, b;
	int x, y, z;
	int uc[][3] = { { 0, 0, 0 },{ 1, 1, 0 },{ 1, 0, 1 },{ 0, 1, 1 } };

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
	double uc[][3] = { { 0.0, 0.0, 0.0 },{ 0.5, 0.5, 0.0 },{ 0.5, 0.0, 0.5 },{ 0.0, 0.5, 0.5 } };
	short uc_types[4];
	double pv[][3] = { { 1.0, 0.0, 0.0 },{ 0.0, 1.0, 0.0 },{ 0.0, 0.0, 1.0 } };
	double cluster[][3] = { { 0, 0, 0 },{ 0.5, 0.5, 0 },{ 0, 0.5, -0.5 },{ -0.5, 0, -0.5 },{ -0.5, 0.5, 0 },
	{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
	{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 } };
	int rotid[][13] = { { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 },{ 0, 6, 5, 4, 9, 8, 7, 12, 11, 10, 3, 2, 1 },
	{ 0, 7, 8, 9, 10, 11, 12, 1, 2, 3, 4, 5, 6 },{ 0, 12, 11, 10, 3, 2, 1, 6, 5, 4, 9, 8, 7 },
	{ 0, 4, 3, 11, 10, 9, 5, 1, 6, 8, 7, 12, 2 },{ 0, 9, 4, 2, 3, 10, 8, 6, 7, 11, 12, 1, 5 },
	{ 0, 10, 9, 5, 4, 3, 11, 7, 12, 2, 1, 6, 8 },{ 0, 3, 10, 8, 9, 4, 2, 12, 1, 5, 6, 7, 11 },
	{ 0, 10, 11, 12, 7, 8, 9, 4, 5, 6, 1, 2, 3 },{ 0, 3, 2, 1, 12, 11, 10, 9, 8, 7, 6, 5, 4 },
	{ 0, 4, 5, 6, 1, 2, 3, 10, 11, 12, 7, 8, 9 },{ 0, 9, 8, 7, 6, 5, 4, 3, 2, 1, 12, 11, 10 },
	{ 0, 7, 12, 2, 1, 6, 8, 10, 9, 5, 4, 3, 11 },{ 0, 12, 1, 5, 6, 7, 11, 3, 10, 8, 9, 4, 2 },
	{ 0, 1, 6, 8, 7, 12, 2, 4, 3, 11, 10, 9, 5 },{ 0, 6, 7, 11, 12, 1, 5, 9, 4, 2, 3, 10, 8 },
	{ 0, 5, 1, 12, 2, 4, 9, 8, 10, 3, 11, 7, 6 },{ 0, 8, 6, 1, 5, 9, 10, 11, 3, 4, 2, 12, 7 },
	{ 0, 11, 7, 6, 8, 10, 3, 2, 4, 9, 5, 1, 12 },{ 0, 2, 12, 7, 11, 3, 4, 5, 9, 10, 8, 6, 1 },
	{ 0, 2, 4, 9, 5, 1, 12, 11, 7, 6, 8, 10, 3 },{ 0, 5, 9, 10, 8, 6, 1, 2, 12, 7, 11, 3, 4 },
	{ 0, 8, 10, 3, 11, 7, 6, 5, 1, 12, 2, 4, 9 },{ 0, 11, 3, 4, 2, 12, 7, 8, 6, 1, 5, 9, 10 } };

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
	if (!MP_KMCAlloc(data, 4, nx / 2, ny / 2, nz / 2, 13, nsolute_max, ntable_step, nevent_step, 100)) return FALSE;
	MP_KMCSetUnitCell(data, uc, uc_types, pv);
	MP_KMCSetCluster(data, cluster, 13);
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
			if (MP_KMCAddCluster(data, types, energy, refcount) < 0) return FALSE;
		}
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nsolute);
	for (i = 0; i < nsolute; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%d %hd %hd", &id, &type, &jump);
		if (MP_KMCAddSolute(data, Modid0to1(data, nx, ny, id), type, jump) < 0) return FALSE;
	}
	gzgets(gfp, buf, 256);
	sscanf(buf, "%s %d", dum, &nevent);
	for (i = 0; i < nevent; i++) {
		gzgets(gfp, buf, 256);
		sscanf(buf, "%d %d %d", &dp, &id0, &id1);
		if (KMCAddEvent(data, dp, Modid0to1(data, nx, ny, id0), Modid0to1(data, nx, ny, id1), 0.0, 0) < 0) return FALSE;
	}
	gzclose(gfp);
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
