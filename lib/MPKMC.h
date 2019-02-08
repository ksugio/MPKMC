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

#ifdef MP_PYTHON_LIB
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

#ifdef _OPENMP
#include <omp.h>
#endif

/*--------------------------------------------------
* kmc typedef and functions
*/
#define MP_KMC_NUC_MAX 32
#define MP_KMC_NCLUSTER_MAX 64
#define MP_KMC_NROT_MAX 64
#define MP_KMC_MEM_ERR -99

enum {MP_KMCFCC};

typedef struct MP_KMCTableItem {
	short types[MP_KMC_NCLUSTER_MAX];
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
	int njump;
	int group;
} MP_KMCSoluteItem;

typedef struct MP_KMCEventItem {
	int dp;
	int id0;
	int id1;
	double de;
	int dmcs;
} MP_KMCEventItem;

typedef struct MP_KMCResultItem {
	long totmcs;
	double temp;
	int ntry;
	int njump;
	int ntable;
	double tote;
} MP_KMCResultItem;

typedef struct MP_KMCData {
#ifdef MP_PYTHON_LIB
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
	int cpmax;
	int nrot;
	int *rotid;
	int table_use;
	int ntable;
	int ntable_step;
	int ntable_max;
	char htable[256];
	MP_KMCTableItem *table;
	int nsolute;
	int nsolute_step;
	int nsolute_max;
	int dpmax;
	MP_KMCSoluteItem *solute;
	int event_record;
	int nevent;
	int nevent_step;
	int nevent_max;
	int step;
	MP_KMCEventItem *event;
	int nresult;
	int nresult_step;
	int nresult_max;
	MP_KMCResultItem *result;
	long rand_seed;
	long totmcs;
	long mcs;
	double tote;
	double kb;
} MP_KMCData;

int MP_KMCAlloc(MP_KMCData *data, int nuc, int nx, int ny, int nz, int ncluster,
	int nsolute_step, int ntable_step, int nevent_step, int nresult_step);
void MP_KMCFree(MP_KMCData *data);
void MP_KMCSetUnitCell(MP_KMCData *data, double uc[][3], short types[], double pv[][3]);
int MP_KMCSetCluster(MP_KMCData *data, double cluster[][3], int cpmax);
void MP_KMCRealPos(MP_KMCData *data, double pos[], double rpos[]);
void MP_KMCIndex2Grid(MP_KMCData *data, int id, int *p, int *x, int *y, int *z);
int MP_KMCGrid2Index(MP_KMCData *data, int p, int x, int y, int z);
void MP_KMCIndex2Pos(MP_KMCData *data, int id, double pos[]);
void MP_KMCClusterIndexes(MP_KMCData *data, int id, int ids[]);
void MP_KMCClusterTypes(MP_KMCData *data, int id, short types[]);
int MP_KMCSearchCluster(MP_KMCData *data, short types[]);
int MP_KMCSearchClusterIDs(MP_KMCData *data, int ids[]);
int MP_KMCAddCluster(MP_KMCData *data, short types[], double energy, long refcount);
int MP_KMCAddClusterIDs(MP_KMCData *data, int ids[], double energy, long refcount);
int MP_KMCCountType(MP_KMCData *data, short type);
void MP_KMCSortTable(MP_KMCData *data);
void MP_KMCResetTable(MP_KMCData *data);
int MP_KMCSearchTable(MP_KMCData *data, char ss[], MP_KMCTableItem list[], int list_max);
int MP_KMCAddResult(MP_KMCData *data, double temp, int ntry, int njump);

/*--------------------------------------------------
* solute functions
*/
int MP_KMCAddSolute(MP_KMCData *data, int id, short type, short jump);
void MP_KMCAddSoluteRandom(MP_KMCData *data, int num, short type, short jump);
int MP_KMCCheckSolute(MP_KMCData *data);
int MP_KMCFindSoluteGroup(MP_KMCData *data, double rcut);

/*--------------------------------------------------
* jump functions
*/
double MP_KMCTotalEnergy(MP_KMCData *data, double(*func)(MP_KMCData *, short *), int *update);
int MP_KMCJump(MP_KMCData *data, int ntry, double temp, double(*func)(MP_KMCData *, short *), int *update);
void MP_KMCStepForward(MP_KMCData *data, int count);
void MP_KMCStepBackward(MP_KMCData *data, int count);
void MP_KMCStepGo(MP_KMCData *data, int step);
long MP_KMCStep2MCS(MP_KMCData *data, int step);
int MP_KMCMCS2Step(MP_KMCData *data, long mcs);
void MP_KMCMCSHistory(MP_KMCData *data, int num, double mcs[]);
void MP_KMCEnergyHistory(MP_KMCData *data, int num, double ene[]);

/*--------------------------------------------------
* rw functions
*/
int MP_KMCWriteTable(MP_KMCData *data, char *filename);
int MP_KMCReadTable(MP_KMCData *data, char *filename);
int MP_KMCWrite(MP_KMCData *data, char *filename, int comp);
int MP_KMCRead(MP_KMCData *data, char *filename, int version);

/*--------------------------------------------------
* rotindex functions
*/
int MP_KMCAddRotIndex(MP_KMCData *data, int ids[]);
int MP_KMCCalcRotIndex(MP_KMCData *data, double step, double tol);

/*--------------------------------------------------
* types functions
*/
void MP_KMCTypes2String(int ncluster, short types[], char str[]);
int MP_KMCString2Types(char str[], short types[]);

/*--------------------------------------------------
* rand functions
*/
float MP_Rand(long *rand_seed);
float MP_RandGauss(long *rand_seed);

/*--------------------------------------------------
* fsfcc typedef and functions
*/

typedef struct MP_FSFCCParm {
#ifdef MP_PYTHON_LIB
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

#ifdef MP_PYTHON_LIB
PyTypeObject MP_FSFCCPyType;
#endif

int MP_FSFCCInit(MP_FSFCCParm *parm, short type);
double MP_FSFCCEnergy(MP_FSFCCParm *parm, MP_KMCData *data, short types[]);

#ifdef __cplusplus
}
#endif

#endif /* _MPKMC_H */
