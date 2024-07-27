#ifndef PARSER_HEAD_INCLUDED
#define PARSER_HEAD_INCLUDED true

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef long int lint;

FILE* openFile(char* name);

void checkMover(char* arr);

bool getFileToInt_32(FILE* fp, lint spoint, int32_t* a);

bool getFileToInt_64(FILE* fp, lint spoint, int64_t* a);

bool getFileToChar(FILE* fp, lint spoint, char* data, lint size);


#endif