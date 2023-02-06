#include <MPKMC.h>
//#include <MPGLKMC.h>

//void GlutWindow(MP_KMCData *data, int width, int height, int argc, char **argv);

static void testFSFCC(MP_KMCData *data)
{
	MP_FSFCC fsfcc;
	MP_KMCHistoryItem ret;
	double T;
	double uc[][3] = { { 0.0, 0.0, 0.0 }, { 0.5, 0.5, 0.0 }, { 0.5, 0.0, 0.5 }, { 0.0, 0.5, 0.5 } };
	short uc_types[] = { 29, 29, 29, 29 };
	double pv[][3] = { { 3.615, 0.0, 0.0 }, { 0.0, 3.615, 0.0 }, { 0.0, 0.0, 3.615 } };
	double cluster[][3] = { { 0, 0, 0 }, { 0.5, 0.5, 0 }, { 0, 0.5, -0.5 }, { -0.5, 0, -0.5 }, { -0.5, 0.5, 0 },
	{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
	{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 },
	{ 1.0, 0, 0 },{ -1.0, 0, 0 },{ 0, 1.0, 0 },{ 0, -1.0, 0 },{ 0, 0, 1.0 },{ 0, 0, -1.0} };

	MP_KMCAlloc(data, 4, 10, 10, 10, 19, 1000, 1000, 100000, 100);
	MP_KMCSetUnitCell(data, uc, uc_types, pv);
	MP_KMCSetCluster(data, cluster, 13);
	MP_KMCCalcRotIndex(data, 5.0, 1.0e-6);
	MP_KMCAddSoluteRandom(data, 30, 0, TRUE);
	MP_FSFCCInit(&fsfcc);
	MP_FSFCCGridEnergy(&fsfcc, data);
	for (T = 1100; T > 1000; T -= 10) {
		ret = MP_FSFCCJump(&fsfcc, data, 10000, T);
		printf("%f %d %d %.15e %f\n", T, ret.njump, ret.ntable, ret.tote, ret.time);
	}
	//MP_KMCWriteTable(&data, "testFSFCC.etb");
	MP_KMCFree(data);
}

static void testMEAM(MP_KMCData *data)
{
	MP_MEAM meam;
	MP_KMCHistoryItem ret;
	double T;
	double uc[][3] = { { 0.0, 0.0, 0.0 },{ 0.5, 0.5, 0.0 },{ 0.5, 0.0, 0.5 },{ 0.0, 0.5, 0.5 } };
	short uc_types[] = { 13, 13, 13, 13 };
	double pv[][3] = { { 4.04466, 0.0, 0.0 },{ 0.0, 4.04466, 0.0 },{ 0.0, 0.0, 4.04466 } };
	double cluster[][3] = { { 0, 0, 0 },{ 0.5, 0.5, 0 },{ 0, 0.5, -0.5 },{ -0.5, 0, -0.5 },{ -0.5, 0.5, 0 },
	{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
	{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 } };

	MP_KMCAlloc(data, 4, 10, 10, 10, 13, 1000, 1000, 100000, 100);
	data->table_use = FALSE;
	data->event_record = FALSE;
	MP_KMCSetUnitCell(data, uc, uc_types, pv);
	MP_KMCSetCluster(data, cluster, 13);
	MP_KMCCalcRotIndex(data, 5.0, 1.0e-6);
	MP_KMCAddSoluteRandom(data, 20, 0, TRUE);
	MP_MEAMInit(&meam);
	//MP_KMCGridEnergy(data, calcMEAM);
	MP_MEAMGridEnergy(&meam, data);
	for (T = 800; T > 600; T -= 10) {
		//ret = MP_KMCJump(data, 10000, T, calcMEAM);
		ret = MP_MEAMJump(&meam, data, 10000, T);
		printf("%f %d %d %.15e %e %f\n", T, ret.njump, ret.ntable, ret.tote, ret.tott, ret.time);
	}
	printf("%e %e\n", data->tott, data->tott * 1.0e-13);
	//MP_KMCWriteTable(&data, "testMEAM.etb");
	MP_KMCFree(data);
}

static void testQE(MP_KMCData* data)
{
	MP_KMCHistoryItem ret;
	double T;
	double uc[][3] = { { 0.0, 0.0, 0.0 }, { 0.5, 0.5, 0.0 }, { 0.5, 0.0, 0.5 }, { 0.0, 0.5, 0.5 } };
	short uc_types[] = { 13, 13, 13, 13 };
	double pv[][3] = { { 4.04360, 0.0, 0.0 }, { 0.0, 4.04360, 0.0 }, { 0.0, 0.0, 4.04360 } };
	double cluster[][3] = { { 0, 0, 0 }, { 0.5, 0.5, 0 }, { 0, 0.5, -0.5 }, { -0.5, 0, -0.5 }, { -0.5, 0.5, 0 },
	 			{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
				{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 } };
	double Kb = 86.1735e-6;
	double Ry = 13.6058;

	MP_KMCAlloc(data, 4, 10, 10, 10, 13, 1000, 1000, 100000, 100);
	data->kb = Kb / Ry;
	MP_KMCSetUnitCell(data, uc, uc_types, pv);
	MP_KMCSetCluster(data, cluster, 13);
	MP_KMCCalcRotIndex(data, 5.0, 1.0e-6);
	MP_KMCReadTable(data, "../python/Al-Si-V.etb");
	printf("Number of Table: %d\n", data->ntable);
	MP_KMCSetMigration(data, 13, 14, 0.66 / Ry);
	MP_KMCAddSoluteRandom(data, 40, 14, 1);
	MP_KMCListTransition(data, NULL);
	for (T = 800; T >= 700; T -= 10) {
		ret = MP_KMCJump2(data, 10000, T, NULL);
		printf("%f %d %d %e %e\n", T, ret.njump, ret.ntable, ret.tote, ret.tott);
	}
	MP_KMCWrite(data, "test.mpkmc", 0);
	MP_KMCRead(data, "test.mpkmc", 3);
	//MP_KMCWriteTable(&data, "testQE.etb");
	MP_KMCFree(data);
}

int main(int argc, char *argv[])
{
	MP_KMCData data;

	testQE(&data);
}
