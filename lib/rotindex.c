#include "MPKMC.h"

static void rotateZ(double az, double v0[], double v1[])
{
	double azr = M_PI * az / 180.0;

	v1[0] = v0[0] * cos(azr) - v0[1] * sin(azr);
	v1[1] = v0[0] * sin(azr) + v0[1] * cos(azr);
	v1[2] = v0[2];
}

static void rotateY(double ay, double v0[], double v1[])
{
	double ayr = M_PI * ay / 180.0;

	v1[0] = v0[0] * cos(ayr) - v0[2] * sin(ayr);
	v1[1] = v0[1];
	v1[2] = v0[0] * sin(ayr) + v0[2] * cos(ayr);
}

static void rotateX(double ax, double v0[], double v1[])
{
	double axr = M_PI * ax / 180.0;

	v1[0] = v0[0];
	v1[1] = v0[1] * cos(axr) - v0[2] * sin(axr);
	v1[2] = v0[1] * sin(axr) + v0[2] * cos(axr);
}

static void rotateZYX(double az, double ay, double ax, double v0[], double v1[])
{
	double v00[3], v01[3];

	rotateZ(az, v0, v00);
	rotateY(ay, v00, v01);
	rotateX(ax, v01, v1);
}

static int searchIndex(MP_KMCData *data, double v[], double tol)
{
	int i;
	double v0[3];
	double dx, dy, dz;

	for (i = 0; i < data->ncluster; i++) {
		MP_KMCRealPos(data, data->cluster[i], v0);
		dx = fabs(v0[0] - v[0]);
		dy = fabs(v0[1] - v[1]);
		dz = fabs(v0[2] - v[2]);
		if (dx < tol && dy < tol && dz < tol) {
			return i;
		}
	}
	return -1;
}

static int updateIndexes(MP_KMCData *data, double az, double ay, double ax, double tol)
{
	int i;
	int id;
	double v0[3], v1[3];
	int ids[MP_KMC_NCLUSTER_MAX];

	for (i = 0; i < data->ncluster; i++) {
		MP_KMCRealPos(data, data->cluster[i], v0);
		rotateZYX(az, ay, ax, v0, v1);
		id = searchIndex(data, v1, tol);
		if (id >= 0) {
			ids[i] = id;
		}
		else {
			return FALSE;
		}
	}
	return MP_KMCAddRotIndex(data, ids);
}

/*static void printIndexes(MP_KMCData *data)
{
	int i, j;

	printf("%d rot indexes\n", data->nrot);
	for (i = 0; i < data->nrot; i++) {
		for (j = 0; j < data->ncluster; j++) {
			printf("%d ", data->rot_index[i][j]);
		}
		printf("\n");
	}
}*/

int MP_KMCAddRotIndex(MP_KMCData *data, int ids[])
{
	int i, j;

	if (data->nrot >= MP_KMC_NROT_MAX) {
		fprintf(stderr, "Error : too many indexes (MP_KMCAddRotIndex)\n");
		return FALSE;
	}
	for (i = 0; i < data->nrot; i++) {
		for (j = 0; j < data->ncluster; j++) {
			if (data->rot_index[i][j] != ids[j]) break;
		}
		if (j == data->ncluster) return FALSE;
	}
	for (j = 0; j < data->ncluster; j++) {
		data->rot_index[i][j] = ids[j];
	}
	data->nrot++;
	return TRUE;
}

int MP_KMCCalcRotIndex(MP_KMCData *data, double step, double tol)
{
	double ax, ay, az;
	int count = 0;

	for (ax = 0.0; ax < 360.0; ax = ax + step) {
		for (ay = 0.0; ay < 360.0; ay = ay + step) {
			for (az = 0.0; az < 360.0; az = az + step) {
				if (updateIndexes(data, az, ay, ax, tol)) {
					count++;
				}
			}
		}
	}
	return count;
}
