# WavFileReader
A simple and easy WAV file reader(loader) in C++.

## How To Use
1. Include `wav_file_reader.h`.  
1. Create `gold::WavFileReader` object and use `WavFileReader.Read()` function to read data from WAV file.  

The way of using it is very similar to  `fread()` function in `stdio.h` or `cstdio`.  
```
#include"wav_file_reader.h"

void example() {
	unsigned char buf[44100];
	gold::WavFileReader wfr("test.wav");
	
	wfr.Read(buf, 44100);

	return;
}
```
See `sample.cpp` for more details.

## Examples

### Loading Data

```
	unsigned char buf[44100];

	gold::WavFileReader wfr("test.wav");

//Load first 44100 samples to buf
//Be aware that second arg is not a size but a count of elements
//Loaded values will be (L+R)/2 if the WAV format is Stereo Channels 
	wfr.Read(buf, 44100);

//Load next 44100 samples to buf
	wfr.Read(buf, 44100);
```

### Loading Left and Right Data Seperately

```
	unsigned char bufL[1000];
	unsigned char bufR[1000];

	gold::WavFileReader wfr("test.wav");

//You can also load left and right data separately
//If the format is Mono, same values will be loaded to bufL and bufR
	wfr.ReadLR(bufL, bufR, 1000);
```

### Seek in the File
```
	gold::WavFileReader wfr("test.wav");

//You can use WavFileReader.Seek() function like fseek() function in stdio.h or cstdio
//The first arg is not a bytes-based size but a samples-based size
//If the format is Stereo, the file pointer will jump to (first arg)*2 samples ahead
	wfr.Seek(5000, SEEK_CUR);
```

### Overloads of Read() and ReadLR() 
```
	int bufInt[1000];
	double bufDouble[1000];
	
	gold::WavFileReader wfr("test.wav");
	
//Read() function and ReadLR() function has several overloads
//unsigned char, signed short, int, double, float are available
	wfr.Read(bufInt, 1000);
	wfr.Read(bufDouble, 1000);
```

### WAV Format Information
```
	gold::WavFileReader wfr("test.wav");

//You can access to some format information 
	int v1 = wfr.NumChannels
	int v2 = wfr.SampleRate
	int v3 = wfr.BitsPerSample
```

## Notice
* Supported WAV format are 8bit and 16bit.
* `WavFileReader.Read()` function has several overloads that accept different data types. Convenient isn't it?
* Second arg of `WavFileReader.Read()` function is not a size of the data but a count of elements of the data.
* Loaded data will be automatically converted and rescaled from signed 16bit to unsigned 8bit by `WavFileReader.Read()` function ONLY WHEN the WAV format is 16bit and the prepared buffer is 8bit.
* `WavFileReader.Read()` function automatically adds and halves left and right channels if the format is Stereo. You can use `WavFileReader.ReadLR()` function if you want to load channels separately.
* `WavFileReader.Read()` function returns (the total number of samples successfully read)/(NumChannels). Use it to check EOF.
* You can access to some WAV format information via instance variables.
* `WavFileReader()` constructor throws WFRException object defined in `wav_file_reader.h` when some exception occured.
* `WavFileReader.Seek()` function returns the same value as `fseek()` function in `stdio.h` or `cstdio`.
