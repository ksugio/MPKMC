#include <MPKMC.h>
#include <MPGLKMC.h>

void GlutWindow(MP_KMCData *data, int width, int height, int argc, char **argv);

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
	return 0.0;
}

int main(int argc, char *argv[])
{
	MP_KMCData data;
	int njump, update;
	double T;	
	double uc[][3] = { { 0.0, 0.0, 0.0 }, { 0.5, 0.5, 0.0 }, { 0.5, 0.0, 0.5 }, { 0.0, 0.5, 0.5 } };
	short uc_types[] = { 29, 29, 29, 29 };
	double pv[][3] = { { 3.615, 0.0, 0.0 }, { 0.0, 3.615, 0.0 }, { 0.0, 0.0, 3.615 } };
	double cluster[][3] = { { 0, 0, 0 }, { 0.5, 0.5, 0 }, { 0, 0.5, -0.5 }, { -0.5, 0, -0.5 }, { -0.5, 0.5, 0 },
				{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
				{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 },
				{ 1.0, 0, 0 },{ -1.0, 0, 0 },{ 0, 1.0, 0 },{ 0, -1.0, 0 },{ 0, 0, 1.0 },{ 0, 0, -1.0}};
	int ngroup;

	MP_KMCAlloc(&data, 4, 10, 10, 10, 19, 1000, 1000, 100000, 100);
	MP_KMCSetUnitCell(&data, uc, uc_types, pv);
	MP_KMCSetCluster(&data, cluster, 13);
	MP_KMCCalcRotIndex(&data, 5.0, 1.0e-6);
	MP_KMCAddSoluteRandom(&data, 100, 0, TRUE);
	data.rand_seed = 543210;
//	data.table_use = FALSE;
	MP_KMCTotalEnergy(&data, calcFSFCC, &update);
	printf("%d %.15e\n", data.ntable, data.tote);
	for (T = 1100; T >= 1000; T -= 10) {
		njump = MP_KMCJump(&data, 10000, T, calcFSFCC, &update);
		MP_KMCAddResult(&data, T, 10000, njump);
		printf("%f %d %d %.15e\n", T, njump, data.ntable, data.tote);
//		MP_KMCSortTable(&data);
	}
	ngroup = MP_KMCFindSoluteGroup(&data, 0.71);
	fprintf(stderr, "ngroup %d\n", ngroup);
	GlutWindow(&data, 800, 600, argc, argv);
	MP_KMCWrite(&data, "test.mpkmc", 0);
	MP_KMCFree(&data);
}
