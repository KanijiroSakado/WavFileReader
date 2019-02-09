//MIT License

//Copyright(c) 2019 GoldSakana
//https://twitter.com/goldsakana

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.


#ifdef WAVE_FILE_READER_H
#else
#define WAV_FILE_READER_H

#include<cstdio>
#include<exception>


namespace gold {

	class WavFileReader {

	public:
		unsigned int FmtSize;
		unsigned short FmtID;
		unsigned short NumChannels;
		unsigned int SampleRate;
		unsigned int BytesPerSec;
		unsigned short BlockAlign;//(bytes per sample)*(channels)
		unsigned short BitsPerSample;
		unsigned int DataSize;
		unsigned short BytesPerSample;
		unsigned int NumData;


		WavFileReader(const char* filename);
		WavFileReader(const char* filename, unsigned int gpBufCnt);
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
		FILE* fp;
		unsigned int numGpBuf;//count of general purpose buf = total buf size / BlockAlign
		void *gpBuf;
		unsigned int dataCnt;
		unsigned char *ucharp;
		signed short int *shortp;
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
