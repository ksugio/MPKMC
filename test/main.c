#include <MPKMC.h>
#include <MPGLKMC.h>

void GlutWindow(MP_KMCData *data, int width, int height, int argc, char **argv);

static double calcFSFCC(MP_KMCData* data, short types[])
{
	return MP_FSFCCEnergy(data->potp, data, types);
}

static void testFSFCC()
{
	MP_KMCData data;
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

	double fe, fa;

	MP_KMCAlloc(&data, 4, 10, 10, 20, 19, 1000, 1000, 100000, 100);
	MP_KMCSetUnitCell(&data, uc, uc_types, pv);
	MP_KMCSetCluster(&data, cluster, 19);
	MP_KMCCalcRotIndex(&data, 5.0, 1.0e-6);
	MP_KMCAddSoluteRandom(&data, 30, 0, TRUE);
	MP_FSFCCInit(&fsfcc);
	data.potp = &fsfcc;
	MP_KMCChangeTypeByPlane(&data, 1, 1, -1, 0.00001, 0);
	//MP_FSFCCGridEnergy(&fsfcc, &data);
	//for (T = 1100; T > 1000; T -= 10) {
		//ret = MP_FSFCCJump(&fsfcc, data, 10000, T);
	//	ret = MP_KMCJump(&data, 10000, T, calcFSFCC);
	//	printf("%f %d %d %.15e %f\n", T, ret.njump, ret.ntable, ret.tote, ret.time);
	//}
	//MP_KMCWriteTable(&data, "testFSFCC.etb");
	GlutWindow(&data, 640, 480, 0, NULL);
	MP_KMCFree(&data);
}

static int count13(MP_KMCData* data)
{
	int i;
	int count = 0;

	for (i = 0; i < data->ntot; i++) {
		if (data->grid[i].type == 13) count++;
	}
	return count;
}

static double calcMEAM(MP_KMCData *data, short types[])
{
	return MP_MEAMEnergy(data->potp, data, types);
}

static void testMEAM(MP_KMCData *data)
{
	MP_KMCData data1, data2;
	MP_MEAM meam;
	MP_KMCHistoryItem ret;
	double T;
	double uc[][3] = { { 0.0, 0.0, 0.0 },{ 0.5, 0.5, 0.0 },{ 0.5, 0.0, 0.5 },{ 0.0, 0.5, 0.5 } };
	short uc_types[] = { 13, 13, 13, 13 };
	double pv[][3] = { { 4.04466, 0.0, 0.0 },{ 0.0, 4.04466, 0.0 },{ 0.0, 0.0, 4.04466 } };
	double cluster[][3] = { { 0, 0, 0 },{ 0.5, 0.5, 0 },{ 0, 0.5, -0.5 },{ -0.5, 0, -0.5 },{ -0.5, 0.5, 0 },
	{ 0, 0.5, 0.5 },{ 0.5, 0, 0.5 },{ 0.5, -0.5, 0 },{ 0, -0.5, 0.5 },
	{ -0.5, 0, 0.5 },{ -0.5, -0.5, 0 },{ 0, -0.5, -0.5 },{ 0.5, 0, -0.5 } };

	int ids[MP_KMC_NCLUSTER_MAX];
	double fe;

	MP_KMCAlloc(data, 4, 10, 10, 10, 13, 1000, 1000, 100000, 100);
	data->table_use = FALSE;
	data->event_record = FALSE;
	MP_KMCSetUnitCell(data, uc, uc_types, pv);
	MP_KMCSetCluster(data, cluster, 13);
	MP_KMCCalcRotIndex(data, 5.0, 1.0e-6);
	//MP_KMCAddSoluteRandom(data, 20, 0, TRUE);
	MP_KMCAddSolute(data, 0, 0, TRUE);
	MP_MEAMInit(&meam);
	data->potp = &meam;
	

	fe = MP_KMCFractureEnergy(data, 0, 0, 1, 0, 0, 4.75, calcMEAM);
	fprintf(stderr, "%f\n", fe);
	//MP_MEAMGridEnergy(&meam, data);
	//fprintf(stderr, "0 %f\n", data->grid[0].energy);
	//fprintf(stderr, "%f\n", data->tote);
	//for (T = 800; T > 780; T -= 10) {
		//ret = MP_KMCJump(data, 10000, T, calcMEAM);
	//	ret = MP_MEAMJump(&meam, data, 10000, T);
	//	printf("%f %d %d %.15e %e %f\n", T, ret.njump, ret.ntable, ret.tote, ret.tott, ret.time);
	//}
	//printf("%e %e\n", data->tott, data->tott * 1.0e-13);
	//MP_KMCWriteTable(&data, "testMEAM.etb");
	//MP_KMCCopy(data, &data1, TRUE, TRUE);
	//MP_KMCSetVaccume(&data1, 0, 0, 1, 0, 0, 5.0);
	//MP_KMCSetVaccume(&data1, 0, 0, 1, 0, 0, 4.75);
	//MP_MEAMGridEnergy(&meam, &data1);
	//fprintf(stderr, "%f\n", data1.tote);
	//MP_KMCCopy(data, &data2, TRUE, TRUE);
	//MP_KMCSetVaccume(&data2, 0, 0, -1, 0, 0, 4.75);
	//MP_MEAMGridEnergy(&meam, &data2);
	//fprintf(stderr, "%f\n", data2.tote);
	//fprintf(stderr, "%f\n", data1.tote + data2.tote - data->tote);
	//MP_KMCTranslate(data, 0, 5, 5, 5);
	//fprintf(stderr, "%d %d\n", count13(&data1), count13(&data2));
	//MP_MEAMGridEnergy(&meam, data);
	GlutWindow(data, 640, 480, 0, NULL);
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
	//MP_KMCReadTable(data, "../python/Al-Si-V.etb");
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
	//MP_KMCData data;

	//testMEAM(&data);
	testFSFCC();
}
