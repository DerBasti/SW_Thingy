#pragma once

#ifndef __MYFILE__
#define __MYFILE__

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>
#include <time.h>
#include <vector>

#include "..\QuickInfos\VarsToString"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned long dword_t;

#pragma warning(disable:4996)

class CMyFile
{
	protected:
		FILE *handle;
		virtual ~CMyFile() {
			this->close();
		}
		bool isUnicode;
	public:
		CMyFile() { }
		bool exists() {
			return this->reopen();
		}
		virtual bool reopen() {
			return false;
		}
		bool close() {
			if (this->handle) {
				fclose(this->handle);
				this->handle = nullptr;
				return true;
			}
			return false;
		}
}; 
	
template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value>::type > class CMyFileReader : public CMyFile {
	protected:
		typedef typename std::basic_string<_Ty, std::char_traits<_Ty> > volatile_typed_string;
		volatile_typed_string filePath;
		volatile_typed_string accessRights;

		typedef FILE* (__cdecl* OpenFileType)(const _Ty* path, const _Ty* access);
		OpenFileType openFile;

		void determineOpenFileType() {
			if (this->isUnicode) {
				this->openFile = (OpenFileType)::_wfopen;
				return;
			}
			this->openFile = (OpenFileType)::fopen;
		}
		CMyFileReader() {
			this->isUnicode = std::is_same<wchar_t, _Ty>::value;
			this->determineOpenFileType();
			this->handle = nullptr;
		}
	public:
		CMyFileReader(const _Ty* filePath, bool binaryMode = true) {
			this->isUnicode = std::is_same<wchar_t, _Ty>::value;
			this->determineOpenFileType();

			this->filePath = volatile_typed_string(filePath);
			this->accessRights = (const _Ty*)"r";
			this->accessRights += binaryMode ? (const _Ty*)"b" : (const _Ty*)"";
			this->handle = this->openFile(this->filePath.c_str(), this->accessRights.c_str());
		}
		~CMyFileReader() {
		}

		bool reopen(){
			if (!this->handle) {
				this->handle = this->openFile(this->filePath.c_str(), this->accessRights.c_str());
			}
			return this->handle != nullptr;
		}

		__inline void setPosition(const dword_t pos) {
			if (!this->handle)
				return;
			fseek(this->handle, pos, SEEK_SET);
		}

		dword_t skip(const dword_t len) {
			if (!this->handle)
				return -1;
			dword_t previousPos = ftell(this->handle);
			fseek(this->handle, previousPos + len, SEEK_SET);
			return static_cast<dword_t>(ftell(this->handle) - previousPos);
		}
		
		template<class _numType, class = typename std::enable_if< std::is_arithmetic<_numType>::value>::type> const _numType read(const DWORD bytesToSkipAfterReading = 0x00) {
			if (!this->handle) {
				return _numType(-1);
			}

			if (bytesToSkipAfterReading > 0) {
				this->skip(bytesToSkipAfterReading);
			}

			_numType res = 0x00;
			fread(&res, sizeof(_numType), 1, this->handle);
			return _numType(res);
		}
		template<class _TyLen = byte_t> _TyLen readLengthThenString(_Ty* buf) {
			_TyLen len = 0x00;
			fread(&len, sizeof(_TyLen), 1, this->handle);
			fread(buf, 1, len, this->handle);
			buf[len] = 0x00;
			return len;
		}
		template<class _TyLen> _TyLen readStringWithGivenLength(_TyLen len, _Ty* buf) {
			_TyLen readBytes = fread(buf, 1, len, this->handle);
			buf[len] = 0x00;
			return readBytes;
		}
		volatile_typed_string readLine() {
			volatile_typed_string result = "";
			int curChar = 0;
			if (this->isUnicode) {
				while (feof(this->handle) != EOF) {
					curChar = fgetwc(this->handle);
					if (curChar == '\n')
						break;
					result += curChar;
				}
			}
			else {
				while (feof(this->handle) != EOF) {
					curChar = fgetc(this->handle);
					if (curChar == '\n')
						break;
					result += curChar;
				}
			}
			return result;
		}
		bool getLine(_Ty *ptr, dword_t ptrLen, DWORD line = 0) {
			if (this->handle || this->reopen()) {
				fseek(this->handle, 0, SEEK_END);
				dword_t maxLen = (dword_t)ftell(this->handle);
				rewind(this->handle);

				dword_t currentLine = 0x00;
				int curChar = 0x00;
				bool readsBinary = this->accessRights.find((const _Ty*)"b") >= 0;
				for (unsigned int i = 0; i < maxLen; i++) {
					if (line == currentLine) {
						for (unsigned int j = i, k=0; j<maxLen && k < ptrLen; j++, k++) {
							
							if (!this->isUnicode) {
								ptr[k] = fgetc(this->handle);
							}
							else {
								if (!readsBinary)
									ptr[k] = fgetwc(this->handle);
								else
									ptr[k] = (fgetc(this->handle) | fgetc(this->handle)<<8);
							}
							if (ptr[k] == 0x0A)
								break;
						}
						return true;
					}
					else {
						if (!this->isUnicode) {
							curChar = fgetc(this->handle);
						}
						else {
							curChar = fgetwc(this->handle);
						}

						if (curChar == 0x0A) {
							currentLine++;
						}
					}
				}
			}
			return false;
		}
		const dword_t getLineAmount() {
			dword_t result = 1;
			if (this->handle || this->reopen()) {
				fseek(this->handle, 0, SEEK_END);
				dword_t maxLen = (dword_t)ftell(this->handle);
				rewind(this->handle);

				int curChar = 0;
				for (unsigned int j = 0; j < maxLen; j++) {
					if (!this->isUnicode) {
						curChar = fgetc(this->handle);
					}
					else {
						curChar = fgetwc(this->handle);
					}
					if (curChar == 0x0A)
						result++;
				}
			}
			return result;
		}

		virtual __inline const dword_t getTotalSize() const { int pos = ftell(this->handle); int retVal = fseek(this->handle, 0, SEEK_END); fseek(this->handle, pos, SEEK_SET); return retVal; }
};
template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value>::type > class CMyBufferedFileReader {
private:
	typedef typename std::basic_string<_Ty, std::char_traits<_Ty> > volatile_typed_string;
	struct {
		const _Ty *data;
		dword_t length;
		dword_t cursor;
	} content;
public:
	CMyBufferedFileReader(const _Ty* cont, const dword_t length) {
		if (cont && length > 0) {
			this->content.length = length;
			this->content.data = cont;
		}
		else {
			this->content.length = 0;
			this->content.data = nullptr;
		}
		this->content.cursor = 0;
	}
	~CMyBufferedFileReader() {
		this->content.data = nullptr;
		this->content.length = 0;
	}
	bool reopen() { 
		this->content.cursor = 0;
		return this->content.length>0; 
	}
	template<class _numType, class = typename std::enable_if< std::is_arithmetic<_numType>::value>::type> const _numType read(const DWORD bytesToSkipBeforeReading = 0x00) {
		if (!this->content.data || ((this->content.cursor + sizeof(_numType)) > this->content.length)) {
			return _numType(0);
		}
		if (bytesToSkipBeforeReading)
			this->skip(bytesToSkipBeforeReading);
		const _numType res = *reinterpret_cast<const _numType*>(&this->content.data[this->content.cursor]);
		this->content.cursor += sizeof(_numType);

		return res;
	}
	template<class _numType, class = typename std::enable_if< std::is_integral<_numType>::value>::type> void readAndAlloc(_Ty** buf, const _numType length) {
		_Ty*& realBuf = *(buf);
		realBuf = new _Ty[length+1];
		for (_numType i = 0; i < length && this->content.cursor < this->content.length; i++) {
			realBuf[i] = this->content.data[this->content.cursor];
			this->content.cursor++;
		}
		realBuf[length] = 0x00;
	}

	template<class _TyLen = byte_t> _TyLen readLengthThenString(_Ty* buf) {
		if (!buf || !this->content.data)
			return _TyLen(0);

		_TyLen strLen = this->read<_TyLen>();
		return this->readStringWithGivenLength(strLen, buf);
	}

	template<class _TyLen = byte_t> _TyLen readStringWithGivenLength(_TyLen length, _Ty* buf) {
		if (length <= 0 || !buf || !this->content.data)
			return _TyLen(-1);
		for (_TyLen i = 0; i < length && this->content.cursor < this->content.length; i++) {
			buf[i] = this->content.data[this->content.cursor];
			this->content.cursor++;
		}
		buf[length] = 0x00;
		return length;
	}

	std::string readStringUntilZero() {
		char buf[0x400] = { 0x00 };
		word_t len = 0;
		for (unsigned int i = 0; this->content.cursor < this->content.length && i < 0x400 && this->content.data[this->content.cursor] != 0; i++, this->content.cursor++) {
			buf[len] = this->content.data[this->content.cursor];
			len++;
		}
		//Increment it past the 0x00
		this->content.cursor++;
		buf[len] = 0x00;
		return std::string(buf);
	}
	std::string readLine() {
		std::string result = "";
		for (unsigned int i = 0; this->content.cursor < this->content.length && i < 0x400 && this->content.data[this->content.cursor] != '\n'; i++, this->content.cursor++) {
			result += this->content.data[this->content.cursor];
			len++;
		}
		//Increment it past the 0x00
		this->content.cursor++;
		return result;
	}
	bool getLine(_Ty *ptr, dword_t ptrLen, dword_t line = 0) const {
		dword_t curLine = 0;
		for (unsigned int i = 0; i < this->content.length; i++) {
			if (curLine == line) {
				for (unsigned int j = 0; j < ptrLen && (i + j) < this->content.length; j++) {
					ptr[j] = this->content.data[i + j];
				}
				return true;
			}
			_Ty curChar = this->content.data[i];
			if (curChar == 0x0A) {
				curLine++;
			}
		}
		return false;
	}
	const dword_t getLineAmount() const {
		dword_t curLine = 0;
		for (unsigned int i = 0; i < this->content.length; i++) {
			_Ty curChar = this->content.data[i];
			if (curChar == 0x0A) {
				curLine++;
			}
		}
		return curLine;
	}
	__inline void setPosition(const dword_t pos) { this->content.cursor = pos; }
	__inline const dword_t getPosition() const { return this->content.cursor; }
	__inline dword_t skip(const dword_t len) { return (this->content.cursor += len); }
	__inline const dword_t getTotalSize() const { return this->content.length; }
	__inline bool exists() { return (this->content.data != nullptr && this->content.length > 0); }
};

