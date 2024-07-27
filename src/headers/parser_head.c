#include "parser_head.h"

FILE* openFile(char* name) {
	FILE* fp = fopen(name, "rb");

	if (fp == NULL) {
		printf("[error] Cannot open file %s", name);
		exit(0);
	}

	return fp;
}

void checkMover(char* arr) {
	arr[0] = 0;
	for (int i = 1; arr[i]; i++) {
		arr[i - 1] = arr[i];
		arr[i] = 0;
	}
}

bool getFileToInt_32(FILE* fp, lint spoint, int32_t* a) {
	fseek(fp, spoint, 0);
	*a = 0;

	for (int i = 0; i < 4; i++) {
		if (feof(fp)) return false;
		*a = *a >> 1;
		fread(a, sizeof(char), 1, fp);
	}
	return true;
}
bool getFileToInt_64(FILE* fp, lint spoint, int64_t* a) {
	fseek(fp, spoint, 0);
	*a = 0;

	for (int i = 0; i < 8; i++) {
		if (feof(fp)) return false;
		*a = *a >> 1;
		fread(a, sizeof(char), 1, fp);
	}
	return true;
}

bool getFileToChar(FILE* fp, lint spoint, char* data, lint size) {
	fseek(fp, spoint, 0);

	for (int i = 0; i < size; i++) {
		if (feof(fp)) return false;

		fread(&data[i], sizeof(char), 1, fp);
	}
	data[size] = 0;

	return true;
}