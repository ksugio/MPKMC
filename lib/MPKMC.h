#ifndef _MPKMC_H
#define _MPKMC_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef _DEBUG
#ifndef Py_PYTHON_H
#include <Python.h>
#endif
#ifndef Py_STRUCTMEMBER_H
#include <structmember.h>
#endif
#endif
#ifndef _INC_STDIO
#include <stdio.h>
#endif
#ifndef _INC_STDLIB
#include <stdlib.h>
#endif
#ifndef _INC_STRING
#include <string.h>
#endif
#ifndef _INC_MATH
#include <math.h>
#endif

#ifndef ZLIB_H
#include <zlib.h>
#endif

#ifdef WIN32
#pragma warning(disable:4996)
#include <windows.h>
#endif

/*--------------------------------------------------
* kmc typedef and functions
*/
#define MP_KMC_NUC_MAX 32
#define MP_KMC_NCLUSTER_MAX 64
#define MP_KMC_NROT_MAX 64


enum {MP_KMCFCC};

typedef struct MP_KMCTableItem {
	short *types;
	double energy;
	long refcount;
} MP_KMCTableItem;

typedef struct MP_KMCGridItem {
	short type;
	double energy;
} MP_KMCGridItem;

typedef struct MP_KMCSoluteItem {
	int id;
	short type;
	short jump;
} MP_KMCSoluteItem;

typedef struct MP_KMCEventItem {
	int dp;
	int id0;
	int id1;
	double de;
} MP_KMCEventItem;

typedef struct MP_KMCData {
#ifndef _DEBUG
	PyObject_HEAD
	PyObject *pyfunc;
#endif
	int nuc;
	double uc[MP_KMC_NUC_MAX][3];
	short uc_types[MP_KMC_NUC_MAX];
	double pv[3][3];
	int size[3];
	int ntot;
	MP_KMCGridItem *grid;
	int ncluster;
	double cluster[MP_KMC_NCLUSTER_MAX][3];
	double rcluster[MP_KMC_NCLUSTER_MAX][3];
	int *clusterid;
	int nrot;
	int *rotid;
	int table_use;
	int ntable;
	int ntable_step;
	int ntable_max;
	char htable[256];
	MP_KMCTableItem *table;
	short *table_types;
	int nsolute;
	int nsolute_max;
	MP_KMCSoluteItem *solute;
	int nevent;
	int nevent_step;
	int nevent_max;
	MP_KMCEventItem *event;
	long rand_seed;
	long step;
	double tote;
} MP_KMCData;

int MP_KMCAlloc(MP_KMCData *data, int nuc, int nx, int ny, int nz, int ncluster,
	int nsolute_max, int ntable_step, int nevent_step);
void MP_KMCFree(MP_KMCData *data);
void MP_KMCSetUnitCell(MP_KMCData *data, double uc[][3], short types[], double pv[][3]);
int MP_KMCSetCluster(MP_KMCData *data, double cluster[][3]);
void MP_KMCRealPos(MP_KMCData *data, double cp[], double rp[]);
void MP_KMCIndex2Grid(MP_KMCData *data, int id, int *p, int *x, int *y, int *z);
int MP_KMCGrid2Index(MP_KMCData *data, int p, int x, int y, int z);
void MP_KMCClusterIndexes(MP_KMCData *data, int id, int ids[]);
int MP_KMCSearchCluster(MP_KMCData *data, short types[]);
int MP_KMCSearchClusterIDs(MP_KMCData *data, int ids[]);
int MP_KMCAddCluster(MP_KMCData *data, short types[], double energy, long refcount);
int MP_KMCAddClusterIDs(MP_KMCData *data, int ids[], double energy, long refcount);
int MP_KMCAddSolute(MP_KMCData *data, int id, short type, short jump);
void MP_KMCAddSoluteRandom(MP_KMCData *data, int num, short type, short jump);
double MP_KMCCalcEnergy(MP_KMCData *data, int id, double(*func)(MP_KMCData *, short *), int *update);
double MP_KMCTotalEnergy(MP_KMCData *data, double(*func)(MP_KMCData *, short *), int *update);
int MP_KMCJump(MP_KMCData *data, int ntry, double kt, double(*func)(MP_KMCData *, short *), int *update);
void MP_KMCStepForward(MP_KMCData *data, int count);
void MP_KMCStepBackward(MP_KMCData *data, int count);
void MP_KMCStepGo(MP_KMCData *data, int step);
void MP_KMCEnergyHistory(MP_KMCData *data, int nhist, double ehist[]);
int MP_KMCWriteTable(MP_KMCData *data, char *filename);
int MP_KMCReadTable(MP_KMCData *data, char *filename);
void MP_KMCSortTable(MP_KMCData *data);
void MP_KMCResetTable(MP_KMCData *data);
int MP_KMCWrite(MP_KMCData *data, char *filename, int comp);
int MP_KMCRead(MP_KMCData *data, char *filename);

/*--------------------------------------------------
* rotindex functions
*/
int MP_KMCAddRotIndex(MP_KMCData *data, int ids[]);
int MP_KMCCalcRotIndex(MP_KMCData *data, double step, double tol);

/*--------------------------------------------------
* rand functions
*/
float MP_Rand(long *rand_seed);
float MP_RandGauss(long *rand_seed);

/*--------------------------------------------------
* fsfcc typedef and functions
*/

typedef struct MP_FSFCCParm {
#ifndef _DEBUG
	PyObject_HEAD
		PyObject *pyfunc;
#endif
	short type;
	double lc;
	double a[6];
	double R[2];
	double A[2];
	double r[6];
} MP_FSFCCParm;

#ifndef _DEBUG
PyTypeObject MP_FSFCCPyType;
#endif

int MP_FSFCCInit(MP_FSFCCParm *parm, short type);
double MP_FSFCCEnergy(MP_FSFCCParm *parm, MP_KMCData *data, short types[]);

/*--------------------------------------------------
* fsbcc typedef and functions
*/

typedef struct MP_FSBCCParm {
#ifndef _DEBUG
	PyObject_HEAD
		PyObject *pyfunc;
#endif
	short type;
	double d;
	double A;
	double beta;
	double c[4];
} MP_FSBCCParm;



#ifdef __cplusplus
}
#endif

#endif /* _MPKMC_H */
