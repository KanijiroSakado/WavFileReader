# WavFileReader
Simple and easy WAV file reader(loader) in C++.

Use WavFileReader.Read() function to read data from WAV file.  
See sample.cpp for more detail.

## Notice
* Loaded data will be automatically converted from signed 16bit to unsigned 8bit ONLY when the WAV format is 16bit and prepared buffer is 8bit.
* WavFileReader() constructor throws WFRException object defined in wav_file_reader.h when some exception occured.
* WavFileReader.Read() functions returns (the total number of samples successfully read)/(Channels).
* WavFileReader.Seek() function returns the same value as fseek() function in cstdio.
