#pragma once

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

struct ValueKeyPairW {
	wchar_t first[0x20]; //32 Characters.
	wchar_t second[MAX_PATH];
};

struct ValueKeyPairA {
	char first[0x20]; //32 Characters.
	char second[MAX_PATH];
};