#define _CRT_SECURE_NO_WARNINGS//without this, Visual Studio throws warnings for fopen function that suggests to use fopen_s instead
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include"wav_file_reader.h"
using namespace gold;

WavFileReader::WavFileReader(const char* filename, unsigned int gpBufCnt) {
	this->gpBufCnt = gpBufCnt;
	WavFileReaderPrivate(filename);
}

WavFileReader::WavFileReader(const char*filename) {
	gpBufCnt = 44100;//default
	WavFileReaderPrivate(filename);
}

void WavFileReader::WavFileReaderPrivate(const char*filename) {

	char ch[5];

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		throw WFRFileOpenException();
	}

	fread(ch, 1, 4, fp);
	ch[4] = '\0';
	if (strcmp(ch, "RIFF")) {
		fclose(fp);
		throw WFRFileValidityException();
	}

	fseek(fp, 4, SEEK_CUR);
	fread(ch, 1, 4, fp);
	ch[4] = '\0';
	if (strcmp(ch, "WAVE")) {
		fclose(fp);
		throw WFRFileValidityException();
	}

	fseek(fp, 4, SEEK_CUR);
	fread(&FmtSize, 4, 1, fp);
	fread(&FmtID, 2, 1, fp);
	fread(&NumChannels, 2, 1, fp);
	fread(&SampleRate, 4, 1, fp);
	fread(&BytesPerSec, 4, 1, fp);
	fread(&BlockAlign, 2, 1, fp);
	fread(&BitsPerSample, 2, 1, fp);
	fseek(fp, FmtSize - 16, SEEK_CUR);
	fseek(fp, 4, SEEK_CUR);
	fread(&DataSize, 4, 1, fp);
	BytesPerSample = BitsPerSample / 8;

	if (BitsPerSample != 8 && BitsPerSample != 16){
		fclose(fp);
		throw WFRSupportedFormatException();
	}

	if (BytesPerSample == 1) {
		gpbuf = (unsigned char*)malloc(sizeof(unsigned char) * gpBufCnt * NumChannels);
	}
	else {
		gpbuf = (signed short*)malloc(sizeof(signed short) * gpBufCnt * NumChannels);
	}

	ucharp = (unsigned char*)gpbuf;
	shortp = (signed short int*)gpbuf;

	return;
}

void WavFileReader::PrintHeader() {

	printf("format size= %d\nformat ID = %d\nchannels = %d\nsampling rate = %d\nbytes per sec = %d\nblock align = %d\nbits per sample = %d\ndata size = %d\n"
		, FmtSize, FmtID, NumChannels, SampleRate, BytesPerSec, BlockAlign, BitsPerSample, DataSize);

	return;
}

WavFileReader::~WavFileReader() {
	fclose(fp);
	free(gpbuf);
}

int WavFileReader::Read(unsigned char *buf, unsigned int leftToRead) {

	unsigned int readCnt,ret=0,temp,i = 0,pointer=0;
	
	readCnt = gpBufCnt*NumChannels;

	while (leftToRead>0) {
		
		if (leftToRead < gpBufCnt) {
			readCnt = leftToRead * NumChannels;
			leftToRead = 0;
		}
		else {
			leftToRead -= gpBufCnt;
		}

		temp = fread(gpbuf, BytesPerSample, readCnt, fp);
		ret += temp;
		if (temp < readCnt) {
			readCnt = temp;
			leftToRead = 0;
		}
		

		if (NumChannels == 1 && BytesPerSample == 1) {
			memcpy(buf + pointer, ucharp, readCnt);
			i = readCnt;
		}
		else if (NumChannels == 1 && BytesPerSample == 2) {
			for (i=0; i < readCnt; i++) {
				buf[pointer + i] = (unsigned int)((int)shortp[i] + 0x8fff) >> 9;
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 1) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				buf[pointer+i] = ((unsigned int)ucharp[i << 1] + (unsigned int)ucharp[(i << 1) + 1]) >> 1;
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 2) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				buf[pointer + i] = (unsigned int)((int)shortp[i << 1] + (int)shortp[(i << 1) + 1] + 0xffff) >> 9;
			}
		}
		pointer += i;
	}

	if (NumChannels == 1)return ret;
	else return ret >> 1;
}

