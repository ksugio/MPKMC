#include "MPKMC.h"

#define CHAR_TABLE_NUM 60

static const char CharTable_q[] = { "0123456789fDMASpsljKacTVrnFoNCZGeHEykRJYzbmLuhPgdItWwiXOBUvx" };
static const char CharTable_Q[] = { "NmSEGvDorzYLfTWROIPAHlbBptnFachiUJujCkKeVMXxwdgys9012345678Z" };

void MP_KMCTypes2String(int ncluster, short types[], char str[])
{
	int i;
	char *p = str;
	int flag = -1;

	for (i = 0; i < ncluster; i++) {
		if (types[i] < CHAR_TABLE_NUM) {
			if (flag != 0) {
				*p++ = 'q';
				flag = 0;
			}
			*p++ = CharTable_q[types[i]];
		}
		else {
			if (flag != 1) {
				*p++ = 'Q';
				flag = 1;
			}
			*p++ = CharTable_Q[types[i]-CHAR_TABLE_NUM];
		}
	}
	*p = '\0';
}

static short SearchType(char c, const char table[]) {
	int i;

	for (i = 0; i < CHAR_TABLE_NUM; i++) {
		if (c == table[i]) return i;
	}
	return -1;
}

int MP_KMCString2Types(char str[], short types[])
{
	char *p = str;
	int flag = -1;
	int count = 0;

	while (*p != '\0' && *p != '\n') {
		if (*p == 'q') flag = 0;
		else if (*p == 'Q') flag = 1;
		else {
			if (flag == 0) {
				types[count++] = SearchType(*p, CharTable_q);
			}
			else if (flag == 1) {
				types[count++] = SearchType(*p, CharTable_Q)+CHAR_TABLE_NUM;
			}
		}
		p++;
	}
	return count;
}
