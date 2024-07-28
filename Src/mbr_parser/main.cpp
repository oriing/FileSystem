#include "parser_head.h"

lint	blockSize = 512;
lint	tableSize = 16;

lint checkSignature(FILE* fp) {
	const char	FILE_SIGNATURE[3] = { 0x55, 0xAA, 0x00 };
	char		d[3] = { 0, 0, 0 };

	if (!getFileToChar(fp, blockSize - 2, d, 2)) {
		printf("[error] File is too small");
		fclose(fp);
		exit(0);
	}

	return cmpTwoChar(FILE_SIGNATURE, d, 2);
}

bool readTableNow(FILE* fp, lint size, lint phy) {
	// print: {파일시스템타입} {실 시작 Sector 위치} {Size}

	int partition = getPartitionType(fp, phy);
	
	switch (partition) {
	case 1:
		printf("NTFS ");
		break;
	case 3:
		printf("FAT32 ");
		break;
	default:
		printf("Unknown ");
	}

	printf("%lld %lld\n", phy, size);

	return true;
}

void readEBR(FILE* fp, lint now, const lint origin, lint before) {
	now += blockSize - 2 - tableSize * 4;
	int32_t temp;
	if (!getFileToInt_32(fp, now + 8, &temp)) {
		printf("[error] on get position\n");
		fclose(fp);
		exit(0);
	}
	lint realPos = temp * (lint)512;
	//printf("%lld\n", (realPos + origin) / 512);

	char mode[2] = {};
	if (!getFileToChar(fp, now + 4, mode, 1)) {
		printf("[error] on get Mode of Point\n");
		fclose(fp);
		exit(0);
	}

	if (mode[0] == 0x05) { // EBR
		readEBR(fp, realPos + origin, origin, realPos);
		readEBR(fp, realPos + 16 + origin, origin, realPos);
		readEBR(fp, realPos + 16 * 2 + origin, origin, realPos);
		readEBR(fp, realPos + 16 * 3 + origin, origin, realPos);
	}
	else if (mode[0] == 0x00) {
		return;
	}
	else {
		int32_t size = 0;
		if (!getFileToInt_32(fp, now + 12, &size)) {
			printf("[error] on get Partition End\n");
			fclose(fp);
			exit(0);
		}
		readTableNow(fp, (lint)size * 512, realPos + origin + before);
	}
}

void readTable(FILE* fp, lint now) {
	int32_t temp;
	if (!getFileToInt_32(fp, now+8, &temp)) {
		printf("[error] on get position\n");
		fclose(fp);
		exit(0);
	}
	lint realPos = temp * (lint)512;

	char mode[2] = {};
	if (!getFileToChar(fp, now + 4, mode, 1)) {
		printf("[error] on get Mode of Point\n");
		fclose(fp);
		exit(0);
	}

	if (mode[0] == 0x05) { // EBR
		readEBR(fp, realPos, realPos, 0);
		readEBR(fp, realPos + 16, realPos, 0);
		readEBR(fp, realPos + 16 * 2, realPos, 0);
		readEBR(fp, realPos + 16 * 3, realPos, 0);
	}
	else if (mode[0] == 0x00) {
		return;
	}
	else {
		int32_t size = 0;
		if (!getFileToInt_32(fp, now + 12, &size)) {
			printf("[error] on get Partition End\n");
			fclose(fp);
			exit(0);
		}
		readTableNow(fp, (lint)size * 512, realPos);
	}
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		printf("[error] Use ./gpt_parser [FileName]\n");
		exit(0);
	}

	FILE* fp = openFile(argv[1]);

	if (!checkSignature(fp)) {
		printf("[error] The filesystem is not MBR Type.\n");
		fclose(fp);
		exit(0);
	}

	readTable(fp, blockSize - 2 - tableSize * 4);
	readTable(fp, blockSize - 2 - tableSize * 3);
	readTable(fp, blockSize - 2 - tableSize * 2);
	readTable(fp, blockSize - 2 - tableSize * 1);

	fclose(fp);

	return 0;
}
