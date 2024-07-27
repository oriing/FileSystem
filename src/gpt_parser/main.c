#include "parser_head.h"

lint	blockSize;
int64_t	lbastart;
int32_t tableSize;

lint checkSignature(FILE* fp) {
	const char	FILE_SIGNATURE[9]	= "EFI PART";
	char		d[9]				= "########";

	fseek(fp, 0, 0);
	
	for (lint i = 0; !feof(fp); i++) {
		checkMover(d, 8);
		fread(&d[7], sizeof(char), 1, fp);

		if (cmpTwoChar(FILE_SIGNATURE, d, 8)) {
			return i - 7;
		}
	}

	return -1;
}

void readHeader(FILE* fp) {
	lint now = blockSize + 16 * 4 + 8;
	if (!getFileToInt_64(fp, now, &lbastart)) {
		printf("[error] on get LBA start\n");
		fclose(fp);
		exit(0);
	}
	
	now = blockSize + 16 * 5 + 4;
	if (!getFileToInt_32(fp, now, &tableSize)) {
		printf("[error] on get table size\n");
		fclose(fp);
		exit(0);
	}
}

bool readTableNow(FILE* fp, lint now) {
	// print: {GUID} {파일시스템타입} {Sector위치} {size}
	char guid[9] = {0, };

	if (!getFileToChar(fp, now, guid, 8)) {
		printf("[error] on get GUID\n");
		fclose(fp);
		exit(0);
	}

	lint start = 0, end = 0;

	if (!getFileToInt_64(fp, now + 32, &start)) {
		printf("[error] on get Partition Start\n");
		fclose(fp);
		exit(0);
	}
	if (!getFileToInt_64(fp, now + 32 + 8, &end)) {
		printf("[error] on get Partition End\n");
		fclose(fp);
		exit(0);
	}

	if (start == 0) return false;

	int partition = getPartitionType(fp, start * 512);
	lint size = (end - start) * tableSize;

	printCharToHex(guid, 8);
	printf(" ");

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

	printf("%lld %lld\n", start, size);

	return true;
}

void readTable(FILE* fp) {
	lint now = blockSize * lbastart;

	while (readTableNow(fp, now)) now += tableSize;
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		printf("[error] Use ./gpt_parser [FileName]\n");
		exit(0);
	}

	FILE* fp = openFile(argv[1]);

	blockSize = checkSignature(fp);
	if (blockSize == -1) {
		printf("[error] The filesystem is not GPT Type.\n");
		fclose(fp);
		exit(0);
	}

	readHeader(fp);

	readTable(fp);

	fclose(fp);

	return 0;
}