template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value>::type > class CMyFileWriter : public CMyFile {
	private:
		typedef typename std::basic_string<_Ty, std::char_traits<_Ty> > volatile_typed_string;
		volatile_typed_string filePath;
		volatile_typed_string accessRights;

		void establishAccessRightByFlags(bool appendMode, bool createIfNonExistent, bool binaryMode) {

			//In case we're appending --> a, otherwise regular (over-)write
			this->accessRights = appendMode ? (const _Ty*)"a" : (const _Ty*)"w";

			//In case we want to be able to write in binary --> add a 'b'
			this->accessRights += binaryMode ? (const _Ty*)"b" : (const _Ty*)"";

			//In case we want to create the file if it wasn't existent before --> add a '+'
			this->accessRights += createIfNonExistent ? (const _Ty*)"+" : (const _Ty*)"";
		}

		//Easier way of handling the opening of files, as they don't really vary in their args
		typedef FILE* (__cdecl* OpenFileType)(const _Ty* path, const _Ty* access);
		OpenFileType openFile;
		
		typedef int (__cdecl* FormatConvertType)(_Ty* buf, const _Ty* fmt, ...);
		FormatConvertType sprintFormat;

		typedef int(__cdecl* PutStringFunction)(const _Ty* pBuf, FILE* handle);
		PutStringFunction putStringFunc;

		typedef _Ty*(__cdecl* IntToStringType)(int value, _Ty* buf, int radix);
		//typedef errno_t(__cdecl* IntToStringType)(int value, _Ty* buf, size_t Size, int radix);
		IntToStringType intToStringReader;

		void determineOpenFileType() {
			if (this->isUnicode) {
				this->openFile = (OpenFileType)::_wfopen;
				this->sprintFormat = (FormatConvertType)::wsprintf;
				this->putStringFunc = (PutStringFunction)::fputws; 
				this->intToStringReader = reinterpret_cast<IntToStringType>(::_itow);
				return;
			}
			this->openFile = (OpenFileType)::fopen;
			this->sprintFormat = (FormatConvertType)::sprintf;
			this->putStringFunc = (PutStringFunction)::fputs;
			this->intToStringReader = (IntToStringType)::_itoa;
		}
		void attachTime() {
			time_t t = time(nullptr);
			struct tm *timeNow = localtime(&t);

			_Ty buf[0x80]; const wchar_t *wFmt = L"[%02i:%02i:%02i] "; const char* fmt = "[%02i:%02i:%02i] ";
			sprintFormat(buf, (this->isUnicode ? (const _Ty*)wFmt : (const _Ty*)fmt), timeNow->tm_hour, timeNow->tm_min, timeNow->tm_sec);
			
			this->putString(buf);
			this->reopen();
		}
#ifdef max
#undef max
#endif
		void _convertLen(word_t curLen, volatile_typed_string& result) {
			if (curLen != std::numeric_limits<word_t>::max()) {
				_Ty buffer[0x40] = { 0x00 };
				this->intToStringReader(curLen, buffer, 10);
				result += buffer;
			}
		}

		volatile_typed_string convertVAArgs(const _Ty* fmt, va_list args) {
			volatile_typed_string result;
			_Ty buffer[0x40] = { 0x00 };
			word_t minLen[2] = { std::numeric_limits<word_t>::max(), std::numeric_limits<word_t>::max() };
			byte_t minLenPos = 0;

			word_t minPres = 0;
			while (*fmt) {
				if (*fmt == '%') {
					*fmt++;
					if (*fmt >= '0' && *fmt <= '9') {
						while (*fmt >= '0' && *fmt <= '9' && minLenPos<2) {
							minLen[minLenPos] = *fmt - 0x30;
							minLenPos++;
							*fmt++;
						}
					}
					minLenPos = 0;
					if (*fmt == '.') {
						*fmt++;
						while (*fmt >= '0' && *fmt <= '9') {
							minPres = minPres + *fmt - 0x30;
							*fmt++;
						}
					}
					volatile_typed_string stringWithFmt = (_Ty*)"%";
					switch (*fmt++) {
						case 0:
							result += stringWithFmt;
						break;
						case 'c':
							result += (_Ty)va_arg(args, _Ty);
						break;
						case 'd':
						case 'i':
							this->_convertLen(minLen[0], stringWithFmt);
							this->_convertLen(minLen[1], stringWithFmt);
							stringWithFmt += (_Ty*)"i";

							this->sprintFormat(buffer, stringWithFmt.c_str(), (int)va_arg(args, int));

							result += buffer;
						break;
						case 'f':
							this->_convertLen(minLen[0], stringWithFmt);
							this->_convertLen(minLen[1], stringWithFmt);
							if (minPres > 0) {
								stringWithFmt += (_Ty*)".";
								this->_convertLen(minPres, stringWithFmt);
							}
							stringWithFmt += (_Ty*)"f";

							this->sprintFormat(buffer, stringWithFmt.c_str(), (double)va_arg(args, double));

							result += buffer;
						break;
						case 's':
						{
							std::string res = (char*)va_arg(args, char*);
							result += volatile_typed_string(res.begin(), res.end());
						}
						break;
						case 'w':
						{
							std::wstring res = (wchar_t*)va_arg(args, wchar_t*);
							result += volatile_typed_string(res.begin(), res.end());
						}
						break;
						case 'x':
							this->_convertLen(minLen[0], stringWithFmt);
							this->_convertLen(minLen[1], stringWithFmt);
							stringWithFmt += (_Ty*)"x";

							this->sprintFormat(buffer, stringWithFmt.c_str(), (unsigned int)va_arg(args, unsigned int));

							result += buffer;

						break;
						default:
							result += (_Ty*)"%";
							result += *(fmt - 1);
					} 
				}
				else {
					result += *fmt;
					*fmt++;
				}
			}
			return result;
		}
	public:
		CMyFileWriter(const _Ty* filePath, bool appendMode = true, bool createIfNonExistent = true, bool binaryMode = true) {
			this->establishAccessRightByFlags(appendMode, createIfNonExistent, binaryMode);
			this->filePath = volatile_typed_string(filePath);
			this->isUnicode = std::is_same<wchar_t, _Ty>::value;
			this->determineOpenFileType();

			this->handle = this->openFile(this->filePath.c_str(), this->accessRights.c_str());
			/*if (GetLastError() == 0xb7) {
				SetLastError(0);
			}*/
		}
		~CMyFileWriter() {
		}

		bool reopen() {
			if (!this->handle) {
				this->handle = this->openFile(this->filePath.c_str(), this->accessRights.c_str());
				return this->handle != nullptr;
			}
			return false;
		}
		void clear() {
			if (this->handle) {
				this->close();
			}
			this->handle = this->openFile(this->filePath.c_str(), (const _Ty*)"w");
			this->close();
		}

		bool writePlain(void *pBuf, unsigned int sizePerItem, unsigned int itemAmount) {
			if (this->handle || this->reopen()) {
				unsigned int byteWritten = fwrite(pBuf, sizePerItem, itemAmount, this->handle);
				this->close();
				if (byteWritten != (sizePerItem * itemAmount))
					return false;
				return true;
			}
			return false;
		}
		bool putString(const std::string& str, bool attachTime = false) {
			return this->putString(str.c_str(), attachTime);
		}
		bool putString(const _Ty* str, bool attachTime = false) {
			if (this->handle || this->reopen()) {
				if (attachTime) {
					this->attachTime();
				}
				this->putStringFunc(str, this->handle);
				this->close();
				return true;
			}
			return false;
		}
		bool putStringWithVar(const _Ty* strWithFmt, ...) {
			if (this->handle || this->reopen()) {
				_Ty buf[0x200] = { 0x00 };
				va_list arg; va_start(arg, strWithFmt);
				volatile_typed_string result = this->convertVAArgs(strWithFmt, arg);
				va_end(arg);
				this->putStringFunc(result.c_str(), this->handle);
			}
			return false;
		}
};

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

//#define putStringWithVar putStringWithVarAndTime

#pragma warning(default:4996)

#endif //__MYFILE__