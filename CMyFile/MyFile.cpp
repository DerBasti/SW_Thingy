#include "MyFile.h"
#include "..\QuickInfos\QuickInfoFuncDefs.h"

/*

#define ATTACH_TIME_VIA(func) SYSTEMTIME st; GetSystemTime(&st); \
		func('[', handle); \
		func((st.wHour / 10) + 0x30, handle); \
		func((st.wHour % 10) + 0x30, handle); \
		func(':', handle); \
		func((st.wMinute / 10) + 0x30, handle); \
		func((st.wMinute % 10) + 0x30, handle); \
		func(':', handle); \
		func((st.wSecond / 10) + 0x30, handle); \
		func((st.wSecond % 10) + 0x30, handle); \
		func('.', handle); \
		func((st.wMilliseconds / 100 % 10) + 0x30, handle); \
		func((st.wMilliseconds % 100 / 10) + 0x30, handle); \
		func((st.wMilliseconds % 10) + 0x30, handle); \
		func(']', handle); \
		func(' ', handle);

#pragma warning(disable:4996)

void logging(CMyFile logFile, const char *func) {
#ifdef _MY_DEBUG
	if (GetLastError() != 0) {
		logFile.putStringWithVar(L"%s returned an error: (0x%x)\n", func, GetLastError());
		SetLastError(0);
	}
	else {
		logFile.putStringWithVar(L"%s was successful\n", func);
	}
#endif
}

void logging(CMyFile logFile, const wchar_t *func) {
#ifdef _MY_DEBUG
	if (GetLastError() != 0) {
		logFile.putStringWithVar(L"%w returned an error: 0x%x\n", func, GetLastError());
		SetLastError(0);
	}
	else {
		logFile.putStringWithVar(L"%w was successful\n", func);
	}
#endif
}



bool CMyFile::putString(const wchar_t *str, bool attachTime) {
	if (this->accessRights[0] == 'w' || this->accessRights[0] == 'a') {
		this->reopen();
		if (attachTime) {
			if(this->unicode) {
				ATTACH_TIME_VIA(fputwc);
			}
			else {
				ATTACH_TIME_VIA(fputc);
			}
		}
		if(this->unicode) {
			fputws(str, handle);
		}
		else {
			for(unsigned int i=0;i<wcslen(str);i++) {
				if (str[i] & 0xFF00)
					fputc(str[i]>>8&0xFF,handle);
				fputc(str[i]&0xFF,handle);
			}
		}
		this->close();
		return true;
	}
	return false;
}

bool CMyFile::putString(const char *str, bool attachTime) {
	if (this->accessRights[0] == 'w' || this->accessRights[0] == 'a') {
		this->reopen();
		if (attachTime) {
			if(this->unicode) {
				ATTACH_TIME_VIA(fputwc);
			} else {	
				ATTACH_TIME_VIA(fputc);
			}
		}
		if(this->unicode) {
			for(unsigned int i=0;i<strlen(str);i++) {
				fputc(0x00 << 8 | str[i], handle);
			}
		} else {
			fputs(str, handle);
		}
		this->close();
		return true;
	}
	return false;
}

bool CMyFile::writePlain(void *ptr, unsigned int sizeOfElement, unsigned int length) {
	if (!handle || !(this->accessRights[0] == 'w' || this->accessRights[0] == 'a'))
		return false;
	return (fwrite(ptr, sizeOfElement, length, handle) > 0);
}

bool CMyFile::putStringWithVarOnly(const char *fmt, ...) {
	if (!(this->accessRights[0] == 'w' || this->accessRights[0] == 'a'))
		return false;
	this->reopen();
	if (this->unicode) {
		ArgConverter( wResult, fmt);
		for (unsigned int i = 0; i < wResult.length(); i++) {
			if (this->accessRights[1] != 'b') //non binary
				fputwc(wResult.c_str()[i], handle);
			else {
				fputc(wResult.c_str()[i] >> 8 & 0xFF, handle);
				fputc(wResult.c_str()[i] & 0xFF, handle);
			}
		}
	}
	else {
		ArgConverterA( cResult, fmt);
		for (unsigned int i = 0; i < cResult.length(); i++) {
			fputc(cResult.c_str()[i], handle);
		}
	}
	this->close();
	return true;
}

bool CMyFile::putStringWithVarOnly(const wchar_t *fmt, ...) {
	if (!(this->accessRights[0] == 'w' || this->accessRights[0] == 'a'))
		return false;
	this->reopen();
	ArgConverter(wResult, fmt);
	for (unsigned int i = 0; i < wResult.length(); i++)
		fputwc(wResult.c_str()[i], handle);
	this->close();
	return true;
}

bool CMyFile::putStringWithVar(const char *fmt, ...) {
	if (!(this->accessRights[0] == 'w' || this->accessRights[0] == 'a'))
		return false;
	this->reopen();
	if (this->unicode) {
		ArgConverter(wResult, fmt);
		ATTACH_TIME_VIA(fputwc);
		for(unsigned int i=0;i<wResult.length();i++) {
			if (this->accessRights[1] != 'b') //non binary
				fputwc(wResult.c_str()[i], handle);
			else {
				fputc(wResult.c_str()[i] >> 8 & 0xFF, handle);
				fputc(wResult.c_str()[i] & 0xFF, handle);
			}
		}
	}
	else {
		ArgConverterA(cResult, fmt);
		for (unsigned int i = 0; i < cResult.length(); i++) {
			fputc(cResult.c_str()[i], handle);
		}
	}
	this->close();
	return true;
}

bool CMyFile::putStringWithVar(const wchar_t *fmt, ...) {
	if (!(this->accessRights[0] == 'w' || this->accessRights[0] == 'a'))
		return false;
	this->reopen();
	ArgConverter(wResult, fmt);
	ATTACH_TIME_VIA(fputwc);
	for (unsigned int i = 0; i < wResult.length(); i++) {
		if (this->accessRights[1] != 'b') //non binary
			fputwc(wResult.c_str()[i], handle);
		else {
			fputc(wResult.c_str()[i] >> 8 & 0xFF, handle);
			fputc(wResult.c_str()[i] & 0xFF, handle);
		}
	}
	this->close();
	return true;
}

bool CMyFile::exists() {
	if (wcslen(wPath) == 0 && strlen(cPath) == 0) {
		return false;
	}
	if(this->handle)
		return true;
	//path was already given, let's see whether it actually exists.
	if(this->reopen()) {
		this->close();
		return true;
	}
	return false;
}

bool CMyFile::close() {
	if (handle) {
		fclose(handle);
		this->handle = NULL;
	}
	return (handle == NULL);
}
#pragma warning(default:4996)
*/