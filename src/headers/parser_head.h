#ifndef PARSER_HEAD_INCLUDED
#define PARSER_HEAD_INCLUDED true

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef int64_t lint;

FILE* openFile(char* name) {
	FILE* fp = fopen(name, "rb");

	if (fp == NULL) {
		printf("[error] Cannot open file %s", name);
		exit(0);
	}

	return fp;
}

void checkMover(char* arr, int count) {
	arr[0] = 0;
	for (int i = 1; i < count; i++) {
		arr[i - 1] = arr[i];
		arr[i] = 0;
	}
}

bool getFileToInt_32(FILE* fp, lint spoint, int32_t* a) {
	fseek(fp, spoint, 0);
	*a = 0;

	for (int i = 0; i < 4; i++) {
		if (feof(fp)) return false;
		*a = *a >> 8;
		fread((char*)a + 3, sizeof(char), 1, fp);
	}
	return true;
}
bool getFileToInt_64(FILE* fp, lint spoint, int64_t* a) {
	fseek(fp, spoint, 0);
	*a = 0;

	for (int i = 0; i < 8; i++) {
		if (feof(fp)) return false;
		*a = *a >> 8;
		fread((char*)a + 7, sizeof(char), 1, fp);
	}
	return true;
}

bool getFileToChar(FILE* fp, lint spoint, char* data, lint size) {
	fseek(fp, spoint, 0);
	fread(data, sizeof(char), size, fp);
	data[size] = 0;

	return true;
}

bool cmpTwoChar(const char* a, const char* b, int size) {
	for (int i = 0; i < size; i++) {
		if (a[i] != b[i]) {
			return false;
		}
	}
	return true;
}

void printCharToHex(char* data, int size) {
	for (int i = 0; i < size; i++) {
		printf("%X", (int)data[i] & 0x000000ff);
	}
}

int getPartitionType(FILE* fp, lint start) {
	/// <summary>
	/// Undefined, NTFS, FAT12/16, FAT32
	/// </summary>
	char d[4] = {0, };
	if (!getFileToChar(fp, start, d, 3)) {
		printf("[error] on get partition type.\n");
		fclose(fp);
		exit(0);
	}

	const char NTFS[4]  = {0xEB, 0x52, 0x90, 0x00};
	const char FAT16[4] = {0xEB, 0x3C, 0x90, 0x00};
	const char FAT32[4] = {0xEB, 0x58, 0x90, 0x00};

	if (cmpTwoChar(d, NTFS , 3)) return 1;
	if (cmpTwoChar(d, FAT16, 3)) return 2;
	if (cmpTwoChar(d, FAT32, 3)) return 3;
	return 0;
}

#endif