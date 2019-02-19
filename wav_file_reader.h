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


		WavFileReader(const char*filename) {
			numGpBuf = 44100;//default
			WavFileReaderPrivate(filename);
		}

		
		WavFileReader(const char* filename, unsigned int gpBufCnt) {
			this->numGpBuf = gpBufCnt;
			WavFileReaderPrivate(filename);
		}

		
		~WavFileReader() {
			fclose(fp);
			free(gpBuf);
		}

		
		void PrintHeader() {

			printf("format size= %d\nformat ID = %d\nchannels = %d\nsampling rate = %d\nbytes per sec = %d\nblock align = %d\nbits per sample = %d\ndata size = %d\n"
				, FmtSize, FmtID, NumChannels, SampleRate, BytesPerSec, BlockAlign, BitsPerSample, DataSize);

			return;
		}

		
		unsigned int Read(signed short *buf, unsigned int count) {

			unsigned int readCnt, numSuccess, leftToRead, i = 0, pointer = 0;

			leftToRead = count;

			if (NumChannels == 1 && BytesPerSample == 2) {
				numSuccess = fread(buf, BytesPerSample, leftToRead, fp);
				dataCnt += numSuccess;
				return numSuccess;
			}

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

		
		unsigned int Read(unsigned char *buf, unsigned int count) {

			unsigned int readCnt, numSuccess, leftToRead, i = 0, pointer = 0;

			leftToRead = count;

			if (NumChannels == 1 && BytesPerSample == 1) {
				numSuccess = fread(buf, BytesPerSample, leftToRead, fp);
				dataCnt += numSuccess;
				return numSuccess;
			}

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

				if (NumChannels == 1 && BytesPerSample == 2) {
					for (i = 0; i < numSuccess; pointer++, i++) {
						buf[pointer] = (unsigned long)((long)shortp[i] + 0x8000) >> 9;
					}
				}
				else if (NumChannels == 2 && BytesPerSample == 1) {
					for (i = 0; i < numSuccess; pointer++, i += 2) {
						buf[pointer] = ((unsigned long)ucharp[i] + (unsigned long)ucharp[i + 1]) >> 1;
					}
				}
				else if (NumChannels == 2 && BytesPerSample == 2) {
					for (i = 0; i < numSuccess; pointer++, i += 2) {
						buf[pointer] = (unsigned long)((long)shortp[i] + (long)shortp[i + 1] + 0x10000) >> 9;
					}
				}
			}
			dataCnt += pointer;
			return pointer;
		}
		
		
		template <class Type> unsigned int Read(Type *buf, unsigned int count) {

			unsigned int readCnt, numSuccess, i = 0, pointer = 0,leftToRead;

			sizeof(ReadFunctionArgumentTypeValidation<Type>);
			//If you have a compiler error here, then the type of the first argument of Read function is illegal
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

		
		unsigned int ReadLR(signed short *bufL, signed short *bufR, unsigned int count) {

			unsigned int readCnt, numSuccess, leftToRead, i = 0, pointer = 0;

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
					memcpy(bufL + pointer, shortp, numSuccess * 2);
					memcpy(bufR + pointer, shortp, numSuccess * 2);
					pointer += numSuccess;
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
		
		
		unsigned int ReadLR(unsigned char *bufL, unsigned char *bufR, unsigned int count) {

			unsigned int readCnt, numSuccess, leftToRead, i = 0, pointer = 0;

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
					memcpy(bufL + pointer, ucharp, numSuccess);
					memcpy(bufR + pointer, ucharp, numSuccess);
					pointer += numSuccess;
				}
				else if (NumChannels == 1 && BytesPerSample == 2) {
					for (i = 0; i < numSuccess; pointer++, i++) {
						bufL[pointer] = (unsigned long)((long)shortp[i] + 0x8000) >> 8;
						bufR[pointer] = (unsigned long)((long)shortp[i] + 0x8000) >> 8;
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
						bufL[pointer] = ((long)shortp[i] + 0x8000) >> 8;
						bufR[pointer] = ((long)shortp[i + 1] + 0x8000) >> 8;
					}
				}
			}
			dataCnt += pointer;
			return pointer;
		}
		
		
		template <class Type> unsigned int ReadLR(Type *bufL, Type *bufR, unsigned int count) {

			unsigned int readCnt, numSuccess, i = 0, pointer = 0,leftToRead;

			sizeof(ReadFunctionArgumentTypeValidation<Type>);
			//If you have a compiler error here, then the type of the first argument of ReadLR function is illegal
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
		unsigned int numGpBuf;//count of general purpose buf = total buf size / BlockAlign
		void *gpBuf;
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

			gpBuf = (unsigned char*)malloc(BytesPerSample * numGpBuf * NumChannels);
			ucharp = (unsigned char*)gpBuf;
			shortp = (signed short*)gpBuf;

			return;
		}

	};

	
}


#endif
