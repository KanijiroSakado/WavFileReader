#ifdef WAV_FILE_READER_H
#else
#define WAV_FILE_READER_H

#define _CRT_SECURE_NO_WARNINGS//without this, Visual Studio throws warnings for fopen function that suggests to use fopen_s instead

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<exception>


namespace gold {

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


	template<typename T> struct ReadFunctionArgumentTypeValidation;
	template<> struct ReadFunctionArgumentTypeValidation<unsigned char> {};
	template<> struct ReadFunctionArgumentTypeValidation<int> {};
	template<> struct ReadFunctionArgumentTypeValidation<short> {};
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


		WavFileReader(const char*filename) {
			numPrimaryBuf = 44100;//default
			WavFileReaderPrivate(filename);
		}

		
		WavFileReader(const char* filename, unsigned int numPrimaryBuf) {
			this->numPrimaryBuf = numPrimaryBuf;
			WavFileReaderPrivate(filename);
		}

		
		~WavFileReader() {
			fclose(fp);
			free(primaryBuf);
		}

		
		void PrintHeader() {

			printf("format size= %d\nformat ID = %d\nchannels = %d\nsampling rate = %d\nbytes per sec = %d\nblock align = %d\nbits per sample = %d\ndata size = %d\n"
				, FmtSize, FmtID, NumChannels, SampleRate, BytesPerSec, BlockAlign, BitsPerSample, DataSize);

			return;
		}

		
		template <class Type> unsigned int Read(Type *buf, unsigned int count) {

			unsigned int readCnt, numSuccess, pointer = 0,leftToRead;

			sizeof(ReadFunctionArgumentTypeValidation<Type>);
			//If you have a compiler error here, then the type of the first argument of Read function is illegal
			//Available types are [unsigned char*, int*, short*, long*, double*, float*]
			
			leftToRead = count;

			if ((NumChannels == 1 && BitsPerSample == 16 && sizeof(Type) == sizeof(short)) ||
				(NumChannels == 1 && BitsPerSample == 8 && sizeof(Type) == sizeof(unsigned char))) 
			{
				numSuccess = fread(buf, BytesPerSample, leftToRead, fp);
				dataCnt += numSuccess;
				return numSuccess;
			}

			
			if (dataCnt + leftToRead > NumData)leftToRead = NumData - dataCnt;
			readCnt = numPrimaryBuf * NumChannels;

			while (leftToRead > 0) {

				if (leftToRead < numPrimaryBuf) {
					readCnt = leftToRead * NumChannels;
					leftToRead = 0;
				}
				else {
					leftToRead -= numPrimaryBuf;
				}

				numSuccess = fread(primaryBuf, BytesPerSample, readCnt, fp);
				if (numSuccess < readCnt) leftToRead = 0;

				ArrangeDataRead(buf, numSuccess, &pointer);
				
			}
			dataCnt += pointer;
			return pointer;
		}


		template <class Type> unsigned int ReadLR(Type *bufL, Type *bufR, unsigned int count) {

			unsigned int readCnt, numSuccess, pointer = 0,leftToRead;

			sizeof(ReadFunctionArgumentTypeValidation<Type>);
			//If you have a compiler error here, then the type of the first argument of ReadLR function is illegal
			//Available types are [unsigned char*, int*, short*, long*, double*, float*]

			leftToRead = count;

			if ((NumChannels == 1 && BitsPerSample == 16 && sizeof(Type) == sizeof(short)) ||
				(NumChannels == 1 && BitsPerSample == 8 && sizeof(Type) == sizeof(unsigned char)))
			{
				numSuccess = fread(bufL, BytesPerSample, leftToRead, fp);
				memcpy(bufR, bufL, numSuccess * BytesPerSample);
				dataCnt += numSuccess;
				return numSuccess;
			}

			if (dataCnt + leftToRead > NumData)leftToRead = NumData - dataCnt;
			readCnt = numPrimaryBuf * NumChannels;

			while (leftToRead > 0) {

				if (leftToRead < numPrimaryBuf) {
					readCnt = leftToRead * NumChannels;
					leftToRead = 0;
				}
				else {
					leftToRead -= numPrimaryBuf;
				}

				numSuccess = fread(primaryBuf, BytesPerSample, readCnt, fp);
				if (numSuccess < readCnt) leftToRead = 0;

				ArrangeDataReadLR(bufL, bufR, numSuccess, &pointer);
				
			}
			dataCnt += pointer;
			return pointer;
		}

		
		int Seek(long offset, int origin) {
			if ((long)dataCnt < (-1) * offset) {//changed the type of dataCnt from uint to long, so I removed the casting dataCnt to long
				fseek(fp, (-1) * dataCnt * BlockAlign, origin);
				dataCnt = 0;
				return 1;
			}
			if ((unsigned long)(dataCnt + offset) > NumData) {
				fseek(fp, (NumData - dataCnt)*BlockAlign, origin);
				dataCnt = NumData;
				return 1;
			}
			dataCnt += offset;
			return fseek(fp, BlockAlign*offset, origin);
		}

		
		unsigned long Tell() {
			return dataCnt;
		}

		
	private:
		FILE* fp;
		unsigned int numPrimaryBuf;//count of general purpose buf = total buf size / BlockAlign
		void *primaryBuf;
		unsigned long dataCnt;
		unsigned char *ucharp;
		signed short *shortp;
		
