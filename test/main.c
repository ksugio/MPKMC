#include <MPKMC.h>
#include <MPGLKMC.h>

static const char DefaultIn[] = {
"&control\n\
 calculation = 'scf'\n\
 prefix='Al'\n\
/\n\
&system\n\
 ibrav = 1\n\
 celldm(1) = 15.42017132\n\
 nat = 13\n\
 ntyp = 2\n\
 ecutwfc = 30.0\n\
 ecutrho = 150.0\n\
 occupations = 'smearing'\n\
 smearing = 'm-p'\n\
 degauss = 0.001\n\
 assume_isolated = 'mp'\n\
/\n\
&electrons\n\
 mixing_beta = 0.7\n\
 conv_thr = 1.0d-8\n\
/\n\
ATOMIC_SPECIES\n\
 Al 26.98 Al.blyp-hgh.UPF\n\
 Si 28.086 Si.blyp-hgh.UPF\n\
ATOMIC_POSITIONS {alat}\n"};

static void writeIN(MP_KMCData *data, int tp)
{
	FILE *fp;
	int i;
	double x, y, z;
	double fcc_pos[][3] = {
		{ 0.0, 0.0, 0.0 },{ 0.5, 0.5, 0.0 },{ 0.0, 0.5, -0.5 },{ -0.5, 0.0, -0.5 },{ -0.5, 0.5, 0.0 },
		{ 0.0, 0.5, 0.5 },{ 0.5, 0.0, 0.5 },{ 0.5, -0.5, 0.0 },{ 0.0, -0.5, 0.5 },
		{ -0.5, 0.0, 0.5 } ,{ -0.5, -0.5, 0.0 },{ 0.0, -0.5, -0.5 },{ 0.5, 0.0, -0.5 } };

	fp = fopen("tmp.in", "w");
	fprintf(fp, "%s", DefaultIn);
	for (i = 0;i < 13;i++) {
		x = fcc_pos[i][0]/2.0 +0.5;
		y = fcc_pos[i][1]/2.0 +0.5;
		z = fcc_pos[i][2]/2.0 +0.5;
		if (data->table[tp].types[i] == 13) fprintf(fp, " Al %3.2f %3.2f %3.2f\n", x, y, z);
		else if (data->table[tp].types[i] == 14) fprintf(fp, " Si %3.2f %3.2f %3.2f\n", x, y, z);
	}
	fprintf(fp, "K_POINTS {automatic}\n 2 2 2 0 0 0\n");
	fclose(fp);
}

static void readOUT(MP_KMCData *data, int tp, char *filename)
{
	FILE *fp;
	char s[256];
	char t[6][256];

	fp = fopen(filename, "r");
	while	(fgets(s, 256, fp) != NULL) {
		if (strstr(s, "!    total energy              =") != NULL) {
			sscanf(s, "%s %s %s %s %s %s", t[0], t[1], t[2], t[3], t[4], t[5]);
			data->table[tp].energy = atof(t[4]);
		}
	}
}

static void calcQE(MP_KMCData *data, int nnew, char *etbfile)
{
	int tp = data->ntable - nnew;
	char out[256], com[256];

	fprintf(stderr, "Number of new cluster = %d\n", nnew);
	while (tp < data->ntable) {
		writeIN(data, tp);
		sprintf(out, "tb%05d.out", tp);
		sprintf(com, "./pw.x < tmp.in > %s", out);
		fprintf(stderr, "%s\n", com);
		system(com);
		readOUT(data, tp, out);
		tp++;
	}
	MP_KMCWriteTable(data, etbfile);
}

static double calcFSFCC(MP_KMCData *data, short types[])
{
	MP_FSFCCParm parm;

	if (MP_FSFCCInit(&parm, types[0])) {
		return MP_FSFCCEnergy(&parm, data, types);
	}
	else {
		fprintf(stderr, "Error : Can't find type, %d\n", types[0]);
		return 0.0;
	}
}

int main(int argc, char *argv[])
{
//	int i;
	//int p, x, y, z;
	//char etbfile[] = {"../python/Al-Si.etb"};
	int update;
	MP_KMCData data;
	//int njump;
	double Kb = 86.1735e-6; // ev/K
	//double Ry = 13.6058; // ev
	//double Kbry = Kb*Ry;
	double T = 500.0;
	//double ehist[100];
	double ene;
	double uc[][3] = { { 0.0, 0.0, 0.0 }, { 0.5, 0.5, 0.0 }, { 0.5, 0.0, 0.5 }, { 0.0, 0.5, 0.5 } };
	short uc_types[] = { 29, 29, 29, 29 };
	double pv[][3] = { { 3.615, 0.0, 0.0 }, { 0.0, 3.615, 0.0 }, { 0.0, 0.0, 3.615 } };
	double cluster[][3] = {{ 0, 0, 0 }, { 0.5, 0.5, 0 }, { 0, 0.5, -0.5 }, { -0.5, 0, -0.5 }, { -0.5, 0.5, 0 },
				{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
				{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 },
				{ 1.0, 0, 0 },{ -1.0, 0, 0 },{ 0, 1.0, 0 },{ 0, -1.0, 0 },{ 0, 0, 1.0 },{ 0, 0, -1.0}};

	//MP_KMCRead(&data, "Al-V1.kmc");
	MP_KMCAlloc(&data, 4, 2, 2, 2, 19, 100, 1000, 10000);
	data.rand_seed = 12345;
	data.table_use = TRUE;
	MP_KMCSetUnitCell(&data, uc, uc_types, pv);
	MP_KMCSetCluster(&data, cluster);
	MP_KMCCalcRotIndex(&data, 5.0, 1.0e-6);
	MP_KMCAddSoluteRandom(&data, 3, 0, TRUE);
	ene = MP_KMCTotalEnergy(&data, calcFSFCC, &update);
	printf("%d %.15e\n", data.ntable, ene);
	MP_KMCJump(&data, 10000, Kb*T, calcFSFCC, &update);
	printf("%d %.15e\n", data.ntable, data.tote);
//	MP_KMCTotalEnergy(&data, calcFSFCC, &update);
//	printf("f %f %d\n", data.tote, update);
//	MP_KMCStepBackward(&data, 9);
//	MP_KMCStepGo(&data, 0);
//	printf("cur %d %f\n", data.step, data.tote);
//	MP_KMCEnergyHistory(&data, 100, ehist);
//	for (i = 0; i <= data.nevent; i++) {
//		printf("%d %f\n", i, ehist[i]);
//	}
	MP_KMCWrite(&data, "test.mpkmc", 0);
//	MP_KMCWriteTable(&data, "test.etb");
	MP_KMCFree(&data);
}
