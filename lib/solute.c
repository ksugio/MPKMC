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

int MP_KMCSoluteCluster(MP_KMCData *data, int ncluster_max, int nsolute_max, int *nsolute, int **ids, short **types)
{
	int i, j;

	for (i = 0; i < ncluster_max; i++) {
		nsolute[i] = 0;
		for (j = 0; j < nsolute_max; j++) {
			ids[i][j] = 0, types[i][j] = 0;
		}
	}

	return 0;
}