#include"wav_file_reader.h"
#include<cstdlib>


void example() {

	WavFileReader wfr("test.wav");
	
	unsigned char *buf = (unsigned char*)malloc(sizeof(unsigned char) * 44100);

	//Load first 44100 samples to buf
	//Loaded values will be (L+R)/2 if the file format is Stereo Channels 
	wfr.Read(buf, 44100);

	//Load next 44100 samples to buf
	wfr.Read(buf, 44100);

	free(buf);

	return;
}

void advanced_example() {

	//The performance will be better if the second arg of the constructor is set to the same value as which is used when loading  
	//The arg is related to the size of internal buffer
	WavFileReader wfr("test.wav",44100);

	//You can also load left and right data separately
	//If the format is Mono, same values will be loaded to bufL and bufR
	unsigned char *bufL = (unsigned char*)malloc(sizeof(unsigned char) * 44100);
	unsigned char *bufR = (unsigned char*)malloc(sizeof(unsigned char) * 44100);

	wfr.ReadLR(bufL, bufR, 44100);

	//You can use WavFileReader.Seek() function like fseek() function in stdio.h
	//The first arg is not a bytes-based size but a sample-based size
	//If the format is Stereo, the file pointer will jump to (first arg)*2 samples ahead
	wfr.Seek(100000, WAV_SEEK_CUR);


	//Read function and ReadLR function has several overloads
	int *bufInt = (int*)malloc(sizeof(int) * 44100);
	double *bufDouble = (double*)malloc(sizeof(double) * 44100);

	wfr.Read(bufInt, 44100);
	wfr.Read(bufDouble, 44100);

	free(bufL);
	free(bufR);
	free(bufInt);
	free(bufDouble);

	return;
}



int main(void) {

	//This prgram only do loading data from file
	//So nothing exciting will happen
	example();
	advanced_example();
	
	getc(stdin);
	return 0;
}






