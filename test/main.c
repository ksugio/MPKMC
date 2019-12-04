#include <MPKMC.h>
#include <MPGLKMC.h>

void GlutWindow(MP_KMCData *data, int width, int height, int argc, char **argv);

static double calcFSFCC(MP_KMCData *data, short types[])
{
	MP_FSFCC fsfcc;

	MP_FSFCCInit(&fsfcc);
	return MP_FSFCCEnergy(&fsfcc, data, types);
}

static void testFSFCC(MP_KMCData *data)
{
	MP_KMCHistoryItem ret;
	double T;
	double uc[][3] = { { 0.0, 0.0, 0.0 }, { 0.5, 0.5, 0.0 }, { 0.5, 0.0, 0.5 }, { 0.0, 0.5, 0.5 } };
	short uc_types[] = { 29, 29, 29, 29 };
	double pv[][3] = { { 3.615, 0.0, 0.0 }, { 0.0, 3.615, 0.0 }, { 0.0, 0.0, 3.615 } };
	double cluster[][3] = { { 0, 0, 0 }, { 0.5, 0.5, 0 }, { 0, 0.5, -0.5 }, { -0.5, 0, -0.5 }, { -0.5, 0.5, 0 },
	{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
	{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 },
	{ 1.0, 0, 0 },{ -1.0, 0, 0 },{ 0, 1.0, 0 },{ 0, -1.0, 0 },{ 0, 0, 1.0 },{ 0, 0, -1.0}};

	MP_KMCAlloc(data, 4, 10, 10, 10, 19, 1000, 1000, 100000, 100);
	data->rand_seed = 12345;
	MP_KMCSetUnitCell(data, uc, uc_types, pv);
	MP_KMCSetCluster(data, cluster, 13);
	MP_KMCCalcRotIndex(data, 5.0, 1.0e-6);
	MP_KMCAddSoluteRandom(data, 30, 0, TRUE);
	MP_KMCGridEnergy(data, calcFSFCC);
	for (T = 1100; T > 1000; T -= 10) {
		ret = MP_KMCJump(data, 1000, T, calcFSFCC);
		printf("%f %d %d %.15e %f\n", T, ret.njump, ret.ntable, ret.tote, ret.time);
	}
}

static double calcMEAM(MP_KMCData *data, short types[])
{
	MP_MEAM meam;

	MP_MEAMInit(&meam);
	return MP_MEAMEnergy(&meam, data, types);
}

static void testMEAM(MP_KMCData *data)
{
	MP_KMCHistoryItem ret;
	double T;
	double uc[][3] = { { 0.0, 0.0, 0.0 },{ 0.5, 0.5, 0.0 },{ 0.5, 0.0, 0.5 },{ 0.0, 0.5, 0.5 } };
	short uc_types[] = { 13, 13, 13, 13 };
	double pv[][3] = { { 4.04466, 0.0, 0.0 },{ 0.0, 4.04466, 0.0 },{ 0.0, 0.0, 4.04466 } };
	double cluster[][3] = { { 0, 0, 0 },{ 0.5, 0.5, 0 },{ 0, 0.5, -0.5 },{ -0.5, 0, -0.5 },{ -0.5, 0.5, 0 },
	{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
	{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 } };

	MP_KMCAlloc(data, 4, 10, 10, 10, 13, 1000, 1000, 100000, 100);
	MP_KMCSetUnitCell(data, uc, uc_types, pv);
	MP_KMCSetCluster(data, cluster, 13);
	MP_KMCCalcRotIndex(data, 5.0, 1.0e-6);
	MP_KMCAddSoluteRandom(data, 20, 0, TRUE);
	MP_KMCGridEnergy(data, calcMEAM);
	for (T = 800; T > 600; T -= 10) {
		ret = MP_KMCJump(data, 10000, T, calcMEAM);
		printf("%f %d %d %.15e %f\n", T, ret.njump, ret.ntable, ret.tote, ret.time);
	}
}

int main(int argc, char *argv[])
{
//	int i;
//	MP_MEAM meam;

//	MP_MEAMInit(&meam);
//	for (i = 0; i < meam.nparm; i++) {
//		fprintf(stderr, "%d %f %f\n", meam.parm[i].type, meam.parm[i].E0i, meam.parm[i].R0i);
//	}
//	fprintf(stderr, "j\n");

	MP_KMCData data;

	testMEAM(&data);
	//testFSFCC(&data);


	
//	double uc[][3] = { { 0.0, 0.0, 0.0 }, { 0.5, 0.5, 0.0 }, { 0.5, 0.0, 0.5 }, { 0.0, 0.5, 0.5 } };
//	short uc_types[] = { 29, 29, 29, 29 };
//	double pv[][3] = { { 3.615, 0.0, 0.0 }, { 0.0, 3.615, 0.0 }, { 0.0, 0.0, 3.615 } };
//	double cluster[][3] = { { 0, 0, 0 }, { 0.5, 0.5, 0 }, { 0, 0.5, -0.5 }, { -0.5, 0, -0.5 }, { -0.5, 0.5, 0 },
//				{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
//				{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 },
//				{ 1.0, 0, 0 },{ -1.0, 0, 0 },{ 0, 1.0, 0 },{ 0, -1.0, 0 },{ 0, 0, 1.0 },{ 0, 0, -1.0}};
//	int ntypes;
//	int i;
//	short types[100];
//	double uc[][3] = { {0, 0, 0}, {2.0 / 3.0, 1.0 / 3.0, 0.5} };
//	short uc_types[] = { 22, 22 };
//	double pv[][3] = { {1.0, 0, 0}, {-0.5, 0.866025403, 0.0}, {0, 0, 1.0} };
//	double msd;

//	MP_KMCAlloc(&data, 4, 10, 10, 10, 19, 1000, 1000, 100000, 100);
//	MP_KMCSetUnitCell(&data, uc, uc_types, pv);
//	MP_KMCSetCluster(&data, cluster, 19);
//	MP_KMCCalcRotIndex(&data, 5.0, 1.0e-6);
//	MP_KMCAddSoluteRandom(&data, 40, 0, TRUE);
//	MP_KMCAddSoluteRandom(&data, 40, 10, TRUE);
//	MP_KMCAddSoluteRandom(&data, 40, 20, TRUE);
//	data.rand_seed = 543210;
//	data.save_grid = TRUE;
//	ntypes = MP_KMCSoluteTypes(&data, 100, types);
//	for (i = 0; i < ntypes; i++) {
//		fprintf(stderr, "%d\n", types[i]);
//	}
//	data.table_use = FALSE;
//	MP_KMCGridEnergy(&data, calcFSFCC);
//	for (i = 0; i < data.ntot; i++) {
//		fprintf(stderr, "%f\n", data.grid[i].energy);
//	}
//	printf("%d %.15e\n", data.ntable, data.tote);
//	for (T = 1100; T >= 1000; T -= 10) {
//		ret = MP_KMCJump(&data, 1000, T, calcFSFCC);
//		printf("%f %d %d %.15e %f\n", T, ret.njump, ret.ntable, ret.tote, ret.time);
//		MP_KMCSortTable(&data);
//	}
	GlutWindow(&data, 800, 600, argc, argv);
//	MP_KMCWrite(&data, "test.mpkmc", 0);
//	MP_KMCRead(&data, "..\python\fsfcc.mpkmc", 1);
	MP_KMCFree(&data);
}