int WavFileReader::Read(signed short *buf, unsigned int leftToRead) {

	unsigned int readCnt, ret = 0, temp, i = 0, pointer = 0;

	readCnt = gpBufCnt * NumChannels;

	while (leftToRead > 0) {

		if (leftToRead < gpBufCnt) {
			readCnt = leftToRead * NumChannels;
			leftToRead = 0;
		}
		else {
			leftToRead -= gpBufCnt;
		}

		temp = fread(gpbuf, BytesPerSample, readCnt, fp);
		ret += temp;
		if (temp < readCnt) {
			readCnt = temp;
			leftToRead = 0;
		}

		if (NumChannels == 1 && BytesPerSample == 1) {
			for (i = 0; i < readCnt; i++) {
				buf[pointer + i] = ucharp[i];
			}
		}
		else if (NumChannels == 1 && BytesPerSample == 2) {
			memcpy(buf + pointer, shortp, readCnt);
			i = readCnt;
		}
		else if (NumChannels == 2 && BytesPerSample == 1) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				buf[pointer + i] = ((int)ucharp[i << 1] + (int)ucharp[(i << 1) + 1])/2;
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 2) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				buf[pointer + i] = ((int)shortp[i << 1] + (int)shortp[(i << 1) + 1])/2;
			}
		}
		pointer += i;
	}

	if (NumChannels == 1)return ret;
	else return ret >> 1;
}

int WavFileReader::Read(int *buf, unsigned int leftToRead) {

	unsigned int readCnt, ret = 0, temp, i = 0, pointer = 0;

	readCnt = gpBufCnt * NumChannels;

	while (leftToRead > 0) {

		if (leftToRead < gpBufCnt) {
			readCnt = leftToRead * NumChannels;
			leftToRead = 0;
		}
		else {
			leftToRead -= gpBufCnt;
		}

		temp = fread(gpbuf, BytesPerSample, readCnt, fp);
		ret += temp;
		if (temp < readCnt) {
			readCnt = temp;
			leftToRead = 0;
		}

		if (NumChannels == 1 && BytesPerSample == 1) {
			for (i = 0; i < readCnt; i++) {
				buf[pointer + i] = ucharp[i];
			}
		}
		else if (NumChannels == 1 && BytesPerSample == 2) {
			for (i = 0; i < readCnt; i++) {
				buf[pointer + i] = shortp[i];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 1) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				buf[pointer + i] = ((int)ucharp[i << 1] + (int)ucharp[(i << 1) + 1]) / 2;
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 2) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				buf[pointer + i] = ((int)shortp[i << 1] + (int)shortp[(i << 1) + 1]) / 2;
			}
		}
		pointer += i;
	}

	if (NumChannels == 1)return ret;
	else return ret >> 1;
}

int WavFileReader::Read(double *buf, unsigned int leftToRead) {

	unsigned int readCnt, ret = 0, temp, i = 0, pointer = 0;

	readCnt = gpBufCnt * NumChannels;

	while (leftToRead > 0) {

		if (leftToRead < gpBufCnt) {
			readCnt = leftToRead * NumChannels;
			leftToRead = 0;
		}
		else {
			leftToRead -= gpBufCnt;
		}

		temp = fread(gpbuf, BytesPerSample, readCnt, fp);
		ret += temp;
		if (temp < readCnt) {
			readCnt = temp;
			leftToRead = 0;
		}

		if (NumChannels == 1 && BytesPerSample == 1) {
			for (i = 0; i < readCnt; i++) {
				buf[pointer + i] = ucharp[i];
			}
		}
		else if (NumChannels == 1 && BytesPerSample == 2) {
			for (i = 0; i < readCnt; i++) {
				buf[pointer + i] = shortp[i];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 1) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				buf[pointer + i] = ((int)ucharp[i << 1] + (int)ucharp[(i << 1) + 1]) / 2;
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 2) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				buf[pointer + i] = ((int)shortp[i << 1] + (int)shortp[(i << 1) + 1]) / 2;
			}
		}
		pointer += i;
	}

	if (NumChannels == 1)return ret;
	else return ret >> 1;
}

