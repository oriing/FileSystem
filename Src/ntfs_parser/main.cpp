#include "parser_head.h"
#include <memory.h>
#include "bigint.h"

lint sectorPerCluster = 8;
lint bytesPerSector   = 256;
lint startMft         = 0;
char fixup[11]        = {};



void readHeader(FILE* fp) {
	int8_t  temp_8;
	int16_t temp_16;

	getFileToInt_8(fp, 13, &temp_8);
	sectorPerCluster = temp_8;

	getFileToInt_16(fp, 11, &temp_16);
	bytesPerSector = temp_16;

	getFileToInt_64(fp, 48, &startMft);
	startMft = startMft * sectorPerCluster * bytesPerSector;
}

bool checkMft(FILE* fp, lint x) {
	char temp[5];
	getFileToChar(fp, x, temp, 4);
	return cmpTwoChar(temp, "FILE", 4);
}

// safety start ==============
bool inboundFixup(lint tar, lint set, lint size) {
	lint x = tar - set;
	x %= 0x200;
	if (0x1ff >= x && 0x1fe < x + size) return true;
	return false;
}
bool fixupChanged(FILE* fp, lint spoint, char* data, lint size, lint startPoint) {
	if (!getFileToChar(fp, spoint, data, size)) return false;
	lint x = spoint - startPoint;
	lint t = x % 0x200;
	lint s = x / 0x200 * 2 + 2;
	for (int i = 0; i < size; i++) {
		if (t + i == 0x1fe) data[i] = fixup[s];
		if (t + i == 0x1ff) data[i] = fixup[s + 1];
	}
	return true;
}
bool getFileToInt_8_safe(FILE* fp, lint spoint, int8_t* a, lint startPoint) {
	if (!inboundFixup(spoint, startPoint, 1)) return getFileToInt_8(fp, spoint, a);
	char data[2];
	if (!fixupChanged(fp, spoint, data, 1, startPoint)) return false;

	*a = data[0];
	return true;
}
bool getFileToInt_16_safe(FILE* fp, lint spoint, int16_t* a, lint startPoint) {
	if (!inboundFixup(spoint, startPoint, 2)) return getFileToInt_16(fp, spoint, a);
	char data[3];
	if (!fixupChanged(fp, spoint, data, 2, startPoint)) return false;

	*a = 0;
	for (int i = 1; i >= 0; i--) {
		*a = *a << 8;
		*a += ((int)data[i] & 0x000000FF);
	}
	return true;
}
bool getFileToInt_32_safe(FILE* fp, lint spoint, int32_t* a, lint startPoint) {
	if (!inboundFixup(spoint, startPoint, 4)) return getFileToInt_32(fp, spoint, a);
	char data[5];
	if (!fixupChanged(fp, spoint, data, 4, startPoint)) return false;

	*a = 0;
	for (int i = 3; i >= 0; i--) {
		*a = *a << 8;
		*a += ((int)data[i] & 0x000000FF);
	}
	return true;
}
bool getFileToInt_64_safe(FILE* fp, lint spoint, int64_t* a, lint startPoint) {
	if (!inboundFixup(spoint, startPoint, 8)) return getFileToInt_64(fp, spoint, a);
	char data[9];
	if (!fixupChanged(fp, spoint, data, 8, startPoint)) return false;

	*a = 0;
	for (int i = 7; i >= 0; i--) {
		*a = *a << 8;
		*a += ((int)data[i] & 0x000000FF);
	}
	return true;
}

bool getFileToChar_safe(FILE* fp, lint spoint, char* data, lint size, lint startPoint) {
	if (!inboundFixup(spoint, startPoint, size)) return getFileToChar(fp, spoint, data, size);
	return fixupChanged(fp, spoint, data, size, startPoint);
}
// safety end ==============

lint readAttribute(FILE* fp, lint thisPos, lint startPoint) {
	int32_t next;
	getFileToInt_32_safe(fp, thisPos + 4, &next, startPoint);
	
	int8_t nrflag;
	getFileToInt_8_safe(fp, thisPos + 8, &nrflag, startPoint);

	if (nrflag == 1) {
		int16_t offsetRunlist;
		getFileToInt_16_safe(fp, thisPos + 32, &offsetRunlist, startPoint);

		int8_t clusterSize;
		lint now = offsetRunlist + thisPos;

		do {
			getFileToInt_8_safe(fp, now, &clusterSize, startPoint);
			if (clusterSize == 0) break;

			int8_t os = (clusterSize >> 4) & 0x0F;
			int8_t ns = clusterSize & 0x0F;
			bigint ox = bigint(0), nx = bigint(0);

			if (os != 0) {
				char* odata = (char*)malloc(os+1);
				if(odata == NULL) {
					printf("[error] malloc failed\n");
					exit(0);
				}

				getFileToChar_safe(fp, now + 1 + ns, odata, os, startPoint);
				for (int i = os - 1; i >= 0; i--) {
					ox <<= 8;
					ox += ((lint)odata[i] & 0x000000FF);
				}

				free(odata);
			}
			if (ns != 0) {
				char* ndata = (char*)malloc(ns+1);
				if (ndata == NULL) {
					printf("[error] malloc failed\n");
					exit(0);
				}
				getFileToChar_safe(fp, now + 1, ndata, ns, startPoint);

				for (int i = ns - 1; i >= 0; i--) {
					nx <<= 8;
					nx += ((lint)ndata[i] & 0x000000FF);
				}

				free(ndata);
			}

			cout << ox << " "<< nx << std::endl;
			now += ns + os + 1;
		} while (true);
	}

	return next;
}

bool readMft(FILE* fp, lint startPoint) {
	if (!checkMft(fp, startPoint)) {
		return false;
	}

	int16_t firstAttribute = 0;
	getFileToInt_16(fp, startPoint + 20, &firstAttribute);

	int16_t fixupOffset;
	getFileToInt_16(fp, startPoint + 4, &fixupOffset);
	int16_t fixupSize;
	getFileToInt_16(fp, startPoint + 6, &fixupSize);

	getFileToChar(fp, startPoint + fixupOffset, fixup, fixupSize*2);
	
	lint thisPos = (lint)firstAttribute + startPoint;
	lint nextPos = 0;
	do {
		nextPos = readAttribute(fp, thisPos, startPoint);
		thisPos += nextPos;
	} while (nextPos != 0);

	return true;
}

int main(int argc, char** argv) {
	if (argc < 1) {
		printf("[error] Use ./ntfs_parser [FileName]\n");
		exit(0);
	}

	FILE* fp = openFile(argv[1]);

	if (getPartitionType(fp, 0)!=1) {
		printf("[error] The filesystem is not NTFS Type.\n");
		fclose(fp);
		exit(0);
	}

	readHeader(fp);

	while (readMft(fp, startMft)) startMft += 0x400;

	fclose(fp);

	return 0;
}
