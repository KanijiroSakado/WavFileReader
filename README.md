# WavFileReader
Simple and easy C++ reader(loader) for WAV file.

Use WavFileReader.Read function to read data from WAV file.
See sample.cpp for more detail.

# notice
* Loaded data will be automatically converted from signed 16bit to unsigned 8bit ONLY when the WAV format is 16bit and prepared buffer is 8bit.
* WavFileReader constructor throws WFRException when some exception occured.
* WavFileReader.Read functions returns (the total number of samples successfully read)/(Channels).
* WavFileReader.Seek function returns the same value as fseek function in stdio.h;
