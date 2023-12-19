#include "Header.hpp"

unsigned long UTime(){
	timeval tv;
	gettimeofday(&tv, nullptr);
	return 1000000*(unsigned long)tv.tv_sec + (unsigned long)tv.tv_usec;
}

//int descriptorSetsInitCount = 0;

