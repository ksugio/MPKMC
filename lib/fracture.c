#include "MPKMC.h"

static void ClusterIndexesCrosses(MP_KMCData* data, double a, double b, double c, double d,
	int side, int id, int ids[], int crss[])
{
	int i;
	int p, x, y, z;
	int np, nx, ny, nz;
	int pt;
	double pos[3], r;

	MP_KMCIndex2Grid(data, id, &p, &x, &y, &z);
	for (i = 0; i < data->ncluster; i++) {
		pt = p * (data->ncluster * 4) + i * 4;
		np = data->clusterid[pt++];
		nx = x + data->clusterid[pt++];
		ny = y + data->clusterid[pt++];
		nz = z + data->clusterid[pt];
		pos[0] = data->uc[np][0] + nx;
		pos[1] = data->uc[np][1] + ny;
		pos[2] = data->uc[np][2] + nz;
		if (nx < 0) {
			nx += data->size[0];
		}
		else if (nx >= data->size[0]) {
			nx -= data->size[0];
		}
		if (ny < 0) {
			ny += data->size[1];
		}
		else if (ny >= data->size[1]) {
			ny -= data->size[1];
		}
		if (nz < 0) {
			nz += data->size[2];
		}
		else if (nz >= data->size[2]) {
			nz -= data->size[2];
		}
		ids[i] = MP_KMCGrid2Index(data, np, nx, ny, nz);
		r = a * pos[0] + b * pos[1] + c * pos[2] + d;
		if (side == 0 && r >= 0.0) {
			crss[i] = TRUE;
		}
		else if (side == 1 && r < 0.0) {
			crss[i] = TRUE;
		}
		else {
			crss[i] = FALSE;
		}
	}
}

static void ClusterTypesCrosses(MP_KMCData* data, double a, double b, double c, double d,
	int side, int id, short perf_types[], short frac_types[])
{
	int i;
	int ids[MP_KMC_NCLUSTER_MAX];
	int crss[MP_KMC_NCLUSTER_MAX];

	ClusterIndexesCrosses(data, a, b, c, d, side, id, ids, crss);
	for (i = 0; i < data->ncluster; i++) {
		perf_types[i] = data->grid[ids[i]].type;
		if (crss[i]) {
			frac_types[i] = 0;
		}
		else {
			frac_types[i] = data->grid[ids[i]].type;
		}
	}
}

static double CalcEnergy(MP_KMCData* data, short types[], 
	double (*func)(MP_KMCData*, short*), int table_add)
{
	int tid;
	double energy = 0.0;

	if (data->table_use) {
		tid = MP_KMCSearchCluster(data, types);
		if (tid >= 0) {
			energy = data->table[tid].energy;
		}
		else if (func != NULL) {
			energy = (func)(data, types);
			if (tid >= 0) {
				data->table[tid].refcount += 1;
			}
			else if (tid == -1 && data->table_add && table_add) {
				MP_KMCAddCluster(data, types, energy, 1);
			}
		}
	}
	else {
		energy = (func)(data, types);
	}
	return energy;
}

double MP_KMCFractureEnergy(MP_KMCData* data, double a, double b, double c, double d, double width,
	double (*perf_func)(MP_KMCData*, short*), double (*frac_func)(MP_KMCData*, short*))
{
	int i;
	short perf_types[MP_KMC_NCLUSTER_MAX];
	short frac_types[MP_KMC_NCLUSTER_MAX];
	double pos[3], r, dis;
	double eperf = 0.0;
	double efrac = 0.0;

	for (i = 0; i < data->ntot; i++) {
		MP_KMCIndex2Pos(data, i, pos);
		r = a * pos[0] + b * pos[1] + c * pos[2] + d;
		dis = fabs(r) / sqrt(a * a + b * b + c * c);
		if (r >= 0.0 && dis < width / 2) {
			ClusterTypesCrosses(data, a, b, c, d, 1, i, perf_types, frac_types);
			eperf += CalcEnergy(data, perf_types, perf_func, TRUE);
			data->grid[i].energy = (frac_func)(data, frac_types);
			efrac += data->grid[i].energy;
		}
		else if (r < 0.0 && dis < width / 2) {
			ClusterTypesCrosses(data, a, b, c, d, 0, i, perf_types, frac_types);
			eperf += (perf_func)(data, perf_types);
			data->grid[i].energy = (frac_func)(data, frac_types);
			efrac += data->grid[i].energy;
		}
	}
	return efrac - eperf;
}