		void WavFileReaderPrivate(const char*filename) {

			char ch[5];
			unsigned int size;//chunk size

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
			fread(ch, 1, 4, fp);
			while (strcmp(ch, "data")) {
				if (fread(&size, 4, 1, fp) != 1) {
					fclose(fp);
					throw WFRFileValidityException();
				}
				fseek(fp, size, SEEK_CUR);
				fread(ch, 1, 4, fp);
			}
			fread(&DataSize, 4, 1, fp);
			BytesPerSample = BitsPerSample / 8;
			NumData = DataSize / BlockAlign;

			if (BitsPerSample != 8 && BitsPerSample != 16) {
				fclose(fp);
				throw WFRSupportedFormatException();
			}

			if (NumChannels != 1 && NumChannels != 2) {
				fclose(fp);
				throw WFRFileValidityException();
			}

			dataCnt = 0;

			primaryBuf = (unsigned char*)malloc(BytesPerSample * numPrimaryBuf * NumChannels);
			ucharp = (unsigned char*)primaryBuf;
			shortp = (signed short*)primaryBuf;

			return;
		}


		template <class Type> void ArrangeDataRead(Type *buf, unsigned int numSuccess, unsigned int *bufPointer) {
			
			unsigned int i,pointer = *bufPointer;

			if (NumChannels == 1 && BitsPerSample == 8) {
				for (i = 0; i < numSuccess; pointer++, i++) {
					buf[pointer] = ucharp[i];
				}
			}
			else if (NumChannels == 1 && BitsPerSample == 16) {

				switch (sizeof(Type)) {
					case sizeof(unsigned char) :
						for (i = 0; i < numSuccess; pointer++, i++)
							buf[pointer] = (unsigned long)((long)shortp[i] + 0x8000) >> 9;
						break;
					default:
						for (i = 0; i < numSuccess; pointer++, i++)
							buf[pointer] = shortp[i];
				}
			}
			else if (NumChannels == 2 && BitsPerSample == 8) {

				switch (sizeof(Type)) {
					case sizeof(unsigned char) :
						for (i = 0; i < numSuccess; pointer++, i += 2)
							buf[pointer] = ((unsigned long)ucharp[i] + (unsigned long)ucharp[i + 1]) >> 1;
						break;
					default:
						for (i = 0; i < numSuccess; pointer++, i += 2)
							buf[pointer] = ((long)ucharp[i] + (long)ucharp[i + 1]) / 2;
				}
			}
			else if (NumChannels == 2 && BitsPerSample == 16) {

				switch (sizeof(Type)) {
					case sizeof(unsigned char) :
						for (i = 0; i < numSuccess; pointer++, i += 2)
							buf[pointer] = (unsigned long)((long)shortp[i] + (long)shortp[i + 1] + 0x10000) >> 9;
						break;
					default:
						for (i = 0; i < numSuccess; pointer++, i += 2)
							buf[pointer] = ((long)shortp[i] + (long)shortp[i + 1]) / 2;
				}
			}
			*bufPointer = pointer;
		}


		template <class Type> void ArrangeDataReadLR(Type *bufL, Type *bufR, unsigned int numSuccess, unsigned int* bufPointer) {
			
			unsigned int i,pointer = *bufPointer;

			if (NumChannels == 1 && BitsPerSample == 8) {

				for (i = 0; i < numSuccess; pointer++, i++) {
					bufL[pointer] = ucharp[i];
					bufR[pointer] = ucharp[i];
				}
			}
			else if (NumChannels == 1 && BitsPerSample == 16) {

				switch (sizeof(Type)) {
					case sizeof(unsigned char) :
						for (i = 0; i < numSuccess; pointer++, i++) {
							bufL[pointer] = (unsigned long)((long)shortp[i] + 0x8000) >> 8;
							bufR[pointer] = (unsigned long)((long)shortp[i] + 0x8000) >> 8;
						}
											   break;
					default:
						for (i = 0; i < numSuccess; pointer++, i++) {
							bufL[pointer] = shortp[i];
							bufR[pointer] = shortp[i];
						}
				}
			}
			else if (NumChannels == 2 && BitsPerSample == 8) {

				for (i = 0; i < numSuccess; pointer++, i += 2) {
					bufL[pointer] = ucharp[i];
					bufR[pointer] = ucharp[i + 1];
				}
			}
			else if (NumChannels == 2 && BitsPerSample == 16) {

				switch (sizeof(Type)) {
					case sizeof(unsigned char) :
						for (i = 0; i < numSuccess; pointer++, i += 2) {
							bufL[pointer] = ((long)shortp[i] + 0x8000) >> 8;
							bufR[pointer] = ((long)shortp[i + 1] + 0x8000) >> 8;
						}
											   break;
					default:
						for (i = 0; i < numSuccess; pointer++, i += 2) {
							bufL[pointer] = shortp[i];
							bufR[pointer] = shortp[i + 1];
						}
				}
			}

			*bufPointer = pointer;
		}

	};

	
}


#endif
