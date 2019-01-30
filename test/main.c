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
}

int main(int argc, char *argv[])
{
	MP_KMCData data;
	int njump, update;
	double Kb = 86.1735e-6; // ev/K
	double T;	
	double uc[][3] = { { 0.0, 0.0, 0.0 }, { 0.5, 0.5, 0.0 }, { 0.5, 0.0, 0.5 }, { 0.0, 0.5, 0.5 } };
	short uc_types[] = { 29, 29, 29, 29 };
	double pv[][3] = { { 3.615, 0.0, 0.0 }, { 0.0, 3.615, 0.0 }, { 0.0, 0.0, 3.615 } };
	double cluster[][3] = { { 0, 0, 0 }, { 0.5, 0.5, 0 }, { 0, 0.5, -0.5 }, { -0.5, 0, -0.5 }, { -0.5, 0.5, 0 },
				{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
				{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 },
				{ 1.0, 0, 0 },{ -1.0, 0, 0 },{ 0, 1.0, 0 },{ 0, -1.0, 0 },{ 0, 0, 1.0 },{ 0, 0, -1.0}};
	short jcluster[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };

	MP_KMCAlloc(&data, 4, 10, 10, 10, 19, 100, 1000, 10000);
	MP_KMCSetUnitCell(&data, uc, uc_types, pv);
	MP_KMCSetCluster(&data, cluster, jcluster);
	MP_KMCCalcRotIndex(&data, 5.0, 1.0e-6);
	MP_KMCAddSoluteRandom(&data, 30, 0, TRUE);
	MP_KMCTotalEnergy(&data, calcFSFCC, &update);
	printf("%d %.15e\n", data.ntable, data.tote);
	for (T = 1100; T >= 1000; T -= 10) {
		njump = MP_KMCJump(&data, 10, Kb*T, calcFSFCC, &update);
		printf("%f %d %d %.15e %d %d\n", T, njump, data.ntable, data.tote,
			MP_KMCCountType(&data, 0), MP_KMCCheckSolute(&data));
	}
	GlutWindow(&data, 800, 600, argc, argv);
	MP_KMCWrite(&data, "test.mpkmc", 0);
	MP_KMCFree(&data);
}