static int Intersection(double a, double b, double c, double d,
	double lx, double ly, double lz, double* x, double* y, double* z)
{
	if (lx < 0.0) {
		if (a == 0.0) return FALSE;
		*x = -(b * ly + c * lz + d) / a;
		*y = ly;
		*z = lz;
		if (*x >= -lx || *x < 0.0) return FALSE;
	}
	if (ly < 0.0) {
		if (b == 0.0) return FALSE;
		*x = lx;
		*y = -(a * lx + c * lz + d) / b;
		*z = lz;
		if (*y >= -ly || *y < 0.0) return FALSE;
	}
	if (lz < 0.0) {
		if (c == 0.0) return FALSE;
		*x = lx;
		*y = ly;
		*z = -(a * lx + b * ly + d) / c;
		if (*z >= -lz || *z < 0.0) return FALSE;
	}
	return TRUE;
}

static int NearestPoint(int npoints, double points[][3], double x, double y, double z, double* dis)
{
	int i;
	int minid;
	double dx, dy, dz, d;

	*dis = 1.0e23;
	for (i = 0; i < npoints; i++) {
		dx = points[i][0] - x;
		dy = points[i][1] - y;
		dz = points[i][2] - z;
		d = sqrt(dx * dx + dy * dy + dz * dz);
		if (d < *dis) {
			*dis = d;
			minid = i;
		}
	}
	return minid;
}

static double HeronArea(double p0[], double p1[], double p2[])
{
	double a = sqrt((p1[0] - p0[0]) * (p1[0] - p0[0])
		+ (p1[1] - p0[1]) * (p1[1] - p0[1]) + (p1[2] - p0[2]) * (p1[2] - p0[2]));
	double b = sqrt((p2[0] - p1[0]) * (p2[0] - p1[0])
		+ (p2[1] - p1[1]) * (p2[1] - p1[1]) + (p2[2] - p1[2]) * (p2[2] - p1[2]));
	double c = sqrt((p0[0] - p2[0]) * (p0[0] - p2[0])
		+ (p0[1] - p2[1]) * (p0[1] - p2[1]) + (p0[2] - p2[2]) * (p0[2] - p2[2]));
	double s = (a + b + c) / 2.0;

	return sqrt(s * (s - a) * (s - b) * (s - c));
}

double MP_KMCFractureArea(MP_KMCData* data, double a, double b, double c, double d)
{
	int i;
	int is;
	double lines[][3] = { {-1.0, 0.0, 0.0}, {0.0, -1.0, 0.0}, {0.0, 0.0, -1.0},
		{-1.0, 1.0, 0.0}, {1.0, -1.0, 0.0}, {1.0, 0.0, -1.0},
		{-1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {1.0, 1.0, -1.0},
		{-1.0, 0.0, 1.0}, {0.0, -1.0, 1.0}, {0.0, 1.0, -1.0}
	};
	double x, y, z;
	int npts = 0;
	double pts[12][3];
	int npos = 0;
	double pos[12][3];
	double rpos[12][3];
	double sx, sy, sz, dis;
	int nid;
	double area = 0.0;

	for (i = 0; i < 12; i++) {
		is = Intersection(a, b, c, d,
			lines[i][0] * data->size[0],
			lines[i][1] * data->size[1],
			lines[i][2] * data->size[2], &x, &y, &z);
		if (is) {
			pts[npts][0] = x;
			pts[npts][1] = y;
			pts[npts][2] = z;
			npts++;
		}
	}
	sx = pts[0][0], sy = pts[0][1], sz = pts[0][2];
	for (i = 0; i < npts; i++) {
		nid = NearestPoint(npts, pts, sx, sy, sz, &dis);
		if (dis > 1.0e-3 || i == 0) {
			sx = pos[npos][0] = pts[nid][0];
			sy = pos[npos][1] = pts[nid][1];
			sz = pos[npos][2] = pts[nid][2];
			npos++;
		}
		pts[nid][0] = 1.0e23;
	}
	for (i = 0; i < npos; i++) {
		MP_KMCRealPos(data, pos[i], rpos[i]);
	}
	for (i = 0; i < npos - 2; i++) {
		area += HeronArea(rpos[0], rpos[i + 1], rpos[i + 2]);
	}
	return area;
}