int WavFileReader::Read(float *buf, unsigned int leftToRead) {

	unsigned int readCnt, ret = 0, temp, i = 0, pointer = 0;

	readCnt = gpBufCnt * NumChannels;

	while (leftToRead > 0) {

		if (leftToRead < gpBufCnt) {
			readCnt = leftToRead * NumChannels;
			leftToRead = 0;
		}
		else {
			leftToRead -= gpBufCnt;
		}

		temp = fread(gpbuf, BytesPerSample, readCnt, fp);
		ret += temp;
		if (temp < readCnt) {
			readCnt = temp;
			leftToRead = 0;
		}

		if (NumChannels == 1 && BytesPerSample == 1) {
			for (i = 0; i < readCnt; i++) {
				buf[pointer + i] = ucharp[i];
			}
		}
		else if (NumChannels == 1 && BytesPerSample == 2) {
			for (i = 0; i < readCnt; i++) {
				buf[pointer + i] = shortp[i];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 1) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				buf[pointer + i] = ((int)ucharp[i << 1] + (int)ucharp[(i << 1) + 1]) / 2;
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 2) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				buf[pointer + i] = ((int)shortp[i << 1] + (int)shortp[(i << 1) + 1]) / 2;
			}
		}
		pointer += i;
	}

	if (NumChannels == 1)return ret;
	else return ret >> 1;
}

int WavFileReader::ReadLR(unsigned char *bufL, unsigned char *bufR, unsigned int leftToRead) {

	unsigned int readCnt,ret=0,temp, i = 0, pointer = 0;

	readCnt = gpBufCnt * NumChannels;

	while (leftToRead > 0) {

		if (leftToRead < gpBufCnt) {
			readCnt = leftToRead * NumChannels;
			leftToRead = 0;
		}
		else {
			leftToRead -= gpBufCnt;
		}

		temp = fread(gpbuf, BytesPerSample, readCnt, fp);
		ret += temp;
		if (temp < readCnt) {
			readCnt = temp;
			leftToRead = 0;
		}

		if (NumChannels == 1 && BytesPerSample == 1) {
			memcpy(bufR + pointer, ucharp, readCnt);
			memcpy(bufL + pointer, ucharp, readCnt);
			i = readCnt;
		}
		else if (NumChannels == 1 && BytesPerSample == 2) {
			for (i = 0; i < readCnt; i++) {
				bufR[pointer + i] = (unsigned int)((int)shortp[i] + 0x8fff) >> 8;
				bufL[pointer + i] = (unsigned int)((int)shortp[i] + 0x8fff) >> 8;
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 1) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				bufR[pointer + i] = ucharp[i << 1];
				bufL[pointer + i] = ucharp[(i << 1) + 1];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 2) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				bufR[pointer + i] = ((int)shortp[i << 1] + 0x8fff) >> 8;
				bufL[pointer + i] = ((int)shortp[(i << 1) + 1] + 0x8fff) >> 8;
			}
		}
		pointer += i;
	}

	if (NumChannels == 1)return ret;
	else return ret >> 1;
}

int WavFileReader::ReadLR(signed short *bufL, signed short *bufR, unsigned int leftToRead) {

	unsigned int readCnt, ret = 0, temp, i = 0, pointer = 0;

	readCnt = gpBufCnt * NumChannels;

	while (leftToRead > 0) {

		if (leftToRead < gpBufCnt) {
			readCnt = leftToRead * NumChannels;
			leftToRead = 0;
		}
		else {
			leftToRead -= gpBufCnt;
		}

		temp = fread(gpbuf, BytesPerSample, readCnt, fp);
		ret += temp;
		if (temp < readCnt) {
			readCnt = temp;
			leftToRead = 0;
		}

		if (NumChannels == 1 && BytesPerSample == 1) {
			for (i = 0; i < readCnt; i++) {
				bufR[pointer + i] = ucharp[i];
				bufL[pointer + i] = ucharp[i];
			}
		}
		else if (NumChannels == 1 && BytesPerSample == 2) {
			memcpy(bufR + pointer, shortp, readCnt);
			memcpy(bufL + pointer, shortp, readCnt);
			i = readCnt;
		}
		else if (NumChannels == 2 && BytesPerSample == 1) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				bufR[pointer + i] = ucharp[i << 1];
				bufL[pointer + i] = ucharp[(i << 1) + 1];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 2) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				bufR[pointer + i] = shortp[i << 1];
				bufL[pointer + i] = shortp[(i << 1) + 1];
			}
		}
		pointer += i;
	}

	if (NumChannels == 1)return ret;
	else return ret >> 1;
}

