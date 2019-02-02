#ifdef WAVE_FILE_READER_H
#else
#define WAV_FILE_READER_H

#include<cstdio>
#include<exception>

#define WAV_SEEK_SET SEEK_SET
#define WAV_SEEK_CUR SEEK_CUR
#define WAV_SEEK_END SEEK_END


class WavFileReader {

public:

	void *gpbuf;
	FILE* fp;
	unsigned int FmtSize;
	unsigned short FmtID;
	unsigned short NumChannels;
	unsigned int SampleRate;
	unsigned int BytesPerSec;
	unsigned short BlockAlign;//(bytes per sample)*(channels)
	unsigned short BitsPerSample;
	unsigned int DataSize;
	unsigned short BytesPerSample;


	WavFileReader(const char* filename);
	WavFileReader(const char* filename,unsigned int gpBufCnt);
	~WavFileReader();

	void PrintHeader();
	
	int Read(signed short *buf, unsigned int count);
	int Read(int *buf, unsigned int count);
	int Read(double *buf, unsigned int count);
	int Read(float *buf, unsigned int count);
	int Read(unsigned char *buf, unsigned int count);

	int ReadLR(signed short *bufL, signed short *bufR, unsigned int count);
	int ReadLR(int *bufL, int *bufR, unsigned int count);
	int ReadLR(double *bufL, double *bufR, unsigned int count);
	int ReadLR(float *bufL, float *bufR, unsigned int count);
	int ReadLR(unsigned char *bufL, unsigned char *bufR, unsigned int count);

	int Seek(long offset, int origin);

private:
	int gpBufCnt;//count of general purpose buf = total buf size / BlockAlign
	void WavFileReaderPrivate(const char*filename);

};



class WFRException :public std::exception {

};

class WFRFileOpenException :public  WFRException {
	const char* what(void) const noexcept {
		return "File Open Failed";
	}
};

class WFRFileValidityException :public  WFRException {
	const char* what(void) const noexcept {
		return "Invalid File";
	}
};

class WFRSupportedFormatException :public  WFRException {
	const char* what(void) const noexcept {
		return "Unsupported format (Supported format is 8bit and 16bit only)";
	}
};





#endif
