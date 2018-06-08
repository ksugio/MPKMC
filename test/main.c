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

static double calcEnergy(MP_KMCData *data, short types[])
{
	int i;
	int count = 0;
	double energy[] = { -52.3, -55.9, -59.6, -63.3, -67.0, -70.7, -74.4, -78.1, -81.7, -85.4, -89.1, -92.8, -96.4, -100.0 };

	for (i = 0; i < data->ncluster; i++) {
		if (types[i] == 14) count++;
		printf("%d ", types[i]);
	}
	printf("\n");
	return energy[count];
}

int main(int argc, char *argv[])
{
	int i;
	//int p, x, y, z;
	//char etbfile[] = {"../python/Al-Si.etb"};
	int update;
	MP_KMCData data;
	MP_FSFCCParm parm;
	//double tote0, tote1;
	//int ret;
	//double Kb = 86.1735e-6; // ev/K
	//double Ry = 13.6058; // ev
	//double Kbry = Kb*Ry;
	//double T = 0.1;
	double ene;
	short types[] = { 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29 };
	short types1[] = { 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 0 };
	short types2[] = { 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 0, 0 };

	double uc[][3] = { {0.0, 0.0, 0.0}, {0.5, 0.5, 0.0}, {0.5, 0.0, 0.5}, {0.0, 0.5, 0.5} };
	double cluster[][3] = {{ 0, 0, 0 }, { 0.5, 0.5, 0 }, { 0, 0.5, -0.5 }, { -0.5, 0, -0.5 }, { -0.5, 0.5, 0 },
				{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
				{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 }};

	//MP_KMCRead(&data, "Al-V1.kmc");
	MP_KMCAlloc(&data, 4, 2, 2, 2, 13, 1000, 1000, 1000);
	MP_KMCSetUnitCell(&data, uc);
	MP_KMCSetCluster(&data, cluster);
	MP_KMCCalcRotIndex(&data, 5.0);
	MP_KMCSetSolvent(&data, 13);

	MP_FSFCCCu(&parm);
	ene = MP_FSFCCEnergy(&parm, &data, types);
	printf("%f\n", ene);

	ene = MP_FSFCCEnergy(&parm, &data, types1);
	printf("%f\n", ene);

	ene = MP_FSFCCEnergy(&parm, &data, types2);
	printf("%f\n", ene);
//	MP_KMCAddSoluteRandom(&data, 3, 0, TRUE);
//	for (i = 0; i < data.ntot; i++) {
//		MP_KMCCalcEnergy(&data, i, calcFS, &update);
//	}
//	MP_KMCWriteTable(&data, "test.etb");
	//tote0 = MP_KMCTotalEnergy(&data);
	//printf("tote0 = %f\n", tote0);
	//for (i = 0; i < 1000;i++) {
	//	ret = MP_KMCJump(&data, Kbry*T, calcEnergy, &update);
	//	if (ret) {
	//		tote1 = MP_KMCTotalEnergy(&data);
	//		printf("diff = %f\n", tote1 - tote0);
	//	}
	//}
	//printf("tote1 = %f\n", tote1);
//	for (i = 0; i < data.nevent; i++) {
//		printf("%d, %d, %d\n", data.event[i].dp, data.event[i].id0, data.event[i].id1);
//	}
//	MP_KMCStepGo(&data, 0);
//	MP_KMCStepGo(&data, 2);
//	MP_KMCStepGo(&data, 6);
//	MP_KMCStepBackward(&data, 2);
//	MP_KMCStepForward(&data, 2);
//	MP_KMCStepForward(&data);
//	MP_KMCRead(&data, "test.mpkmc");
//	MP_KMCWrite(&data, "test2.mpkmc", 0);
	//GLWindow(&data, argc, argv);
	//MP_KMCWriteTable(&data, "test.etb");
	MP_KMCFree(&data);
}
