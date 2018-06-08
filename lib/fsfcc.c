#include "MPKMC.h"

void MP_FSFCCCu(MP_FSFCCParm *parm)
{
	double a[] = { 29.059214, -140.05681, 130.07331, -17.48135, 31.82546, 71.58749 };
	double R[] = { 1.2247449, 1.0 };
	double A[] = { 9.806694, 16.774638 };
	double r[] = { 1.2247449, 1.1547054, 1.1180065, 1.0, 0.8660254, 0.7071068 };
	int i;

	parm->type = 29;
	parm->lc = 3.61;
	for (i = 0; i < 2; i++) {
		parm->R[i] = R[i];
		parm->A[i] = A[i];
	}
	for (i = 0; i < 6; i++) {
		parm->a[i] = a[i];
		parm->r[i] = r[i];
	}
}

double MP_FSFCCEnergy(MP_FSFCCParm *parm, MP_KMCData *data, short types[])
{
	int i, j;
	double r, Ui;
	double rho, sV;
	double phi, V;

	if (types[0] != parm->type) return 0.0;
	for (i = 1, rho = 0.0, sV = 0.0; i < data->ncluster; i++) {
		if (types[i] == parm->type) {
			r = sqrt(data->cluster[i][0] * data->cluster[i][0]
				+ data->cluster[i][1] * data->cluster[i][1]
				+ data->cluster[i][2] * data->cluster[i][2]);
			for (j = 0, phi = 0.0; j < 2; j++) {
				if (parm->R[j] >= r) {
					phi += parm->A[j] * pow(parm->R[j] - r, 3.0);
				}
			}
			rho += phi;
			for (j = 0, V = 0.0; j < 6; j++) {
				if (parm->r[j] >= r) {
					V += parm->a[j] * pow(parm->r[j] - r, 3.0);
				}
			}
			sV += V;
		}
	}
	Ui = -sqrt(rho) + 0.5*sV;
	return Ui;
}