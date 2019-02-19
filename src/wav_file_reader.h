#ifdef WAV_FILE_READER_H
#else
#define WAV_FILE_READER_H

#include<cstdio>
#include<exception>


namespace gold {

	template<typename T> struct ReadFunctionArgumentTypeValidation;
	template<> struct ReadFunctionArgumentTypeValidation<int> {};
	template<> struct ReadFunctionArgumentTypeValidation<long> {};
	template<> struct ReadFunctionArgumentTypeValidation<double> {};
	template<> struct ReadFunctionArgumentTypeValidation<float> {};


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
		unsigned int Read(unsigned char *buf, unsigned int count);
		
		//I know this is kind of like a mess, but function templates have to be defined inside headers...
		template <class Type> unsigned int Read(Type *buf, unsigned int count) {

			unsigned int readCnt, numSuccess, i = 0, pointer = 0,leftToRead;

			sizeof(ReadFunctionArgumentTypeValidation<Type>);
			//If you have compiler error here, then the type of the first argument of Read function is illegal
			//Available types are [unsigned char*, int*, short*, long*, double*, float*]

			leftToRead = count;
			if (dataCnt + leftToRead > NumData)leftToRead = NumData - dataCnt;
			readCnt = numGpBuf * NumChannels;

			while (leftToRead > 0) {

				if (leftToRead < numGpBuf) {
					readCnt = leftToRead * NumChannels;
					leftToRead = 0;
				}
				else {
					leftToRead -= numGpBuf;
				}

				numSuccess = fread(gpBuf, BytesPerSample, readCnt, fp);
				if (numSuccess < readCnt) leftToRead = 0;

				if (NumChannels == 1 && BytesPerSample == 1) {
					for (i = 0; i < numSuccess; pointer++, i++) {
						buf[pointer] = ucharp[i];
					}
				}
				else if (NumChannels == 1 && BytesPerSample == 2) {
					for (i = 0; i < numSuccess; pointer++, i++) {
						buf[pointer] = shortp[i];
					}
				}
				else if (NumChannels == 2 && BytesPerSample == 1) {
					for (i = 0; i < numSuccess; pointer++, i += 2) {
						buf[pointer] = ((long)ucharp[i] + (long)ucharp[i + 1]) / 2;
					}
				}
				else if (NumChannels == 2 && BytesPerSample == 2) {
					for (i = 0; i < numSuccess; pointer++, i += 2) {
							buf[pointer] = ((long)shortp[i] + (long)shortp[i + 1]) / 2;
					}
				}
			}

			dataCnt += pointer;
			return pointer;
		}

		unsigned int ReadLR(signed short *bufL, signed short *bufR, unsigned int count);
		unsigned int ReadLR(unsigned char *bufL, unsigned char *bufR, unsigned int count);
		
		//I know this is kind of like a mess, but function templates have to be defined inside headers...
		template <class Type> unsigned int ReadLR(Type *bufL, Type *bufR, unsigned int count) {

			unsigned int readCnt, numSuccess, i = 0, pointer = 0,leftToRead;

			sizeof(ReadFunctionArgumentTypeValidation<Type>);
			//If you have compiler error here, then the type of the first argument of ReadLR function is illegal
			//Available types are [unsigned char*, int*, short*, long*, double*, float*]

			leftToRead = count;
			if (dataCnt + leftToRead > NumData)leftToRead = NumData - dataCnt;
			readCnt = numGpBuf * NumChannels;

			while (leftToRead > 0) {

				if (leftToRead < numGpBuf) {
					readCnt = leftToRead * NumChannels;
					leftToRead = 0;
				}
				else {
					leftToRead -= numGpBuf;
				}

				numSuccess = fread(gpBuf, BytesPerSample, readCnt, fp);
				if (numSuccess < readCnt) leftToRead = 0;

				if (NumChannels == 1 && BytesPerSample == 1) {
					for (i = 0; i < numSuccess; pointer++, i++) {
						bufL[pointer] = ucharp[i];
						bufR[pointer] = ucharp[i];
					}
				}
				else if (NumChannels == 1 && BytesPerSample == 2) {
					for (i = 0; i < numSuccess; pointer++, i++) {
						bufL[pointer] = shortp[i];
						bufR[pointer] = shortp[i];
					}
				}
				else if (NumChannels == 2 && BytesPerSample == 1) {
					for (i = 0; i < numSuccess; pointer++, i += 2) {
						bufL[pointer] = ucharp[i];
						bufR[pointer] = ucharp[i + 1];
					}
				}
				else if (NumChannels == 2 && BytesPerSample == 2) {
					for (i = 0; i < numSuccess; pointer++, i += 2) {
						bufL[pointer] = shortp[i];
						bufR[pointer] = shortp[i + 1];
					}
				}
			}

			dataCnt += pointer;
			return pointer;
		}

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

	class WFRException :public std::exception {};

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
			return "Unsupported format";
		}
	};


}


#endif