int WavFileReader::ReadLR(int *bufL, int *bufR, unsigned int leftToRead) {

	unsigned int readCnt, ret = 0, temp, i = 0, pointer = 0;

	readCnt = gpBufCnt * NumChannels;

	while (leftToRead > 0) {

		if (leftToRead < gpBufCnt) {
			readCnt = leftToRead * NumChannels;
			leftToRead = 0;
		}
		else {
			leftToRead -= gpBufCnt;
		}

		temp = fread(gpbuf, BytesPerSample, readCnt, fp);
		ret += temp;
		if (temp < readCnt) {
			readCnt = temp;
			leftToRead = 0;
		}

		if (NumChannels == 1 && BytesPerSample == 1) {
			for (i = 0; i < readCnt; i++) {
				bufR[pointer + i] = ucharp[i];
				bufL[pointer + i] = ucharp[i];
			}
		}
		else if (NumChannels == 1 && BytesPerSample == 2) {
			for (i = 0; i < readCnt; i++) {
				bufR[pointer + i] = shortp[i];
				bufL[pointer + i] = shortp[i];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 1) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				bufR[pointer + i] = ucharp[i << 1];
				bufL[pointer + i] = ucharp[(i << 1) + 1];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 2) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				bufR[pointer + i] = shortp[i << 1];
				bufL[pointer + i] = shortp[(i << 1) + 1];
			}
		}
		pointer += i;
	}

	if (NumChannels == 1)return ret;
	else return ret >> 1;
}

int WavFileReader::ReadLR(double *bufL, double *bufR, unsigned int leftToRead) {

	unsigned int readCnt, ret = 0, temp, i = 0, pointer = 0;

	readCnt = gpBufCnt * NumChannels;

	while (leftToRead > 0) {

		if (leftToRead < gpBufCnt) {
			readCnt = leftToRead * NumChannels;
			leftToRead = 0;
		}
		else {
			leftToRead -= gpBufCnt;
		}

		temp = fread(gpbuf, BytesPerSample, readCnt, fp);
		ret += temp;
		if (temp < readCnt) {
			readCnt = temp;
			leftToRead = 0;
		}

		if (NumChannels == 1 && BytesPerSample == 1) {
			for (i = 0; i < readCnt; i++) {
				bufR[pointer + i] = ucharp[i];
				bufL[pointer + i] = ucharp[i];
			}
		}
		else if (NumChannels == 1 && BytesPerSample == 2) {
			for (i = 0; i < readCnt; i++) {
				bufR[pointer + i] = shortp[i];
				bufL[pointer + i] = shortp[i];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 1) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				bufR[pointer + i] = ucharp[i << 1];
				bufL[pointer + i] = ucharp[(i << 1) + 1];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 2) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				bufR[pointer + i] = shortp[i << 1];
				bufL[pointer + i] = shortp[(i << 1) + 1];
			}
		}
		pointer += i;
	}

	if (NumChannels == 1)return ret;
	else return ret >> 1;
}

int WavFileReader::ReadLR(float *bufL, float *bufR, unsigned int leftToRead) {

	unsigned int readCnt, ret = 0, temp, i = 0, pointer = 0;

	readCnt = gpBufCnt * NumChannels;

	while (leftToRead > 0) {

		if (leftToRead < gpBufCnt) {
			readCnt = leftToRead * NumChannels;
			leftToRead = 0;
		}
		else {
			leftToRead -= gpBufCnt;
		}

		temp = fread(gpbuf, BytesPerSample, readCnt, fp);
		ret += temp;
		if (temp < readCnt) {
			readCnt = temp;
			leftToRead = 0;
		}

		if (NumChannels == 1 && BytesPerSample == 1) {
			for (i = 0; i < readCnt; i++) {
				bufR[pointer + i] = ucharp[i];
				bufL[pointer + i] = ucharp[i];
			}
		}
		else if (NumChannels == 1 && BytesPerSample == 2) {
			for (i = 0; i < readCnt; i++) {
				bufR[pointer + i] = shortp[i];
				bufL[pointer + i] = shortp[i];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 1) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				bufR[pointer + i] = ucharp[i << 1];
				bufL[pointer + i] = ucharp[(i << 1) + 1];
			}
		}
		else if (NumChannels == 2 && BytesPerSample == 2) {
			temp = readCnt >> 1;
			for (i = 0; i < temp; i++) {
				bufR[pointer + i] = shortp[i << 1];
				bufL[pointer + i] = shortp[(i << 1) + 1];
			}
		}
		pointer += i;
	}

	if (NumChannels == 1)return ret;
	else return ret >> 1;
}

int WavFileReader::Seek(long offset, int origin) {
	return fseek(fp, BlockAlign*offset, origin);
}











