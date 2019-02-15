#include "MPKMC.h"

static MP_MEAMParm MEAMParmTable[] = {
	{29, 3.540, 2.56, 5.22, 1.07, {3.63, 2.2, 6.0, 2.2}, {1, 3.14, 2.49, 2.95}},
	{47, 2.850, 2.88, 5.89, 1.06, {4.46, 2.2, 6.0, 2.2}, {1, 5.54, 2.45, 1.29}},
	{79, 3.930, 2.88, 6.34, 1.04, {5.45, 2.2, 6.0, 2.2}, {1, 1.59, 1.51, 2.61}},
	{28, 4.450, 2.49, 4.99, 1.10, {2.45, 2.2, 6.0, 2.2}, {1, 3.57, 1.60, 3.70}},
	{46, 3.910, 2.75, 6.43, 1.01, {4.98, 2.2, 6.0, 2.2}, {1, 2.34, 1.38, 4.48}},
	{78, 5.770, 2.77, 6.44, 1.04, {4.67, 2.2, 6.0, 2.2}, {1, 2.73, -1.38, 3.29}},
	{13, 3.580, 2.86, 4.61, 1.07, {2.21, 2.2, 6.0, 2.2}, {1, -1.78, -2.21, 8.01}},
	{82, 2.040, 3.50, 6.06, 1.01, {5.31, 2.2, 6.0, 2.2}, {1, 2.74, 3.06, 1.20}},
	{45, 5.750, 2.69, 6.00, 1.05, {1.13, 1.0, 2.0, 1.0}, {1, 2.99, 4.61, 4.80}},
	{77, 6.930, 2.72, 6.52, 1.05, {1.13, 1.0, 2.0, 1.0}, {1, 1.50, 8.10, 4.80}},
	{3, 1.650, 3.04, 2.97, 0.87, {1.43, 1.0, 1.0, 1.0}, {1, 0.26, 0.44, -0.20}},
	{11, 1.130, 3.72, 3.64, 0.90, {2.31, 1.0, 1.0, 1.0}, {1, 3.55, 0.69, -0.20}},
	{19, 0.941, 4.63, 3.90, 0.92, {2.69, 1.0, 1.0, 1.0}, {1, 5.10, 0.69, -0.20}},
	{23, 5.300, 2.63, 4.83, 1.00, {4.11, 1.0, 1.0, 1.0}, {1, 4.20, 4.10, -1.00}},
	{41, 7.470, 2.86, 4.79, 1.00, {4.37, 1.0, 1.0, 1.0}, {1, 3.76, 3.83, -1.00}},
	{73, 8.089, 2.86, 4.90, 0.99, {3.71, 1.0, 1.0, 1.0}, {1, 4.96, 3.35, -1.50}},
	{24, 4.100, 2.50, 5.12, 0.94, {3.22, 1.0, 1.0, 1.0}, {1, -0.21, 12.26, -1.90}},
	{42, 6.810, 2.73, 5.85, 0.99, {4.48, 1.0, 1.0, 1.0}, {1, 3.48, 9.49, -2.90}},
	{74, 8.660, 2.74, 5.63, 0.98, {3.98, 1.0, 1.0, 1.0}, {1, 3.16, 8.25, -2.70}},
	{26, 4.290, 2.48, 5.07, 0.89, {2.94, 1.0, 1.0, 1.0}, {1, 3.94, 4.12, -1.50}},
	{6, 7.370, 1.54, 4.31, 1.80, {5.50, 4.3, 3.1, 6.0}, {1, 5.57, 1.94, -0.77}},
	{14, 4.630, 2.35, 4.87, 1.00, {4.40, 5.5, 5.5, 5.5}, {1, 3.13, 4.47, -1.80}},
	{32, 3.850, 2.45, 4.98, 1.00, {4.55, 5.5, 5.5, 5.5}, {1, 4.02, 5.23, -1.60}}
};

void MP_MEAMInit(MP_MEAM *meam)
{
	int i;
	int ntable = sizeof(MEAMParmTable)/sizeof(MEAMParmTable[0]);

	meam->nparm = 0;
	for (i = 0; i < ntable; i++) {
		if (MP_MEAMAddParm(meam, MEAMParmTable[i]) < 0) break;
	}
}

int MP_MEAMAddParm(MP_MEAM *meam, MP_MEAMParm parm)
{
	if (meam->nparm >= MP_MEAM_NPARM_MAX) {
		fprintf(stderr, "Error : can't add MEAM parameter (MP_MEAMAddParm)\n");
		return -1;
	}
	meam->parm[meam->nparm] = parm;
	return meam->nparm++;
}

static MP_MEAMParm MEAMGetParm(MP_MEAM *meam, short type)
{
	int i;
	MP_MEAMParm err = { 0, 0.0, 0.0, 0.0, 0.0, { 0.0, 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0, 0.0 } };

	for (i = 0; i < meam->nparm; i++) {
		if (meam->parm[i].type == type) return meam->parm[i];
	}
	return err;
}

static void MEAMParmSi(int ncluster, short types[], double Si[])
{
	int i;
	int zd = 0;

	for (i = 0; i < ncluster; i++) {
		if (types[i] > 0) zd++;
	}
	Si[0] = zd * zd, Si[1] = 0.0, Si[2] = 0.0, Si[3] = 0.0;
}

double MP_MEAMEnergy(MP_MEAM *meam, MP_KMCData *data, short types[])
{
	double Si[4];
	MP_MEAMParm p;

	MEAMParmSi(data->ncluster, types, Si);
	p = MEAMGetParm(meam, types[0]);
	fprintf(stderr, "%d %f %f\n", p.type, p.E0i, p.R0i);

	return 0.0;
}