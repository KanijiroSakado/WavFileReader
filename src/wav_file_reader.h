#ifdef WAVE_FILE_READER_H
#else
#define WAV_FILE_READER_H

#include<cstdio>
#include<exception>


namespace gold {

	class WavFileReader {

	public:
		unsigned long FmtSize;
		unsigned short FmtID;
		unsigned short NumChannels;
		unsigned long SampleRate;
		unsigned long BytesPerSec;
		unsigned short BlockAlign;//(bytes per sample)*(channels)
		unsigned short BitsPerSample;
		unsigned long DataSize;
		unsigned short BytesPerSample;
		unsigned long NumData;


		WavFileReader(const char* filename);
		WavFileReader(const char* filename, unsigned int gpBufCnt);
		~WavFileReader();

		void PrintHeader();

		unsigned int Read(signed short *buf, unsigned int count);
		unsigned int Read(int *buf, unsigned int count);
		unsigned int Read(double *buf, unsigned int count);
		unsigned int Read(float *buf, unsigned int count);
		unsigned int Read(unsigned char *buf, unsigned int count);

		unsigned int ReadLR(signed short *bufL, signed short *bufR, unsigned int count);
		unsigned int ReadLR(int *bufL, int *bufR, unsigned int count);
		unsigned int ReadLR(double *bufL, double *bufR, unsigned int count);
		unsigned int ReadLR(float *bufL, float *bufR, unsigned int count);
		unsigned int ReadLR(unsigned char *bufL, unsigned char *bufR, unsigned int count);

		int Seek(long offset, int origin);
		unsigned long Tell();

	private:
		FILE* fp;
		unsigned int numGpBuf;//count of general purpose buf = total buf size / BlockAlign
		void *gpBuf;
		unsigned long dataCnt;
		unsigned char *ucharp;
		signed short *shortp;
		void WavFileReaderPrivate(const char *filename);

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

}


#endif
