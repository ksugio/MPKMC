#include "MPKMC.h"

int MP_KMCAddSolute(MP_KMCData *data, int id, short type, short jump)
{
	int i;
	int nsolute_max;
	int sid;
	MP_KMCSoluteItem last;

	if (id < 0 || id >= data->ntot) {
		fprintf(stderr, "Error : invalid index %d.\n", id);
		return -1;
	}
	if (data->nsolute >= data->nsolute_max) {
		nsolute_max = data->nsolute_max + data->nsolute_step;
		data->solute = (MP_KMCSoluteItem *)realloc(data->solute, nsolute_max * sizeof(MP_KMCSoluteItem));
		if (data->solute == NULL) {
			fprintf(stderr, "Error : allocation failure (MP_KMCAddSolute)\n");
			return MP_KMC_MEM_ERR;
		}
		data->nsolute_max = nsolute_max;
	}
	for (i = 0; i < data->nsolute; i++) {
		if (data->solute[i].id == id) return -1;
	}
	data->grid[id].type = type;
	sid = data->nsolute;
	data->solute[sid].id = id;
	data->solute[sid].type = type;
	data->solute[sid].jump = jump;
	data->solute[sid].njump = 0;
	data->solute[sid].group = -1;
	data->nsolute++;
	if (jump) {
		if (sid > data->dpmax) {
			last = data->solute[sid];
			for (i = sid; i >= data->dpmax; i--) {
				data->solute[i + 1] = data->solute[i];
			}
			data->solute[data->dpmax] = last;
			sid = data->dpmax;
		}
		data->dpmax++;
	}
	return sid;
}

void MP_KMCAddSoluteRandom(MP_KMCData *data, int num, short type, short jump)
{
	int count = 0;
	int id;
	int sid;

	while (count < num) {
		id = (int)(MP_Rand(&(data->rand_seed)) * data->ntot);
		sid = MP_KMCAddSolute(data, id, type, jump);
		if (sid >= 0) count++;
		else if (sid == MP_KMC_MEM_ERR) return;
	}
}

int MP_KMCCheckSolute(MP_KMCData *data)
{
	int i, j;
	int count = 0;

	for (i = 0; i < data->nsolute; i++) {
		for (j = 0; j < data->nsolute; j++) {
			if (i != j && data->solute[i].id == data->solute[j].id) {
				fprintf(stderr, "Error : overlapped ID, %d.(MP_KMCCheckSolute)\n", data->solute[i].id);
				return FALSE;
			}
		}
	}
	for (i = 0; i < data->nsolute; i++) {
		if (data->grid[data->solute[i].id].type != data->solute[i].type) {
			fprintf(stderr, "Error : type mismatch, %d.(MP_KMCCheckSolute)\n", data->solute[i].id);
			return FALSE;
		}
	}
	return TRUE;
}

static void KMCFindGroup(MP_KMCData *data, int sid, int group, double rcut)
{
	int i;
	int p0, x0, y0, z0;
	int p1, x1, y1, z1;
	double dx, dy, dz, dr2;

	data->solute[sid].group = group;
	MP_KMCIndex2Grid(data, data->solute[sid].id, &p0, &x0, &y0, &z0);
	for (i = 0; i < data->nsolute; i++) {
		if (i != sid && data->solute[i].group < 0) {
			MP_KMCIndex2Grid(data, data->solute[i].id, &p1, &x1, &y1, &z1);
			if (x1 - x0 >= data->size[0] - 1) x1 -= data->size[0];
			else if (x1 - x0 <= -data->size[0] + 1) x1 += data->size[0];
			if (y1 - y0 >= data->size[1] - 1) y1 -= data->size[1];
			else if (y1 - y0 <= -data->size[1] + 1) y1 += data->size[1];
			if (z1 - z0 >= data->size[2] - 1) z1 -= data->size[2];
			else if (z1 - z0 <= -data->size[2] + 1) z1 += data->size[2];
			dx = (x1 + data->uc[p1][0]) - (x0 + data->uc[p0][0]);
			dy = (y1 + data->uc[p1][1]) - (y0 + data->uc[p0][1]);
			dz = (z1 + data->uc[p1][2]) - (z0 + data->uc[p0][2]);
			dr2 = dx * dx + dy * dy + dz * dz;
			if (dr2 <= rcut * rcut) {
				KMCFindGroup(data, i, group, rcut);
			}
		}
	}
}

int MP_KMCFindSoluteGroup(MP_KMCData *data, double rcut)
{
	int i;
	int group = 0;

	for (i = 0; i < data->nsolute; i++) {
		data->solute[i].group = -1;
	}
	for (i = 0; i < data->nsolute; i++) {
		if (data->solute[i].group < 0) {
			KMCFindGroup(data, i, group++, rcut);
		}
	}
	return group;
}
