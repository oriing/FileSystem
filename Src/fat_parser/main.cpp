#include "parser_head.h"


lint	startPoint = 0;
lint	sectorSize = 0;


void readHeader(FILE* fp) {
	int16_t temp;
	if (!getFileToInt_16(fp, 11, &temp)) {
		printf("[error] on get sector size.\n");
		fclose(fp);
		exit(0);
	}
	sectorSize = temp;

	if (!getFileToInt_16(fp, 14, &temp)) {
		printf("[error] on get table pointer.\n");
		fclose(fp);
		exit(0);
	}
	startPoint = temp;
}

const char endSignature[5]  = { 0xFF, 0xFF, 0xFF, 0x0F };
const char noneSignature[5] = { 0xFF, 0xFF, 0xFF, 0xFF };
const char emptySignature[5] = { 0x00, 0x00, 0x00, 0x00 };

void readTable(FILE* fp, lint now) {
	char thisData[5] = {};
	lint npoint = now * 4 + startPoint * sectorSize;
	
	getFileToChar(fp, npoint, thisData, 4);
	if (!strncmp(thisData, endSignature, 4)) {
		printf("%d\n", now);
		return;
	}
	if (!strncmp(thisData, noneSignature, 4)) {
		printf("\n"); return;
	}
	if (!strncmp(thisData, emptySignature, 4)) {
		printf("\n"); return;
	}

	printf("%d ", now);
	int32_t next = 0;
	getFileToInt_32(fp, npoint, &next);
	readTable(fp, next);
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		printf("[error] Use ./fat_parser [FileName] [Start_Cluster]\n");
		exit(0);
	}

	FILE* fp = openFile(argv[1]);

	if (getPartitionType(fp, 0)!=3) {
		printf("[error] The filesystem is not FAT32 Type.\n");
		fclose(fp);
		exit(0);
	}

	readHeader(fp);

	readTable(fp, atoi(argv[2]));

	fclose(fp);

	return 0;
}
