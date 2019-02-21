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
		if (MP_MEAMSetParm(meam, MEAMParmTable[i]) < 0) break;
	}
	meam->S[0] = 0.0, meam->S[1] = 0.0, meam->S[2] = 0.0;
}

int MP_MEAMSetParm(MP_MEAM *meam, MP_MEAMParm parm)
{
	int i;

	if (meam->nparm >= MP_MEAM_NPARM_MAX) {
		fprintf(stderr, "Error : can't add MEAM parameter (MP_MEAMSetParm)\n");
		return -1;
	}
	for (i = 0; i < meam->nparm; i++) {
		if (meam->parm[i].type == parm.type) {
			meam->parm[i] = parm;
			return i;
		}
	}
	meam->parm[meam->nparm] = parm;
	return meam->nparm++;
}

static MP_MEAMParm GetMEAMParm(MP_MEAM *meam, short type)
{
	int i;
	MP_MEAMParm err = { -1, 0.0, 0.0, 0.0, 0.0, { 0.0, 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0, 0.0 } };

	for (i = 0; i < meam->nparm; i++) {
		if (meam->parm[i].type == type) return meam->parm[i];
	}
	return err;
}

static int RijXij(MP_KMCData *data, short types[], short ntypes[], double rij[], double xij[][3])
{
	int i;
	int zi = 0;
	double dx, dy, dz, r;

	for (i = 1; i < data->ncluster; i++) {
		ntypes[zi] = types[i];
		dx = data->rcluster[i][0] - data->rcluster[0][0];
		dy = data->rcluster[i][1] - data->rcluster[0][1];
		dz = data->rcluster[i][2] - data->rcluster[0][2];
		r = sqrt(dx * dx + dy * dy + dz * dz);
		rij[zi] = r;
		xij[zi][0] = dx / r;
		xij[zi][1] = dy / r;
		xij[zi][2] = dz / r;
		zi++;
	}
	return zi;
}

static double Rhoia(double R, double Betai, double R0i)
{
	return exp(-Betai * (R / R0i - 1.0));
}

static double Rho_i(MP_MEAM *meam, int zi, short ntypes[], double rij[], double xij[][3], double Ti[])
{
	int j, l;
	int a, b, c;
	MP_MEAMParm p;
	double Betaj[MP_KMC_NCLUSTER_MAX][4];
	double R0j[MP_KMC_NCLUSTER_MAX];
	double t0, t1, t2;
	double rhoi2[4];

	for (j = 0; j < zi; j++) {
		if (ntypes[j] > 0) {
			p = GetMEAMParm(meam, ntypes[j]);
			Betaj[j][0] = p.Betai[0];
			Betaj[j][1] = p.Betai[1];
			Betaj[j][2] = p.Betai[2];
			Betaj[j][3] = p.Betai[3];
			R0j[j] = p.R0i;
		}
	}
	// rhoi0
	t0 = 0.0;
	for (j = 0; j < zi; j++) {
		if (ntypes[j] > 0) {
			t0 += Rhoia(rij[j], Betaj[j][0], R0j[j]);
		}
	}
	rhoi2[0] = t0 * t0;
	// rhoi1
	t0 = 0.0;
	for (a = 0; a < 3; a++) {
		t1 = 0.0;
		for (j = 0; j < zi; j++) {
			if (ntypes[j] > 0) {
				t1 += xij[j][a] * Rhoia(rij[j], Betaj[j][1], R0j[j]);
			}
		}
		t0 += t1 * t1;
	}
	rhoi2[1] = t0;
	// rhoi2
	t0 = 0.0;
	for (a = 0; a < 3; a++) {
		for (b = 0; b < 3; b++) {
			t1 = 0.0;
			for (j = 0; j < zi; j++) {
				if (ntypes[j] > 0) {
					t1 += xij[j][a] * xij[j][b] * Rhoia(rij[j], Betaj[j][2], R0j[j]);
				}
			}
			t0 += t1 * t1;
		}
	}
	t2 = 0.0;
	for (j = 0; j < zi; j++) {
		if (ntypes[j] > 0) {
			t2 += Rhoia(rij[j], Betaj[j][2], R0j[j]);
		}
	}
	rhoi2[2] = t0 - t2 * t2 / 3.0;
	// rhoi3
	t0 = 0.0;
	for (a = 0; a < 3; a++) {
		for (b = 0; b < 3; b++) {
			for (c = 0; c < 3; c++) {
				t1 = 0.0;
				for (j = 0; j < zi; j++) {
					if (ntypes[j] > 0) {
						t1 += xij[j][a] * xij[j][b] * xij[j][c] * Rhoia(rij[j], Betaj[j][3], R0j[j]);
					}
				}
				t0 += t1 * t1;
			}
		}
	}
	rhoi2[3] = t0;
	// rho_i
	t0 = 0.0;
	for (l = 0; l < 4; l++) {
		t0 += Ti[l] * rhoi2[l];
	}
	return sqrt(t0);
}

static double Fi(double rho, double Ai, double E0i)
{
	return Ai * E0i * rho * log(rho);
}

static double Eui(double R, double E0i, double R0i, double Alphai)
{
	double aa;

	aa = Alphai*(R / R0i - 1.0);
	return -E0i*(1.0 + aa)*exp(-aa);
}

static double Rho_0i(double rij, double R0i, double Betai[], double Ti[], double Si[])
{
	int l;
	double t0 = 0.0;

	for (l = 0; l < 4; l++) {
		t0 += Ti[l] * Si[l] * pow(Rhoia(rij, Betai[l], R0i), 2.0);
	}
	return sqrt(t0);
}

double MP_MEAMEnergy(MP_MEAM *meam, MP_KMCData *data, short types[])
{
	int j;
	int zi;
	double Si[4];
	MP_MEAMParm p;
	short ntypes[MP_KMC_NCLUSTER_MAX];
	double rij[MP_KMC_NCLUSTER_MAX];
	double xij[MP_KMC_NCLUSTER_MAX][3];
	double t1, t2, t3;
	double rho_i, rho_0i;

	p = GetMEAMParm(meam, types[0]);
	zi = RijXij(data, types, ntypes, rij, xij);
	Si[0] = zi * zi;
	Si[1] = meam->S[0], Si[2] = meam->S[1], Si[3] = meam->S[2];
	// term 1
	t1 = 0.0;
	for (j = 0; j < zi; j++) {
		t1 += Eui(rij[j], p.E0i, p.R0i, p.Alphai);
	}
	t1 = t1 / zi;
	// term 2
	rho_i = Rho_i(meam, zi, ntypes, rij, xij, p.Ti);
	t2 = Fi(rho_i / zi, p.Ai, p.E0i);
	// term 3
	t3 = 0.0;
	for (j = 0; j < zi; j++) {
		rho_0i = Rho_0i(rij[j], p.R0i, p.Betai, p.Ti, Si) / zi;
		t3 += Fi(rho_0i, p.Ai, p.E0i);
	}
	t3 = t3 / zi;
	return t1+t2-t3;
}
