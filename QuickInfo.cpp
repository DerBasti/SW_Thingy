#include <ctime>
#include "QuickInfo.h"
#include "D:\Programmieren\MessageBox\MessageBox.h"

namespace QuickInfo {

	const wchar_t *windowName = nullptr;
	const wchar_t *windowExeName = nullptr;
#ifndef WINXP_DEF
	procEnumProcessModulesExPtr procEnumProcessModulesEx = (procEnumProcessModulesExPtr)GetProcAddress(LoadLibrary(L"Psapi.dll"), "EnumProcessModulesEx");
#else
	procEnumProcessModulesExPtr procEnumProcessModulesEx = nullptr;
#endif
	HWND hWindow;

	//Not very pretty, but it works for every compiler type
	template<typename _funcType> void executeFunctions(const size_t num, ...) {
		va_list ap;
		_funcType func;
		va_start(ap, num);

		BYTE curNum = 0x00;
		while( curNum < num ) {
			_funcType& func = va_arg(ap, _funcType);
			func();
			curNum++;
		}
		va_end(ap);
	}


	void replaceTermInString(std::string& textBlock, const char* termToSearchFor, const char* replacementForSearchTerm) {
		unsigned int position = 0x00; 
		unsigned int termLength = strlen(termToSearchFor);
		unsigned int replacementLength = strlen(replacementForSearchTerm);
		while ((position = textBlock.find(termToSearchFor, (position == 0 ? 0 : position + replacementLength))) < MAXDWORD) {
			textBlock = textBlock.substr(0, position) + replacementForSearchTerm + textBlock.substr(position + termLength);
		}
	}
	void removeTermFromString(std::string& textBlock, const char* term) {
		unsigned int positionStart = 0x00;
		unsigned int strOffset = strlen(term);

		while ((positionStart = textBlock.find(term)) < MAXDWORD) {
			textBlock = textBlock.substr(0, positionStart) + textBlock.substr(positionStart + strOffset);
		}
	}
	void removeTermFromString(std::string& textBlock, const char* termStart, const char* termEnd) {
		unsigned int positionStart = 0x00; 
		unsigned int positionEnd = 0x00; 
		unsigned int strOffset = strlen(termEnd);
		
		while ((positionStart = textBlock.find(termStart)) < MAXDWORD && (positionEnd = textBlock.find(termEnd, positionStart + 1)) < MAXDWORD) {
			textBlock = textBlock.substr(0, positionStart) + textBlock.substr(positionEnd + strOffset);
		}
	}

	bool substringExists(const char* buffer, const char* termToLookFor, bool caseSensitive) {
		if (!buffer || !termToLookFor)
			return false;
		dword_t bufferLength = strlen(buffer);
		dword_t termLength = strlen(termToLookFor);
		if (termLength > bufferLength)
			return false;
		dword_t b_cursor = 0, t_cursor = 0;
		std::function<bool(const char*, const char*, std::function<int(int)>)> comparison = [&](const char* buf, const char* term, std::function<int(int)>& applyFunction) {
			for (; t_cursor + b_cursor < bufferLength && t_cursor < termLength; t_cursor++, b_cursor++) {
				if (applyFunction(buf[b_cursor]) != applyFunction(term[t_cursor]))
					return false;
			}
			return true;
		};
		//std::function<int(int)> toLowerFunction = [](int c) { return tolower(c); };
		std::function<int(int)> blankFunction = [](int c) {
			return c;
		};
		while (b_cursor + termLength <= bufferLength && buffer[b_cursor] != 0) {
			if (!caseSensitive) {
				if (tolower(buffer[b_cursor]) == tolower(termToLookFor[t_cursor])) {
					if (comparison(buffer, termToLookFor, tolower))
						return true;
					t_cursor = 0;
				}
			}
			else {
				if (buffer[b_cursor] == termToLookFor[t_cursor]) {
					if (comparison(buffer, termToLookFor, blankFunction))
						return true;
					t_cursor = 0;
				}
			}
			b_cursor++;
		}
		return false;
	}
	
	void wstringAsCurrency(const std::wstring& data, DWORD& result) {
		result = 0x00; bool comma = false; unsigned int hasCurrencyValue = 0;
		for(unsigned int i=0;i<data.length();i++) {
			const wchar_t& ref = data.at(i);
			if(ref >= '0' && ref <= '9') {
				if(comma) {
					hasCurrencyValue++;
				}
				result = result * 10 + (ref - 0x30);
			}
			if((ref == '.' || ref == ',') && !comma) {
				comma = true;
			}
			if(hasCurrencyValue == 2)
				return;
		}
		result = result * static_cast<DWORD>(pow(10.0, static_cast<int>(2 - hasCurrencyValue)));
	}
	
	void wstringAsFloat(const std::wstring& data, float& toReturn) {
		toReturn = 0.0f; bool isAfterComma = false;
		for(int i = 0, j = 1;i<static_cast<int>(data.length()) ;i++) {
			const wchar_t& ref = data.at(i);
			float afterCommaNum = 0.0f;
			if(ref >= '0' && ref <= '9') {
				if(!isAfterComma) {
					toReturn = toReturn * 10 + (ref - 0x30);
				}
				else {
					afterCommaNum = static_cast<float>((ref - 0x30) / pow(10.0f, j));
					toReturn += afterCommaNum;
					j++;
				}
			}
			if(ref == '.' || ref == ',') {
				if(isAfterComma)
					break;
				isAfterComma = true;
			}
		}
		return;
	}
	
	void wstringAsInt(const std::wstring& data, DWORD& result, bool ignoreComma) {
		result = static_cast<DWORD>(0x00);
		for(unsigned int i=0;i<data.length();i++) {
			const wchar_t& ref = data.at(i);
			if(ref >= '0' && ref <= '9')
				result = static_cast<DWORD>(result * 10) + static_cast<DWORD>(ref - 0x30);
			else if((ref == '.' || ref == ',') && ignoreComma)
				continue;
			else
				return;
		}	
		return;
	}

	void floatAsInt(const float& toConvert, DWORD& result) {
		result = static_cast<DWORD>(toConvert * 10000);
		if(result % 10 >= 5)
			result += 10 - (result%10);
		else
			result -= (result % 10);
		//result = static_cast<_Ty>(result * 10) + static_cast<_Ty>
		
		return;
	}
	float intAsFloat(const DWORD& toConvert) {
		return float(static_cast<float>(toConvert));
	}

	BYTE getOperatingSystem() {
		OSVERSIONINFO OS;
		OS.dwOSVersionInfoSize = sizeof(OS);
		GetVersionEx(&OS);
		switch(OS.dwPlatformId) {
			case 0:
				return OS_Windows_31;
			break;
			case 1:
				switch (OS.dwMinorVersion)
				{
					case 0:
						return OS_Windows_95;
					break;
					case 10:
						return OS_Windows_98;
					break;
					case 98:
						return OS_Windows_ME;
					break;
				}
			break;
			case 2:
				switch (OS.dwMajorVersion)
				{
					case 3:
					case 4:
						return OS_Windows_NT;
					break;
					case 5:
						switch (OS.dwMinorVersion)
						{
							case 0:
								return OS_Windows_2000;
							break;
							case 1:
								return OS_Windows_XP;
							break;
						}
					break;
					case 6:
						switch (OS.dwMinorVersion)
						{
							case  0:
								return OS_Windows_Vista;
							break;
							case 1:
								return OS_Windows_7;
							break;
						}
					break;
				}
			break;
		}
		return OS_Windows_31;
	}

	bool is64BitSystem() {
		int cpuInfo[4];
		__cpuid(cpuInfo, 0);
		return ((cpuInfo[3] & 0x20000000) > 0);
	}

	bool is64OperatingSystem() {
		wchar_t path[MAX_PATH] = { 0x00 };

		//If it's a 64bit system, it has a WOW64-dir. Otherwise the call will fail
		if (GetSystemWow64Directory(path, MAX_PATH) == 0 && GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
			return false;
		return true;
	}

	bool is64BitModule(DWORD threadId) {
		HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, threadId);
		if (!process)
			return false;
		bool result = is64BitModule(process);
		CloseHandle(process);
		return result;
	}

	bool is64BitModule(HANDLE process) {
		if (!process)
			return false;
		wchar_t moduleName[MAX_PATH] = { 0x00 };
		GetProcessImageFileName(process, moduleName, MAX_PATH);

		std::wstring realModuleName = std::wstring(moduleName);
		convertImageNameToRealName(&realModuleName);
		return is64BitModule(realModuleName.c_str());
	}

	bool is64BitModule(HMODULE module) {
		if (!module)
			return false;
		wchar_t moduleName[MAX_PATH] = { 0x00 };
		GetModuleFileName(module, moduleName, MAX_PATH);
		return is64BitModule(moduleName);
	}

	bool is64BitModule(const wchar_t *moduleName) {
		if (!moduleName || wcslen(moduleName) < 4)
			return false;
		DWORD binaryType = 0;
		GetBinaryType(moduleName, &binaryType);
		return (binaryType == SCS_64BIT_BINARY);
	}

	void getClipboardData(std::wstring& dataString) {
		dataString = L"";
		if (!OpenClipboard(nullptr))
			return;
		HANDLE handlePtr = GetClipboardData(CF_UNICODETEXT);
		if (!handlePtr) {
			CloseClipboard();
			return;
		}
		wchar_t* dataPtr = (wchar_t*)GlobalLock(handlePtr);
		dataString = dataPtr;
		GlobalUnlock(handlePtr);
		CloseClipboard();
	}

	void setClipboardData(std::wstring& dataToAssign) {
		if (!OpenClipboard(nullptr))
			return;
		unsigned int bytes = dataToAssign.length() * sizeof(wchar_t);
		HANDLE hClipboardData = GlobalAlloc(NULL, bytes);
		if (!hClipboardData) {
			CloseClipboard();
			return;
		}
		EmptyClipboard();
		wchar_t* dataPtr = (wchar_t*)(GlobalLock(hClipboardData));
		if (dataPtr && hClipboardData) {
			bytes /= sizeof(wchar_t);
			wcsncpy_s(dataPtr, bytes, dataToAssign.c_str(), dataToAssign.length());
			dataPtr[dataToAssign.length() - 1] = 0x00;

			GlobalUnlock(hClipboardData);

			SetClipboardData(CF_UNICODETEXT, hClipboardData);
		}
		CloseClipboard();
	}
	
	int stringCompare(const std::wstring& wString, const std::string& aString) {
		if(wString.length() != aString.length())
			return wString.length() - aString.length();

		for(unsigned int i=0;i<wString.length();i++) {
			if(wString.at(i) >= 0xFF)
				return wString.at(i) - aString.at(i);
			if(wString.at(i) != aString.at(i))
				return (wString.at(i)&0xFF) - aString.at(i);
		}
		return 0;
	}
	
	void convertStringAW(std::string& storage, const std::wstring& data, bool forceShrink) {
		storage = "";
		for(unsigned int i=0;i<data.length();i++) {
			//Just get the first 8 bits of the wide char
			if (forceShrink) {
				storage += (data[i] & 0xFF);
			}
			else {
				storage += data[i] & 0xFF;
				storage += (data[i] & 0xFF00)>>8;
			}
		}
	}
	void convertStringWA(std::wstring& storage, const std::string& data, bool forceWide) {
		storage = L"";
		for(unsigned int i=0;i<data.length();i++) {
			if (forceWide) {
				//wchar_t prepend

				//0xd1, 0x82 = 0x442
				//0xd1, 0x80 = 0x440


				//0xd0, 0x92 = 0x392
				//0xd0, 0xb8 = 0x418
				//0xd0, 0xb0 = 0x410
				//0xd0, 0xd0 = 0x430
				unsigned char firstByte = data[i];
				if (firstByte >= 0x80 && (i + 1) < data.length()) {
					storage += ((firstByte - 0xCC)<<8) + data[i+1];
					i += 1;
				}
				else {
					storage += data[i];
				}
			}
			else {
				storage += data[i];
			}
		}
	}

	void getPath(std::string *cArray, bool removeHandle) {
		(*cArray) = "";
		HMODULE exe = GetModuleHandle(nullptr);
		if (!exe)
			return;
		char tmp[MAX_PATH] = { 0x00 };
		GetModuleFileNameA(exe, tmp, MAX_PATH); 
	
		*cArray = std::string(tmp);
		if (removeHandle)
			*cArray = (*cArray).substr(0, (*cArray).find_last_of("\\"));
	}

	void getPath(std::wstring *wArray, bool removeHandle) {
		(*wArray) = L"";
		HMODULE exe = GetModuleHandle(nullptr);
		if (!exe)
			return;
		wchar_t tmp[MAX_PATH] = { 0x00 };
		GetModuleFileName(exe, tmp, MAX_PATH);

		*wArray = std::wstring(tmp);
		if (removeHandle)
			*wArray = (*wArray).substr(0, (*wArray).find_last_of(L"\\"));
	}

	void getPath(DWORD handle, std::wstring *wArray, bool removeHandle) {
		if (!handle) {
			(*wArray) = L"";
			return;
		}
		wchar_t path[MAX_PATH] = { 0x00 };
		HANDLE pHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, handle);
		long cpyLen = GetProcessImageFileName(pHandle, path, MAX_PATH);
		if (cpyLen < 0) {
			(*wArray) = L"GET_IMAGE_NAME_FAILED";
			return;
		}
		CloseHandle(pHandle);

		(*wArray) = std::wstring(path);
		convertImageNameToRealName(wArray);
		if (removeHandle) {
			(*wArray) = (*wArray).substr(0, (*wArray).find_last_of(L"\\"));
		}
	}

	void getPath(HMODULE handle, std::wstring *wArray, bool removeHandle) {
		if (!handle) {
			(*wArray) = L"";
			return;
		}
		wchar_t path[MAX_PATH] = { 0x00 };
		GetModuleFileName((HMODULE)handle, path, MAX_PATH);

		(*wArray) = std::wstring(path);
		if (removeHandle) {
			(*wArray) = (*wArray).substr(0, (*wArray).find_last_of(L"\\"));
		}
	}


	void getPath(HANDLE handle, std::wstring *wArray, bool removeHandle) {
		if (!handle) {
			(*wArray) = L"";
			return;
		}
		wchar_t path[MAX_PATH] = { 0x00 };
		long cpyLen = GetProcessImageFileName(handle, path, MAX_PATH);
		if (cpyLen < 0) {
			(*wArray) = L"GET_IMAGE_NAME_FAILED";
			return;
		}
	
		(*wArray) = std::wstring(path);
		convertImageNameToRealName(wArray);
		if (removeHandle) {
			(*wArray) = (*wArray).substr(0, (*wArray).find_last_of(L"\\"));
		}
	}

	void getPath(HWND handle, std::wstring *wArray, bool removeHandle) {
		if (!handle) {
			(*wArray) = L"";
			return;
		}
		wchar_t path[MAX_PATH] = { 0x00 };
		DWORD threadId; GetWindowThreadProcessId(handle,&threadId);
		HANDLE pHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, threadId);
		long cpyLen = GetProcessImageFileName(pHandle, path, MAX_PATH);
		if (cpyLen < 0) {
			(*wArray) = L"GET_IMAGE_NAME_FAILED";
			return;
		}
		CloseHandle(pHandle);
		(*wArray) = std::wstring(path);
		convertImageNameToRealName(wArray);
		if (removeHandle) {
			(*wArray) = (*wArray).substr(0, (*wArray).find_last_of(L"\\"));
		}
	}

	void getHandleName(std::wstring *wArray, bool removePath){
		(*wArray) = L"";
		HMODULE exe = GetModuleHandle(nullptr);
		if (!exe)
			return;
		wchar_t tmp[MAX_PATH] = { 0x00 };
		GetModuleFileName(exe, tmp, MAX_PATH);

		*wArray = std::wstring(tmp); 
		if (removePath) {
			(*wArray) = (*wArray).substr((*wArray).find_last_of(L"\\") + 1, (*wArray).length() - (*wArray).find_last_of(L"\\") - 1);
		}
	}

	void getHandleName(HMODULE handle, std::wstring *wArray, bool removePath) {
		if (!handle) {
			(*wArray) = L"";
			return;
		}
		wchar_t path[MAX_PATH] = { 0x00 };
		GetModuleFileName((HMODULE)handle, path, MAX_PATH);

		(*wArray) = std::wstring(path);
		if (removePath) {
			(*wArray) = (*wArray).substr((*wArray).find_last_of(L"\\") + 1, (*wArray).length() - (*wArray).find_last_of(L"\\") - 1);
		}
	}

	void getHandleName(HANDLE handle, std::wstring *wArray, bool removePath) {
		if (!handle) {
			(*wArray) = L"INVALID_HANDLE";
			return;
		}
		wchar_t path[MAX_PATH] = { 0x00 };
		long cpyLen = GetProcessImageFileName((HANDLE)handle, path, MAX_PATH);
		if (cpyLen < 0) {
			(*wArray) = L"GET_IMAGE_NAME_FAILED";
			return;
		}

		(*wArray) = std::wstring(path);
		convertImageNameToRealName(wArray);
		if (removePath) {
			(*wArray) = (*wArray).substr((*wArray).find_last_of(L"\\") + 1, (*wArray).length() - (*wArray).find_last_of(L"\\") - 1);
		}
	}

	void getHandleName(HWND handle, std::wstring *wArray, bool removePath) {
		if (!handle) {
			(*wArray) = L"";
			return;
		}
		wchar_t path[MAX_PATH] = { 0x00 };
		DWORD threadId; GetWindowThreadProcessId((HWND)handle, &threadId);
		HANDLE pHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, threadId); 
		if(!pHandle) {
			(*wArray) = L"";
			return;
		}
		long cpyLen = GetProcessImageFileName(pHandle, path, MAX_PATH);
		if (cpyLen < 0) {
			(*wArray) = L"GET_IMAGE_NAME_FAILED";
			return;
		}
		CloseHandle(pHandle);

		(*wArray) = std::wstring(path);
		convertImageNameToRealName(wArray);
		if (removePath) {
			(*wArray) = (*wArray).substr((*wArray).find_last_of(L"\\") + 1, (*wArray).length() - (*wArray).find_last_of(L"\\") - 1);
		}
	}

	void getHandleName(DWORD handle, std::wstring *wArray, bool removePath) {
		if (!handle || handle == -1) {
			(*wArray) = L"HANDLE_VALUE_EXCEPTION";
			return;
		}
		wchar_t path[MAX_PATH] = { 0x00 }; 
		HANDLE pHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, handle);
		if (!pHandle) {
			(*wArray) = L"INVALID_PROCESS_THREAD_ID_GIVEN";
			return;
		}
		long cpyLen = GetProcessImageFileName(pHandle, path, MAX_PATH);
		if (cpyLen < 0) {
			(*wArray) = L"GET_IMAGE_NAME_FAILED";
			return;
		}
		CloseHandle(pHandle);

		(*wArray) = std::wstring(path);
		convertImageNameToRealName(wArray);
		if (removePath) {
			(*wArray) = (*wArray).substr((*wArray).find_last_of(L"\\") + 1, (*wArray).length() - (*wArray).find_last_of(L"\\") - 1);
		}
	}

	time_t _fileTimeToLocalTime(LPSYSTEMTIME fileTime) {
		std::tm time;

		time.tm_sec = fileTime->wSecond;
		time.tm_min = fileTime->wMinute;
		time.tm_hour = fileTime->wHour;
		time.tm_mday = fileTime->wDay;
		time.tm_mon = fileTime->wMonth - 1;
		time.tm_year = fileTime->wYear - 1900;
		time.tm_isdst = -1;

		return std::mktime(&time);
	}

	void _fileTimeFromFileHandle(HANDLE file, SYSTEMTIME resultTime[]) {
		FILETIME timer[3] = { FILETIME(), FILETIME(), FILETIME() };
		if (!GetFileTime(file, &timer[0], &timer[1], &timer[2]))
			return;
		FileTimeToSystemTime(&timer[0], &resultTime[0]);
		FileTimeToSystemTime(&timer[1], &resultTime[1]);
		FileTimeToSystemTime(&timer[2], &resultTime[2]);
	}

	time_t getFileCreationTime(const char* filePath) {
		HANDLE file = CreateFileA(filePath, FILE_READ_ATTRIBUTES | FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
		SYSTEMTIME times[3] = { SYSTEMTIME(), SYSTEMTIME(), SYSTEMTIME() };
		_fileTimeFromFileHandle(file, times);
		CloseHandle(file);
		return _fileTimeToLocalTime(&times[0]);
	}
	time_t getFileCreationTime(const wchar_t* filePath) {
		HANDLE file = CreateFileW(filePath, FILE_READ_ATTRIBUTES | FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
		SYSTEMTIME times[3] = { SYSTEMTIME(), SYSTEMTIME(), SYSTEMTIME() };
		_fileTimeFromFileHandle(file, times);
		CloseHandle(file);
		return _fileTimeToLocalTime(&times[0]);
	}
	time_t getFileLastChangeTime(const char* filePath) {
		HANDLE file = CreateFileA(filePath, FILE_READ_ATTRIBUTES | FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
		SYSTEMTIME times[3] = { SYSTEMTIME(), SYSTEMTIME(), SYSTEMTIME() };
		_fileTimeFromFileHandle(file, times);
		CloseHandle(file);
		return _fileTimeToLocalTime(&times[2]);
	}
	time_t getFileLastChangeTime(const wchar_t *filePath) {
		HANDLE file = CreateFileW(filePath, FILE_READ_ATTRIBUTES | FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
		SYSTEMTIME times[3] = { SYSTEMTIME(), SYSTEMTIME(), SYSTEMTIME() };
		_fileTimeFromFileHandle(file, (SYSTEMTIME*)&times);
		CloseHandle(file);
		return _fileTimeToLocalTime(&times[2]);
	}
	
	bool loadFont(const wchar_t* path) {
		return (AddFontResourceEx(path, FR_PRIVATE, nullptr) == 1);
	}

	void convertImageNameToRealName(std::wstring *wArray) {
		if (!wArray || (*wArray).length() == 0) {
			return;
		}
		// \Device\HarddiskVolumeX\.....

		for (int i = 0; i < 2; i++) {
			(*wArray) = (*wArray).substr((*wArray).find(L"\\") + 1, (*wArray).length() - (*wArray).find(L"\\") - 1);
		}
		unsigned int len = wcslen(L"HardDiskVolume");
		wchar_t num[2] = { 0x00 };

		unsigned int w = _wtoi((*wArray).substr((*wArray).find(L"HardDiskVolume") + len + 1, (*wArray).find(L"\\")).c_str()) + 'A';

		num[0] = static_cast<wchar_t>(w);

		(*wArray) = (*wArray).substr((*wArray).find(L"\\") + 1, (*wArray).length() - (*wArray).find(L"\\") - 1);
		(*wArray) = std::wstring(num).append(L":\\").append((*wArray));
	}


	bool isTarget64Bit(const wchar_t *windowName) {
		HWND targetWindow;
		isWindowExistent(windowName, &targetWindow);
		if (!targetWindow) {
			return false;
		}
		return is64BitModule(getThreadIdByHWND(targetWindow));
	}

	bool isTarget64Bit(const wchar_t *windowName, const wchar_t *exeName) {
		HWND targetWindow;
		isWindowExistent(windowName, exeName, &targetWindow);
		if (!targetWindow) {
			return false;
		}
		return is64BitModule(getThreadIdByHWND(targetWindow));
	}

	DWORD getThreadIdByName(const wchar_t *executableName) {
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(snapshot, &entry) == TRUE) {
			do {
				if (_wcsicmp(executableName, entry.szExeFile) == 0)
					return entry.th32ProcessID;
			} while (Process32Next(snapshot, &entry) == TRUE);
		}
		return std::numeric_limits<DWORD>::max();
	}

	struct windowByProcess {
		HWND window;
		DWORD id;
	};

	BOOL CALLBACK EnumWindowByProcessId(HWND hWnd, LPARAM lParam) {
		windowByProcess* wbp = (windowByProcess*)lParam;
		
		DWORD tmpId = -1;
		GetWindowThreadProcessId(hWnd, &tmpId);

		if (tmpId == wbp->id) {
			wbp->window = hWnd;
			return FALSE;
		}
		return TRUE;
	}

	HWND getWindowByProcessID(DWORD processId) {
		windowByProcess wbp = { nullptr, processId };
		EnumWindows(EnumWindowByProcessId, (LPARAM)&wbp);
		
		return wbp.window;
	}

	void loadAllDllsOfProcess(HANDLE process) {
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(process));
		MODULEENTRY32 entry;
		entry.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(snapshot, &entry) == TRUE) {
			do {
				SymLoadModuleExW(process, 0, entry.szExePath, entry.szModule, (DWORD64)entry.modBaseAddr, entry.modBaseSize, nullptr, 0);
			} while (Module32Next(snapshot, &entry) == TRUE);
		}
	}

	void getCallStack(std::string& stackToWriteOn) {
		HMODULE dbgHelp = LoadLibraryA("%windir%\\system32\\dbghelp.dll");
		HANDLE currentProcess = GetCurrentProcess();
		std::string symInit = "SRV*%windir%\\system32\\symsrv.dll*http://msdl.microsoft.com/download/symbols";
		if (!SymInitialize(currentProcess, nullptr, false)) {
			return;
		}
		QuickInfo::loadAllDllsOfProcess(currentProcess);
		DWORD symFlags = SymGetOptions();
		SymSetOptions(symFlags | SYMOPT_LOAD_LINES | SYMOPT_FAIL_CRITICAL_ERRORS);
		CONTEXT context;
		context.ContextFlags = CONTEXT_FULL;

		HANDLE currentThread = GetCurrentThread();
		if (!currentThread) {
			return;
		}
		do {
			memset(&context, 0, sizeof(CONTEXT));
			__asm    call x
			__asm x: pop eax
			__asm    mov context.Eip, eax
			__asm    mov context.Ebp, ebp
			__asm    mov context.Esp, esp
		} while (0);

		STACKFRAME64 stackFrame;
#ifdef _M_IX86
		DWORD imageType = IMAGE_FILE_MACHINE_I386;
		stackFrame.AddrPC.Offset = context.Eip;
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = context.Ebp;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = context.Esp;
		stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
		DWORD imageType = IMAGE_FILE_MACHINE_AMD64;
		stackFrame.AddrPC.Offset = context.Rip;
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = context.Ebp;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = context.Esp;
		stackFrame.AddrStack.Mode = AddrModeFlat;
#else
#error "Not supported"
#endif
		//For some reason, trying to allocate space to the char-array 
		IMAGEHLP_SYMBOL64 *symbol = (IMAGEHLP_SYMBOL64*)malloc(sizeof(IMAGEHLP_SYMBOL64)+1024);
		memset(symbol, 0, sizeof(IMAGEHLP_SYMBOL64)+1024);
		symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		symbol->MaxNameLength = 1024;

		IMAGEHLP_LINE64 line;
		memset(&line, 0, sizeof(IMAGEHLP_LINE));
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

		IMAGEHLP_MODULE module;
		memset(&module, 0, sizeof(IMAGEHLP_MODULE));
		module.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

		//auto readProcessMemory = [](HANDLE hProcess, DWORD64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead) -> BOOL {
		//	return ReadProcessMemory(hProcess, &qwBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
		//};
		DWORD64 offsetFromSymbol64 = 0x00;
		DWORD offsetFromLine = 0x00;
		char buf[1024] = { 0x00 };
		for (unsigned int frameNum = 0;; ++frameNum) {
			if (!StackWalk64(imageType, currentProcess, currentThread, &stackFrame, &context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
				break;

			//Loop?
			if (stackFrame.AddrPC.Offset == stackFrame.AddrReturn.Offset)
				break;

			if (stackFrame.AddrPC.Offset != 0) {
				if (SymGetSymFromAddr64(currentProcess, stackFrame.AddrPC.Offset, &offsetFromSymbol64, symbol)) {
					UnDecorateSymbolName(symbol->Name, buf, 1024, UNDNAME_COMPLETE);
					stackToWriteOn += buf;
				}
				if (SymGetLineFromAddr64(currentProcess, stackFrame.AddrPC.Offset, &offsetFromLine, &line)) {
					sprintf_s(buf, " (File: %s, Line: %i)\n", line.FileName, line.LineNumber);
					stackToWriteOn += buf;
				}
				else {
					stackToWriteOn += "\n";
				}
			}
		}
		free(symbol);
		FreeLibrary(dbgHelp);
	}
	

	DWORD getParentThreadId()  {
		wchar_t wDllName[MAX_PATH] = { 0x00 };
		GetModuleFileName(GetModuleHandle(nullptr), wDllName, MAX_PATH);
		std::wstring dllName = std::wstring(wDllName);
		dllName = dllName.substr(dllName.find_last_of(L"\\") + 1, dllName.length() - dllName.find_last_of(L"\\") - 1);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(snapshot, &entry) == TRUE) {
			do {
				HMODULE hMods[1024];
				DWORD cbNeeded;
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, entry.th32ProcessID);
				if (hProcess) {
					if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
					{
						for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
						{
							wchar_t szModName[MAX_PATH];
							if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
								sizeof(szModName) / sizeof(wchar_t)))
							{
								std::wstring cmpStr = std::wstring(szModName);
								if (cmpStr.find(dllName.c_str()) >= 0) {
									CloseHandle(hProcess);
									return entry.th32ProcessID;
								}
							}
						}
					}
				}
			} while (Process32Next(snapshot, &entry) == TRUE);
		}
		return std::numeric_limits<DWORD>::max();
	}

	DWORD getParentThreadId(const wchar_t *exeName) {
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(snapshot, &entry) == TRUE) {
			do {
				if (_wcsicmp(entry.szExeFile, exeName) == 0) {
					return entry.th32ProcessID;
				}
			} while (Process32Next(snapshot, &entry) == TRUE);
		}
		return std::numeric_limits<DWORD>::max();
	}

	
#undef max
	template<class _Ty> const _Ty& max(const _Ty& a, const _Ty& b) {
		return (a<b)?b:a;
	}
#undef min
	template<class _Ty> const _Ty& min(const _Ty& a, const _Ty& b) {
		return (a<b)?a:b;
	}

	float fRand(const float max, bool allowNegative) {
		float res = static_cast<float>(rand() / static_cast<float>(RAND_MAX)) * max;
		if(allowNegative) {
			res = (rand() % 100 < 50 ? res : res * -1);
		}
		return res;
	}

	void wstringToUpper(std::wstring& str) {
		for (unsigned int i = 0; i < str.length(); i++) {
			str[i] = towupper(str[i]);
		}
	}

	void wstringToLower(std::wstring& str) {
		for (unsigned int i = 0; i < str.length(); i++) {
			str[i] = towlower(str[i]);
		}
	}

	BOOL CALLBACK QuickEnumWindowsProc(HWND hwnd, LONG lParam) {
	#ifdef _MY_DEBUG
		std::wstring modulePath = L"";
		getPath(&modulePath);
		CMyFile logger; logger.openFile(modulePath.append(L"\\enumwindows.log").c_str(), L"a+");
	#endif
		wchar_t wText[MAX_PATH] = { 0x00 };
		GetWindowText(hwnd, wText, MAX_PATH - 1);
		std::wstring windowText = std::wstring(wText);
		wstringToUpper(windowText);
		if (GetLastError() != 0) {
			/*
			char msg[0x100] = { 0x00 };
			sprintf(msg, "Error in EnumWindowsProc(...) at GetWindowText(...): 0x%x\n", GetLastError());
			MessageBoxA(nullptr, msg, "", 0);
			*/
			SetLastError(0);
		}
		if (windowText.length() < 2)
			return TRUE;
	
		if (windowExeName != nullptr) {
			std::wstring wantedExeName = std::wstring(windowExeName);
			wstringToUpper(wantedExeName);

			std::wstring currentExeName = L"";
			getHandleName(hwnd, &currentExeName);
			wstringToUpper(currentExeName);

			if (currentExeName.find(wantedExeName.c_str()) == -1) {
	#ifdef _MY_DEBUG
				logger.putStringWithVar("DIFFERENT EXE-NAMES: %w <--> %w\n", exeName.c_str(), windowExeName);
	#endif
				return TRUE;
			}
			else {
	#ifdef _MY_DEBUG
				logger.putStringWithVar("Found Exe Name: %w <--> %w\n", exeName.c_str(), windowExeName);
	#endif
			}
		}
		//If we're looking for window names, rather than IDs
		if (lParam < WINDOW_THREAD_SEARCH) {
			bool found = false;
			if (lParam & WINDOW_SHARP_SEARCH) {
				if (_wcsicmp(windowText.c_str(), windowName) == 0) {
					found = true;
				}
			}
			if (lParam & WINDOW_FUZZY_SEARCH) {
				size_t wantedWindowTextLength = wcslen(windowName);

				for (unsigned int i = 0; i<windowText.length() && !found; i++) {
					if (windowText[i] == towupper(windowName[0])) {
						if (i + wantedWindowTextLength > windowText.length())
							break;

						for (unsigned int j = 0; j < wantedWindowTextLength; j++) {
							if (windowText[i + j] != towupper(windowName[j])) {
								found = false;
								break;
							}
							found = true;
						}
					}
				}
			}
			if (found) {
				hWindow = hwnd;
				return false;
			}
		}
		else if (lParam < WINDOW_HANDLE_SEARCH){ //WINDOW_THREAD_SEARCH
			DWORD currentThreadId; DWORD otherThreadId;
			otherThreadId = GetWindowThreadProcessId(hwnd, &currentThreadId);
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, true, currentThreadId);
		
			std::wstring handle = L"";
			getHandleName(hProcess, &handle);

			DWORD realId = GetProcessId(hProcess);
	#ifdef _MY_DEBUG
			logger.putStringWithVar("Given LParam: 0x%x, WindowThreadId: 0x%x (CreatorThreadId: 0x%x) of %w\n", lParam, currentThreadId, otherThreadId, handle.c_str());
	#endif
			if (realId == static_cast<DWORD>(lParam % WINDOW_THREAD_SEARCH) || otherThreadId == static_cast<DWORD>(lParam % WINDOW_THREAD_SEARCH)) {
				hWindow = hwnd;
				return false;
			}
		}
		else {
			DWORD currentThreadId;
			GetWindowThreadProcessId(hwnd, &currentThreadId);
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, true, currentThreadId);
			if (hProcess == (HANDLE)lParam)	{
				hWindow = hwnd;
				return false;
			}
		}
		return true;
	}

	DWORD getAllRunningProcesses(std::wstring *result) {
		DWORD arraySize = 512; DWORD cbNecessary;
		DWORD *processArray = nullptr;
		do {
			if (processArray) {
				delete[] processArray;
				processArray = nullptr;
			}
			processArray = new DWORD[arraySize];
			EnumProcesses(processArray, sizeof(DWORD)* arraySize, &cbNecessary);
		} while ((cbNecessary/sizeof(DWORD)) >= arraySize);

		wchar_t tmpBuf[0x100] = { 0x00 };
		for (unsigned int i = 0; i < (cbNecessary / sizeof(DWORD)); i++) {
			getHandleName(processArray[i], result);
			wsprintf(tmpBuf, L" (%i)\n", processArray[i]);
			(*result) = (*result).append(std::wstring(tmpBuf));
		}
		delete[] processArray;
		processArray = nullptr;
		return (cbNecessary / sizeof(DWORD));
	}


	DWORD getAllRunningProcesses(QUICKPROCESSINFOS *result, DWORD maxEntries) {
		DWORD cbNecessary;
		DWORD *processArray = new DWORD[maxEntries];
		EnumProcesses(processArray, sizeof(DWORD)* maxEntries, &cbNecessary);

		for (unsigned int i = 0; i < (cbNecessary / sizeof(DWORD)); i++) {
			result[i].threadId = processArray[i];
			getHandleName(result[i].threadId, &result[i].path, false);
			result[i].handleName = result[i].path.substr(result[i].path.find_last_of(L"\\") + 1, result[i].path.length() - result[i].path.find_last_of(L"\\") - 1);
			result[i].path = result[i].path.substr(0, result[i].path.find_last_of(L"\\"));
		}
		delete[] processArray;
		processArray = nullptr;
		return (cbNecessary / sizeof(DWORD));
	}

	void getAllDllsOfProcess(DWORD threadId, std::wstring *result, bool removePath) {
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, threadId);
		if (!hProcess) {
			*result = L"OPEN_PROCESS_FAILED";
			return;
		}
		getAllDllsOfProcess(hProcess, result, removePath);
		CloseHandle(hProcess);
	}

	void getAllDllsOfProcess(HANDLE hProcess, std::wstring *result, bool removePath) {
		HMODULE hMods[1024] = { 0x00 }; DWORD cbNeeded = 0x00;
	#ifndef WINXP_DEF
		procEnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL);
	#else
		EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded);
	#endif
		for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			wchar_t szModName[MAX_PATH];
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
				sizeof(szModName) / sizeof(wchar_t)))
			{
				std::wstring cmpStr = std::wstring(szModName);
				if (removePath)
					cmpStr = cmpStr.substr(cmpStr.find_last_of(L"\\") + 1, cmpStr.length() - cmpStr.find_last_of(L"\\") - 1);
				(*result) = (*result).append(cmpStr).append(L"\n");
			}
		}
	}

	void getFilesFromDirectoryW(std::wstring& filePath, std::vector<std::wstring>& result, bool parseSubDirectories) {
		std::wstring emptyString = std::wstring(L"");
		return getFilesFromDirectoryW(filePath, emptyString, result, parseSubDirectories);
	}

	void getFilesFromDirectoryW(std::wstring& filePath, std::wstring& extensionFilter, std::vector<std::wstring>& result, bool parseSubDirectories) {
		WIN32_FIND_DATAW dataAttrib;

		std::wstring pathToSearchFor = filePath;
		wchar_t lastChar = pathToSearchFor.at(pathToSearchFor.length() - 1);

		bool filepathContainsEndSlashes = false;
		if (lastChar == '\\') {
			pathToSearchFor += L"*";
			filepathContainsEndSlashes = true;
		}
		else {
			pathToSearchFor += L"\\*";
		}
		HANDLE hNextFile = FindFirstFileW(pathToSearchFor.c_str(), &dataAttrib);
		if (hNextFile == INVALID_HANDLE_VALUE)
			return;
		if (extensionFilter.find(L".") != -1) {
			extensionFilter = extensionFilter.substr(extensionFilter.find(L".") + 1);
		}
		do {
			std::wstring fileName = std::wstring(dataAttrib.cFileName);
			if (fileName.at(0) == '.') {
				if (fileName.length() == 1 || (fileName.at(1) == '.' && fileName.length() == 2))
					continue;
			}
			if (dataAttrib.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (parseSubDirectories) {
					fileName = (filePath + fileName + std::wstring(L"\\"));
					getFilesFromDirectoryW(fileName, extensionFilter, result, parseSubDirectories);
				}
			}
			else {
				fileName = fileName.substr(fileName.find_last_of(L".") + 1);
				if (extensionFilter.length() > 0) {
					if (_wcsicmp(fileName.c_str(), extensionFilter.c_str()) == 0)
						result.push_back((filePath + (filepathContainsEndSlashes ? std::wstring(L"") : std::wstring(L"\\")) + std::wstring(dataAttrib.cFileName)));
				}
				else {
					result.push_back((filePath + (filepathContainsEndSlashes ? std::wstring(L"") : std::wstring(L"\\")) + std::wstring(dataAttrib.cFileName)));
				}
			}
		} while (FindNextFileW(hNextFile, &dataAttrib));
		FindClose(hNextFile);
		return;
	}

	void getFilesFromDirectoryA(std::string& filePath, std::vector<std::string>& result, bool parseSubDirectories) {
		std::string emptyString = std::string("");
		return getFilesFromDirectoryA(filePath, emptyString, result, parseSubDirectories);
	}
	void getFilesFromDirectoryA(std::string& filePath, std::string& extensionFilter, std::vector<std::string>& result, bool parseSubDirectories) {
		WIN32_FIND_DATAA dataAttrib;

		std::string pathToSearchFor = filePath; 
		char lastChar = pathToSearchFor.at(pathToSearchFor.length() - 1);

		bool filepathContainsEndSlashes = false;
		if (lastChar == '\\') {
			pathToSearchFor += "*";
			filepathContainsEndSlashes = true;
		}
		else {
			pathToSearchFor += "\\*";
		}
		HANDLE hNextFile = FindFirstFileA(pathToSearchFor.c_str(), &dataAttrib);
		if (hNextFile == INVALID_HANDLE_VALUE)
			return;
		if (extensionFilter.find(".") != -1) {
			extensionFilter = extensionFilter.substr(extensionFilter.find(".") + 1);
		}
		do {
			std::string fileName = std::string(dataAttrib.cFileName);
			if (fileName.at(0) == '.') {
				if (fileName.length() == 1 || (fileName.at(1) == '.' && fileName.length()==2))
					continue;
			}
			if (dataAttrib.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if(parseSubDirectories) {
					fileName = (filePath + fileName + std::string("\\"));
					getFilesFromDirectoryA( fileName, extensionFilter, result, parseSubDirectories);
				}
			}
			else {
				fileName = fileName.substr(fileName.find_last_of(".") + 1);
				if (extensionFilter.length() > 0) {
					if (_stricmp(fileName.c_str(), extensionFilter.c_str()) == 0)
						result.push_back((filePath + (filepathContainsEndSlashes ? std::string("") : std::string("\\")) + std::string(dataAttrib.cFileName)));
				}
				else {
					result.push_back((filePath + (filepathContainsEndSlashes ? std::string("") : std::string("\\")) + std::string(dataAttrib.cFileName)));
				}
			}
		} while (FindNextFileA(hNextFile, &dataAttrib));
		FindClose(hNextFile);
		return;
	}

	void isWindowExistent(const wchar_t *windowName, const wchar_t *exeName, HWND *exHwnd) {
		*exHwnd = nullptr; hWindow = nullptr;
		windowExeName = exeName;
		isWindowExistent(windowName, exHwnd);
		windowExeName = nullptr; 
		if (hWindow!=nullptr)
			*exHwnd = hWindow;
	}

	BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lParam) {
		std::vector<HWND>* v = (std::vector<HWND>*)lParam;
		v->push_back(hwnd);
		return TRUE;
	}

	std::vector<HWND> getAllWindowsOfProcess(DWORD threadId) {
		std::vector<HWND> result;
		EnumThreadWindows(threadId, EnumThreadWndProc, (LPARAM)&result);
		return result;
	}

	void isWindowExistent(const wchar_t *wName, HWND *exHwnd) {
		*exHwnd = nullptr; hWindow = nullptr;
		windowName = wName;
		EnumWindows((WNDENUMPROC)QuickEnumWindowsProc, (LPARAM)WINDOW_SHARP_SEARCH);
		if (hWindow == nullptr) {
			EnumWindows((WNDENUMPROC)QuickEnumWindowsProc, (LPARAM)WINDOW_FUZZY_SEARCH);
		}
		if (hWindow != nullptr)
			*exHwnd = hWindow;
	}

	void isWindowExistent(DWORD threadId, const wchar_t *exeName, HWND *result) {
		*result = nullptr; hWindow = nullptr;
		windowExeName = exeName;
		EnumWindows((WNDENUMPROC)QuickEnumWindowsProc, (LPARAM)(WINDOW_THREAD_SEARCH | threadId));
		windowExeName = nullptr;
		if (hWindow != nullptr)
			*result = hWindow;
	}

	void isWindowExistent(DWORD threadId, HWND *result) {
		*result = nullptr; hWindow = nullptr;
		EnumWindows((WNDENUMPROC)QuickEnumWindowsProc, (LPARAM)(WINDOW_THREAD_SEARCH | threadId));
		if (hWindow != nullptr)
			*result = hWindow;
	}

	DWORD getThreadIdByHWND(HWND window) {
		DWORD threadId = 0;
		GetWindowThreadProcessId(window, &threadId);
		return threadId;
	}

	void getDllExports(HMODULE dllModule, std::wstring* result) {
		if (!dllModule)
			return;
	#ifdef _MY_DEBUG
		CMyFile errorLog; errorLog.openFile("export.log", "a+");
	#endif
		PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)dllModule;
	#ifdef _MY_DEBUG
		errorLog.putStringWithVar("HeaderType: %i", dosHeader->e_magic);
	#endif
		if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
	#ifdef _MY_DEBUG
			errorLog.putString("DLL-Header has a valid signature!\n", true);
	#endif
			PIMAGE_NT_HEADERS header = (PIMAGE_NT_HEADERS)((BYTE *)dllModule + ((PIMAGE_DOS_HEADER)dllModule)->e_lfanew);

			if (header->Signature != IMAGE_NT_SIGNATURE || header->OptionalHeader.NumberOfRvaAndSizes == 0)
				return;
	#ifdef _MY_DEBUG
			errorLog.putString("DLL-Header has the NT-Signature!\n", true);
	#endif
			PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)dllModule + header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
			PVOID names = (BYTE *)dllModule + exports->AddressOfNames;
			PVOID addr = (BYTE *)dllModule + exports->AddressOfFunctions;
	#ifdef _MY_DEBUG
			errorLog.putString("Parsing through DLL-exports...\n", true);
	#endif
			wchar_t maxName[MAX_PATH] = { 0x00 };
			wchar_t printBuf[MAX_PATH] = { 0x00 };
			for (unsigned int i = 0; i < exports->NumberOfNames; i++) {
				char *currentName = (char*)((BYTE *)dllModule + ((DWORD *)names)[i]);
	#ifdef _MY_DEBUG
				errorLog.putStringWithVar("Found function %s at 0x%x...\n", currentName, (unsigned long)((BYTE *)dllModule + ((DWORD *)addr)[i]));
	#endif
				for (unsigned int j = 0; j < strlen(currentName); j++) {
					maxName[j] = currentName[j];
				}
				wsprintf(printBuf, L"%s - 0x%x", maxName, (unsigned long)((BYTE *)dllModule + ((DWORD *)addr)[i]));
				(*result) = (*result).append(printBuf).append(L"\n");
				memset(maxName, 0x00, sizeof(wchar_t)*MAX_PATH);
			}
		}
	}

	bool isDllLoaded(HMODULE hMod) {
		HANDLE hProcess = GetCurrentProcess();
		HMODULE hModules[1024]; DWORD cbNeeded;
	#ifndef WINXP_DEF
		procEnumProcessModulesEx(hProcess, hModules, sizeof(HMODULE)* 1024, &cbNeeded, LIST_MODULES_ALL);
	#else
		EnumProcessModules(hProcess, hModules, sizeof(HMODULE)* 1024, &cbNeeded);
	#endif
		for (unsigned int x = 0; x < (cbNeeded / sizeof(HMODULE)); x++)
		{
			wchar_t szModName[MAX_PATH];
							// Get the full path to the module's file.
			if (GetModuleFileNameExW(hProcess, hModules[x], szModName,
				sizeof(szModName) / sizeof(wchar_t)))
			{
				if (hModules[x] == hMod)
					return true;
			}	
		}
		return false;
	}

	bool isDllLoaded(std::wstring pathToDll) {
		HANDLE hProcess = GetCurrentProcess();
		HMODULE hModules[1024]; DWORD cbNeeded;
	#ifndef WINXP_DEF
		if (procEnumProcessModulesEx(hProcess, hModules, sizeof(HMODULE)* 1024, &cbNeeded, LIST_MODULES_ALL)) {
	#else
		if (EnumProcessModules(hProcess, hModules, sizeof(HMODULE) * 1024, &cbNeeded)) {
	#endif
			for (unsigned int x = 0; x < (cbNeeded / sizeof(HMODULE)); x++)
			{
				wchar_t szModName[MAX_PATH];

				// Get the full path to the module's file.
				if (GetModuleFileNameExW(hProcess, hModules[x], szModName,
					sizeof(szModName) / sizeof(wchar_t)))
				{
					if (pathToDll.find(szModName)!=-1)
						return true;
				}
			}
		}
		return false;
	}

	bool isDllLoaded(const wchar_t* pathToDll) {
		HANDLE hProcess = GetCurrentProcess();
		HMODULE hModules[1024]; DWORD cbNeeded;
	#ifndef WINXP_DEF
		if (procEnumProcessModulesEx(hProcess, hModules, sizeof(HMODULE)* 1024, &cbNeeded, LIST_MODULES_ALL)) {
	#else
		if (EnumProcessModules(hProcess, hModules, sizeof(HMODULE) * 1024, &cbNeeded)) {
	#endif
			for (unsigned int x = 0; x < (cbNeeded / sizeof(HMODULE)); x++)
			{
				wchar_t szModName[MAX_PATH];

				// Get the full path to the module's file.
				if (GetModuleFileNameExW(hProcess, hModules[x], szModName,
					sizeof(szModName) / sizeof(wchar_t)))
				{
					if (_wcsicmp(szModName, pathToDll) == 0)
						return true;
				}
			}
		}
		return false;
	}

	void getDllExports(std::wstring pathToDll, std::wstring* result) {
		HMODULE dllModule = LoadLibraryEx(pathToDll.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
		getDllExports(dllModule, result);
		FreeLibrary(dllModule);
	}

	#include <d3d9.h>

	unsigned long* createVTable(HMODULE d9dll, DWORD *vTable, std::wstring *log, DWORD vTableSize, DWORD timeoutInMs, bool logOffsets) {

		HWND window;

		clock_t startTime = clock();
		LPDIRECT3D9 pD3D = nullptr;
		IDirect3D9* (__stdcall* procD3DCreate)(UINT);
		procD3DCreate = (IDirect3D9* (__stdcall*)(UINT))GetProcAddress(d9dll, "Direct3DCreate9");

		//clear the table, just in case.
		for (unsigned int i = 0; i < vTableSize; i++)
			vTable[i] = 0x00;

		bool finishedTheJob = false;
		while (!finishedTheJob) {
			if (timeoutInMs != 0 && (DWORD)(clock() - startTime) >= timeoutInMs)
				break;
			Sleep(10);
			std::wstring handleName = L"";
			getHandleName(&handleName);
			DWORD exeId = getThreadIdByName(handleName.c_str());
			if (exeId == -1) {
				if (log != nullptr) {
					(*log) = L"";
					(*log) = (*log).append(L"INVALID_THREAD_ID_FOR_").append(handleName);
				}
				continue;
			}

			isWindowExistent(exeId, &window);
			if (!window) {
				isWindowExistent(L"D3D", &window);
				if (!window) {
					if (log != nullptr) {
						(*log) = L"";
						*log = (*log).append(L"WINDOW_NOT_EXISTENT_FOR_").append(handleName);
					}
					continue;
				}
			}

			MODULEINFO sModuleInfo;
			GetModuleInformation(GetCurrentProcess(), d9dll, &sModuleInfo, sizeof(MODULEINFO));

			if (!pD3D) {
				pD3D = procD3DCreate(D3D_SDK_VERSION);
				if (!pD3D) {
					if (log != nullptr) {
						(*log) = L"";
						*log = (*log).append(L"procD3DCreate_FAILED");
					}
					continue;
				}
			}
			D3DDISPLAYMODE d3ddm;
			pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
			D3DPRESENT_PARAMETERS d3dpp;
			ZeroMemory(&d3dpp, sizeof(d3dpp));
			d3dpp.Windowed = true;
			d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dpp.BackBufferFormat = d3ddm.Format;

			IDirect3DDevice9 *pDevice;
			HRESULT deviceRes = pD3D->CreateDevice(D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,	// the device we suppose any app would be using.
				window,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
				&d3dpp, &pDevice);

			if (!pDevice) {
				if (log != nullptr) {
					(*log) = L"";
					wchar_t num[0x20] = { 0x00 }; _itow_s(deviceRes, num, 0x10);
					*log = (*log).append(L"CREATE_DEVICE_FAILED_WITH_ERROR_").append(std::wstring(num));
				}
				continue;
			}
			unsigned long *pVTable = (unsigned long*)*(unsigned long*)pDevice;
			wchar_t tmpBuf[0x100] = { 0x00 };
			for (unsigned int i = 0; i <= vTableSize; i++) {
				long offset = pVTable[i] - reinterpret_cast<unsigned int>(d9dll);
				if (pVTable[i] == 0 || (unsigned)abs(offset) > sModuleInfo.SizeOfImage) {
					finishedTheJob = true;
					break;
				}
				if (i + 1 == vTableSize)
					finishedTheJob = true;
				vTable[i] = pVTable[i];
				if (logOffsets && log != nullptr) {
					wsprintf(tmpBuf, L"[%i] 0x%x (Offset: 0x%x)\n", i, pVTable[i], pVTable[i] - reinterpret_cast<unsigned int>(d9dll));
					(*log) = (*log).append(std::wstring(tmpBuf));
				}
			}
		}
		return vTable;
	}


	bool ejectDLL(DWORD threadId, std::wstring dllName) {
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, threadId);
		if (!hProcess)
			throw TraceableException("OpenProcess failed! ErrorCode: %i", GetLastError());
		bool res = ejectDLL(hProcess, dllName);
		CloseHandle(hProcess);
		return res;
	}

	bool ejectDLL(HANDLE hProcess, std::wstring dllName) throw() {
		if (!hProcess)
			return false;
		HMODULE hModules[1024]; DWORD dCopiedEntries = 0;
	#ifndef WINXP_DEF
		procEnumProcessModulesEx(hProcess, hModules, sizeof(HMODULE)* 1024, &dCopiedEntries, LIST_MODULES_ALL);
	#else
		EnumProcessModules(hProcess, hModules, sizeof(HMODULE) * 1024, &dCopiedEntries);
	#endif
		for (unsigned int i = 0; i < dCopiedEntries / sizeof(HMODULE); i++) {
			HMODULE hMod = hModules[i];
			wchar_t hModName[MAX_PATH] = { 0x00 };
			GetModuleFileNameEx(hProcess, hMod, hModName, MAX_PATH);

			std::wstring wsModName = std::wstring(hModName);
			if (wsModName.find(dllName) != -1) {
				LPVOID lpLoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FreeLibrary");
				HANDLE remoteThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpLoadLibraryAddr, hModules[i], NULL, NULL);

				DWORD dExitCode = 0;
				WaitForSingleObject(remoteThread, INFINITE);
				GetExitCodeThread(remoteThread, &dExitCode);
				CloseHandle(remoteThread);
				return true;
			}
		}
		return false;
	}

	static int CALLBACK __folderCallback(HWND hwnd, UINT msg, LPARAM, LPARAM lpData) {
		if (msg == BFFM_INITIALIZED) {
			LPCTSTR path = reinterpret_cast<LPCTSTR>(lpData);
			::SendMessage(hwnd, BFFM_SETSELECTION, true, lpData);
		}
		return 0;
	};
	LPCTSTR folderDialog(LPCTSTR filePath, tstring& result) {
		BROWSEINFO info = { 0x00 };
		info.ulFlags = BIF_NEWDIALOGSTYLE;
		info.lParam = reinterpret_cast<LPARAM>(filePath);
		info.lpfn = __folderCallback;
		LPITEMIDLIST pidl = SHBrowseForFolder(&info);

		if (pidl != 0)
		{
			TCHAR path[MAX_PATH];
			//get the name of the folder and put it in path
			SHGetPathFromIDList(pidl, path);

			//free memory used
			IMalloc * imalloc = 0;
			if (SUCCEEDED(SHGetMalloc(&imalloc)))
			{
				imalloc->Free(pidl);
				imalloc->Release();
			}
			result = tstring(path);
			return result.c_str();
		}
		return nullptr;
	}

	unsigned int* removeHandleFromPath(std::wstring res, unsigned int *pos) {
		pos[0] = 0x00;
		pos[1] = res.find_last_of(L"\\");
		return pos;
	}
	unsigned int* removePathFromHandle(std::wstring res, unsigned int *pos) {
		pos[0] = res.find_last_of(L"\\") + 1;
		pos[1] = res.length() - res.find_last_of(L"\\") - 1;
		return pos;
	}

	DWORD getDirectXOffset(unsigned long *pVTable, DWORD funcId) {
		if (!pVTable || funcId >= D3D9_ENUMEND)
			return MAXDWORD;
		return pVTable[funcId];
	}

	void intAsWString(const DWORD data, std::wstring& result) {
		result = L"";
		if (data == 0)
			result += L"0";

		DWORD d = data;
		while (d > 0) {
			result = static_cast<wchar_t>((d % 10) + 0x30) + result;
			d /= 10;
		}
	}
	void intAsWString(const unsigned int data, std::wstring& result) {
		return intAsWString(static_cast<const DWORD>(data), result);
	}
	DWORD numlen(DWORD number) {
		DWORD checkNum = 0x00; DWORD len = 0x01;
		for (; checkNum <= number; len++) {
			checkNum = (DWORD)pow(10.0f, (float)len);
		}
		return len;
	}

	DWORD fnumlen(double number) {
		double afterCommaNum = (double)(number - (DWORD)number) ;
		double checkNum = 0;
		DWORD len = 0x00;

		//Number: 125,00
		//(DWORD)1250 == 1250.00000) --> true, no after-comma numbers.
		if (afterCommaNum <= 0.00001)
			return 0;

		while (!(checkNum >= 1000.00f && ((DWORD)checkNum % 1000 == 999 || (DWORD)checkNum % 1000 == 0))) {
			//number: 0.0125 --> numlen = 4
			//number: 0.1 --> numlen = 1
			//0.1 --> 1 --> 10 --> 100.00
			checkNum = afterCommaNum * (DWORD)pow(10.0f, (float)len); //0.0125000 --> 0.125000 --> 1.25000 --> 12.5000 --> 125.000 --> 1250.000 --> 12500.000
			len++;
		}
		return len - 3;
	}

	const char* getErrorName(DWORD reason) {
		switch(reason) { 
			case 0x0: return "ERROR_SUCCESS";
			case 0x1: return "ERROR_INVALID_FUNCTION";
			case 0x2: return "ERROR_FILE_NOT_FOUND";
			case 0x3: return "ERROR_PATH_NOT_FOUND";
			case 0x4: return "ERROR_TOO_MANY_OPEN_FILES";
			case 0x5: return "ERROR_ACCESS_DENIED";
			case 0x6: return "ERROR_INVALID_HANDLE";
			case 0x7: return "ERROR_ARENA_TRASHED";
			case 0x8: return "ERROR_NOT_ENOUGH_MEMORY";
			case 0x9: return "ERROR_INVALID_BLOCK";
			case 0xA: return "ERROR_BAD_ENVIRONMENT";
			case 0xB: return "ERROR_BAD_FORMAT";
			case 0xC: return "ERROR_INVALID_ACCESS";
			case 0xD: return "ERROR_INVALID_DATA";
			case 0xE: return "ERROR_OUTOFMEMORY";
			case 0xF: return "ERROR_INVALID_DRIVE";
			case 0x10: return "ERROR_CURRENT_DIRECTORY";
			case 0x11: return "ERROR_NOT_SAME_DEVICE";
			case 0x12: return "ERROR_NO_MORE_FILES";
			case 0x13: return "ERROR_WRITE_PROTECT";
			case 0x14: return "ERROR_BAD_UNIT";
			case 0x15: return "ERROR_NOT_READY";
			case 0x16: return "ERROR_BAD_COMMAND";
			case 0x17: return "ERROR_CRC";
			case 0x18: return "ERROR_BAD_LENGTH";
			case 0x19: return "ERROR_SEEK";
			case 0x1A: return "ERROR_NOT_DOS_DISK";
			case 0x1B: return "ERROR_SECTOR_NOT_FOUND";
			case 0x1C: return "ERROR_OUT_OF_PAPER";
			case 0x1D: return "ERROR_WRITE_FAULT";
			case 0x1E: return "ERROR_READ_FAULT";
			case 0x1F: return "ERROR_GEN_FAILURE";
			case 0x20: return "ERROR_SHARING_VIOLATION";
			case 0x21: return "ERROR_LOCK_VIOLATION";
			case 0x22: return "ERROR_WRONG_DISK";
			case 0x24: return "ERROR_SHARING_BUFFER_EXCEEDED";
			case 0x26: return "ERROR_HANDLE_EOF";
			case 0x27: return "ERROR_HANDLE_DISK_FULL";
			case 0x32: return "ERROR_NOT_SUPPORTED";
			case 0x33: return "ERROR_REM_NOT_LIST";
			case 0x34: return "ERROR_DUP_NAME";
			case 0x35: return "ERROR_BAD_NETPATH";
			case 0x36: return "ERROR_NETWORK_BUSY";
			case 0x37: return "ERROR_DEV_NOT_EXIST";
			case 0x38: return "ERROR_TOO_MANY_CMDS";
			case 0x39: return "ERROR_ADAP_HDW_ERR";
			case 0x3A: return "ERROR_BAD_NET_RESP";
			case 0x3B: return "ERROR_UNEXP_NET_ERR";
			case 0x3C: return "ERROR_BAD_REM_ADAP";
			case 0x3D: return "ERROR_PRINTQ_FULL";
			case 0x3E: return "ERROR_NO_SPOOL_SPACE";
			case 0x3F: return "ERROR_PRINT_CANCELLED";
			case 0x40: return "ERROR_NETNAME_DELETED";
			case 0x41: return "ERROR_NETWORK_ACCESS_DENIED";
			case 0x42: return "ERROR_BAD_DEV_TYPE";
			case 0x43: return "ERROR_BAD_NET_NAME";
			case 0x44: return "ERROR_TOO_MANY_NAMES";
			case 0x45: return "ERROR_TOO_MANY_SESS";
			case 0x46: return "ERROR_SHARING_PAUSED";
			case 0x47: return "ERROR_REQ_NOT_ACCEP";
			case 0x48: return "ERROR_REDIR_PAUSED";
			case 0x50: return "ERROR_FILE_EXISTS";
			case 0x52: return "ERROR_CANNOT_MAKE";
			case 0x53: return "ERROR_FAIL_I24";
			case 0x54: return "ERROR_OUT_OF_STRUCTURES";
			case 0x55: return "ERROR_ALREADY_ASSIGNED";
			case 0x56: return "ERROR_INVALID_PASSWORD";
			case 0x57: return "ERROR_INVALID_PARAMETER";
			case 0x58: return "ERROR_NET_WRITE_FAULT";
			case 0x59: return "ERROR_NO_PROC_SLOTS";
			case 0x64: return "ERROR_TOO_MANY_SEMAPHORES";
			case 0x65: return "ERROR_EXCL_SEM_ALREADY_OWNED";
			case 0x66: return "ERROR_SEM_IS_SET";
			case 0x67: return "ERROR_TOO_MANY_SEM_REQUESTS";
			case 0x68: return "ERROR_INVALID_AT_INTERRUPT_TIME";
			case 0x69: return "ERROR_SEM_OWNER_DIED";
			case 0x6A: return "ERROR_SEM_USER_LIMIT";
			case 0x6B: return "ERROR_DISK_CHANGE";
			case 0x6C: return "ERROR_DRIVE_LOCKED";
			case 0x6D: return "ERROR_BROKEN_PIPE";
			case 0x6E: return "ERROR_OPEN_FAILED";
			case 0x6F: return "ERROR_BUFFER_OVERFLOW";
			case 0x70: return "ERROR_DISK_FULL";
			case 0x71: return "ERROR_NO_MORE_SEARCH_HANDLES";
			case 0x72: return "ERROR_INVALID_TARGET_HANDLE";
			case 0x75: return "ERROR_INVALID_CATEGORY";
			case 0x76: return "ERROR_INVALID_VERIFY_SWITCH";
			case 0x77: return "ERROR_BAD_DRIVER_LEVEL";
			case 0x78: return "ERROR_CALL_NOT_IMPLEMENTED";
			case 0x79: return "ERROR_SEM_TIMEOUT";
			case 0x7A: return "ERROR_INSUFFICIENT_BUFFER";
			case 0x7B: return "ERROR_INVALID_NAME";
			case 0x7C: return "ERROR_INVALID_LEVEL";
			case 0x7D: return "ERROR_NO_VOLUME_LABEL";
			case 0x7E: return "ERROR_MOD_NOT_FOUND";
			case 0x7F: return "ERROR_PROC_NOT_FOUND";
			case 0x80: return "ERROR_WAIT_NO_CHILDREN";
			case 0x81: return "ERROR_CHILD_NOT_COMPLETE";
			case 0x82: return "ERROR_DIRECT_ACCESS_HANDLE";
			case 0x83: return "ERROR_NEGATIVE_SEEK";
			case 0x84: return "ERROR_SEEK_ON_DEVICE";
			case 0x85: return "ERROR_IS_JOIN_TARGET";
			case 0x86: return "ERROR_IS_JOINED";
			case 0x87: return "ERROR_IS_SUBSTED";
			case 0x88: return "ERROR_NOT_JOINED";
			case 0x89: return "ERROR_NOT_SUBSTED";
			case 0x8A: return "ERROR_JOIN_TO_JOIN";
			case 0x8B: return "ERROR_SUBST_TO_SUBST";
			case 0x8C: return "ERROR_JOIN_TO_SUBST";
			case 0x8D: return "ERROR_SUBST_TO_JOIN";
			case 0x8E: return "ERROR_BUSY_DRIVE";
			case 0x8F: return "ERROR_SAME_DRIVE";
			case 0x90: return "ERROR_DIR_NOT_ROOT";
			case 0x91: return "ERROR_DIR_NOT_EMPTY";
			case 0x92: return "ERROR_IS_SUBST_PATH";
			case 0x93: return "ERROR_IS_JOIN_PATH";
			case 0x94: return "ERROR_PATH_BUSY";
			case 0x95: return "ERROR_IS_SUBST_TARGET";
			case 0x96: return "ERROR_SYSTEM_TRACE";
			case 0x97: return "ERROR_INVALID_EVENT_COUNT";
			case 0x98: return "ERROR_TOO_MANY_MUXWAITERS";
			case 0x99: return "ERROR_INVALID_LIST_FORMAT";
			case 0x9A: return "ERROR_LABEL_TOO_LONG";
			case 0x9B: return "ERROR_TOO_MANY_TCBS";
			case 0x9C: return "ERROR_SIGNAL_REFUSED";
			case 0x9D: return "ERROR_DISCARDED";
			case 0x9E: return "ERROR_NOT_LOCKED";
			case 0x9F: return "ERROR_BAD_THREADID_ADDR";
			case 0xA0: return "ERROR_BAD_ARGUMENTS";
			case 0xA1: return "ERROR_BAD_PATHNAME";
			case 0xA2: return "ERROR_SIGNAL_PENDING";
			case 0xA4: return "ERROR_MAX_THRDS_REACHED";
			case 0xA7: return "ERROR_LOCK_FAILED";
			case 0xAA: return "ERROR_BUSY";
			case 0xAB: return "ERROR_DEVICE_SUPPORT_IN_PROGRESS";
			case 0xAD: return "ERROR_CANCEL_VIOLATION";
			case 0xAE: return "ERROR_ATOMIC_LOCKS_NOT_SUPPORTED";
			case 0xB4: return "ERROR_INVALID_SEGMENT_NUMBER";
			case 0xB6: return "ERROR_INVALID_ORDINAL";
			case 0xB7: return "ERROR_ALREADY_EXISTS";
			case 0xBA: return "ERROR_INVALID_FLAG_NUMBER";
			case 0xBB: return "ERROR_SEM_NOT_FOUND";
			case 0xBC: return "ERROR_INVALID_STARTING_CODESEG";
			case 0xBD: return "ERROR_INVALID_STACKSEG";
			case 0xBE: return "ERROR_INVALID_MODULETYPE";
			case 0xBF: return "ERROR_INVALID_EXE_SIGNATURE";
			case 0xC0: return "ERROR_EXE_MARKED_INVALID";
			case 0xC1: return "ERROR_BAD_EXE_FORMAT";
			case 0xC2: return "ERROR_ITERATED_DATA_EXCEEDS_64k";
			case 0xC3: return "ERROR_INVALID_MINALLOCSIZE";
			case 0xC4: return "ERROR_DYNLINK_FROM_INVALID_RING";
			case 0xC5: return "ERROR_IOPL_NOT_ENABLED";
			case 0xC6: return "ERROR_INVALID_SEGDPL";
			case 0xC7: return "ERROR_AUTODATASEG_EXCEEDS_64k";
			case 0xC8: return "ERROR_RING2SEG_MUST_BE_MOVABLE";
			case 0xC9: return "ERROR_RELOC_CHAIN_XEEDS_SEGLIM";
			case 0xCA: return "ERROR_INFLOOP_IN_RELOC_CHAIN";
			case 0xCB: return "ERROR_ENVVAR_NOT_FOUND";
			case 0xCD: return "ERROR_NO_SIGNAL_SENT";
			case 0xCE: return "ERROR_FILENAME_EXCED_RANGE";
			case 0xCF: return "ERROR_RING2_STACK_IN_USE";
			case 0xD0: return "ERROR_META_EXPANSION_TOO_LONG";
			case 0xD1: return "ERROR_INVALID_SIGNAL_NUMBER";
			case 0xD2: return "ERROR_THREAD_1_INACTIVE";
			case 0xD4: return "ERROR_LOCKED";
			case 0xD6: return "ERROR_TOO_MANY_MODULES";
			case 0xD7: return "ERROR_NESTING_NOT_ALLOWED";
			case 0xD8: return "ERROR_EXE_MACHINE_TYPE_MISMATCH";
			case 0xD9: return "ERROR_EXE_CANNOT_MODIFY_SIGNED_BINARY";
			case 0xDA: return "ERROR_EXE_CANNOT_MODIFY_STRONG_SIGNED_BINARY";
			case 0xDC: return "ERROR_FILE_CHECKED_OUT";
			case 0xDD: return "ERROR_CHECKOUT_REQUIRED";
			case 0xDE: return "ERROR_BAD_FILE_TYPE";
			case 0xDF: return "ERROR_FILE_TOO_LARGE";
			case 0xE0: return "ERROR_FORMS_AUTH_REQUIRED";
			case 0xE1: return "ERROR_VIRUS_INFECTED";
			case 0xE2: return "ERROR_VIRUS_DELETED";
			case 0xE5: return "ERROR_PIPE_LOCAL";
			case 0xE6: return "ERROR_BAD_PIPE";
			case 0xE7: return "ERROR_PIPE_BUSY";
			case 0xE8: return "ERROR_NO_DATA";
			case 0xE9: return "ERROR_PIPE_NOT_CONNECTED";
			case 0xEA: return "ERROR_MORE_DATA";
			case 0xF0: return "ERROR_VC_DISCONNECTED";
			case 0xFE: return "ERROR_INVALID_EA_NAME";
			case 0xFF: return "ERROR_EA_LIST_INCONSISTENT";
			case 0x103: return "ERROR_NO_MORE_ITEMS";
			case 0x10A: return "ERROR_CANNOT_COPY";
			case 0x10B: return "ERROR_DIRECTORY";
			case 0x113: return "ERROR_EAS_DIDNT_FIT";
			case 0x114: return "ERROR_EA_FILE_CORRUPT";
			case 0x115: return "ERROR_EA_TABLE_FULL";
			case 0x116: return "ERROR_INVALID_EA_HANDLE";
			case 0x11A: return "ERROR_EAS_NOT_SUPPORTED";
			case 0x120: return "ERROR_NOT_OWNER";
			case 0x12A: return "ERROR_TOO_MANY_POSTS";
			case 0x12B: return "ERROR_PARTIAL_COPY";
			case 0x12C: return "ERROR_OPLOCK_NOT_GRANTED";
			case 0x12D: return "ERROR_INVALID_OPLOCK_PROTOCOL";
			case 0x12E: return "ERROR_DISK_TOO_FRAGMENTED";
			case 0x12F: return "ERROR_DELETE_PENDING";
			case 0x130: return "ERROR_INCOMPATIBLE_WITH_GLOBAL_SHORT_NAME_REGISTRY_SETTING";
			case 0x131: return "ERROR_SHORT_NAMES_NOT_ENABLED_ON_VOLUME";
			case 0x132: return "ERROR_SECURITY_STREAM_IS_INCONSISTENT";
			case 0x133: return "ERROR_INVALID_LOCK_RANGE";
			case 0x134: return "ERROR_IMAGE_SUBSYSTEM_NOT_PRESENT";
			case 0x135: return "ERROR_NOTIFICATION_GUID_ALREADY_DEFINED";
			case 0x136: return "ERROR_INVALID_EXCEPTION_HANDLER";
			case 0x137: return "ERROR_DUPLICATE_PRIVILEGES";
			case 0x138: return "ERROR_NO_RANGES_PROCESSED";
			case 0x139: return "ERROR_NOT_ALLOWED_ON_SYSTEM_FILE";
			case 0x13A: return "ERROR_DISK_RESOURCES_EXHAUSTED";
			case 0x13B: return "ERROR_INVALID_TOKEN";
			case 0x13C: return "ERROR_DEVICE_FEATURE_NOT_SUPPORTED";
			case 0x13D: return "ERROR_MR_MID_NOT_FOUND";
			case 0x13E: return "ERROR_SCOPE_NOT_FOUND";
			case 0x13F: return "ERROR_UNDEFINED_SCOPE";
			case 0x140: return "ERROR_INVALID_CAP";
			case 0x141: return "ERROR_DEVICE_UNREACHABLE";
			case 0x142: return "ERROR_DEVICE_NO_RESOURCES";
			case 0x143: return "ERROR_DATA_CHECKSUM_ERROR";
			case 0x144: return "ERROR_INTERMIXED_KERNEL_EA_OPERATION";
			case 0x146: return "ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED";
			case 0x147: return "ERROR_OFFSET_ALIGNMENT_VIOLATION";
			case 0x148: return "ERROR_INVALID_FIELD_IN_PARAMETER_LIST";
			case 0x149: return "ERROR_OPERATION_IN_PROGRESS";
			case 0x14A: return "ERROR_BAD_DEVICE_PATH";
			case 0x14B: return "ERROR_TOO_MANY_DESCRIPTORS";
			case 0x14C: return "ERROR_SCRUB_DATA_DISABLED";
			case 0x14D: return "ERROR_NOT_REDUNDANT_STORAGE";
			case 0x14E: return "ERROR_RESIDENT_FILE_NOT_SUPPORTED";
			case 0x14F: return "ERROR_COMPRESSED_FILE_NOT_SUPPORTED";
			case 0x150: return "ERROR_DIRECTORY_NOT_SUPPORTED";
			case 0x151: return "ERROR_NOT_READ_FROM_COPY";
			case 0x15E: return "ERROR_FAIL_NOACTION_REBOOT";
			case 0x15F: return "ERROR_FAIL_SHUTDOWN";
			case 0x160: return "ERROR_FAIL_RESTART";
			case 0x161: return "ERROR_MAX_SESSIONS_REACHED";
			case 0x190: return "ERROR_THREAD_MODE_ALREADY_BACKGROUND";
			case 0x191: return "ERROR_THREAD_MODE_NOT_BACKGROUND";
			case 0x192: return "ERROR_PROCESS_MODE_ALREADY_BACKGROUND";
			case 0x193: return "ERROR_PROCESS_MODE_NOT_BACKGROUND";
			case 0x1E7: return "ERROR_INVALID_ADDRESS";
			case 0x1F4: return "ERROR_USER_PROFILE_LOAD";
			case 0x216: return "ERROR_ARITHMETIC_OVERFLOW";
			case 0x217: return "ERROR_PIPE_CONNECTED";
			case 0x218: return "ERROR_PIPE_LISTENING";
			case 0x219: return "ERROR_VERIFIER_STOP";
			case 0x21A: return "ERROR_ABIOS_ERROR";
			case 0x21B: return "ERROR_WX86_WARNING";
			case 0x21C: return "ERROR_WX86_ERROR";
			case 0x21D: return "ERROR_TIMER_NOT_CANCELED";
			case 0x21E: return "ERROR_UNWIND";
			case 0x21F: return "ERROR_BAD_STACK";
			case 0x220: return "ERROR_INVALID_UNWIND_TARGET";
			case 0x221: return "ERROR_INVALID_PORT_ATTRIBUTES";
			case 0x222: return "ERROR_PORT_MESSAGE_TOO_LONG";
			case 0x223: return "ERROR_INVALID_QUOTA_LOWER";
			case 0x224: return "ERROR_DEVICE_ALREADY_ATTACHED";
			case 0x225: return "ERROR_INSTRUCTION_MISALIGNMENT";
			case 0x226: return "ERROR_PROFILING_NOT_STARTED";
			case 0x227: return "ERROR_PROFILING_NOT_STOPPED";
			case 0x228: return "ERROR_COULD_NOT_INTERPRET";
			case 0x229: return "ERROR_PROFILING_AT_LIMIT";
			case 0x22A: return "ERROR_CANT_WAIT";
			case 0x22B: return "ERROR_CANT_TERMINATE_SELF";
			case 0x22C: return "ERROR_UNEXPECTED_MM_CREATE_ERR";
			case 0x22D: return "ERROR_UNEXPECTED_MM_MAP_ERROR";
			case 0x22E: return "ERROR_UNEXPECTED_MM_EXTEND_ERR";
			case 0x22F: return "ERROR_BAD_FUNCTION_TABLE";
			case 0x230: return "ERROR_NO_GUID_TRANSLATION";
			case 0x231: return "ERROR_INVALID_LDT_SIZE";
			case 0x233: return "ERROR_INVALID_LDT_OFFSET";
			case 0x234: return "ERROR_INVALID_LDT_DESCRIPTOR";
			case 0x235: return "ERROR_TOO_MANY_THREADS";
			case 0x236: return "ERROR_THREAD_NOT_IN_PROCESS";
			case 0x237: return "ERROR_PAGEFILE_QUOTA_EXCEEDED";
			case 0x238: return "ERROR_LOGON_SERVER_CONFLICT";
			case 0x239: return "ERROR_SYNCHRONIZATION_REQUIRED";
			case 0x23A: return "ERROR_NET_OPEN_FAILED";
			case 0x23B: return "ERROR_IO_PRIVILEGE_FAILED";
			case 0x23C: return "ERROR_CONTROL_C_EXIT";
			case 0x23D: return "ERROR_MISSING_SYSTEMFILE";
			case 0x23E: return "ERROR_UNHANDLED_EXCEPTION";
			case 0x23F: return "ERROR_APP_INIT_FAILURE";
			case 0x240: return "ERROR_PAGEFILE_CREATE_FAILED";
			case 0x241: return "ERROR_INVALID_IMAGE_HASH";
			case 0x242: return "ERROR_NO_PAGEFILE";
			case 0x243: return "ERROR_ILLEGAL_FLOAT_CONTEXT";
			case 0x244: return "ERROR_NO_EVENT_PAIR";
			case 0x245: return "ERROR_DOMAIN_CTRLR_CONFIG_ERROR";
			case 0x246: return "ERROR_ILLEGAL_CHARACTER";
			case 0x247: return "ERROR_UNDEFINED_CHARACTER";
			case 0x248: return "ERROR_FLOPPY_VOLUME";
			case 0x249: return "ERROR_BIOS_FAILED_TO_CONNECT_INTERRUPT";
			case 0x24A: return "ERROR_BACKUP_CONTROLLER";
			case 0x24B: return "ERROR_MUTANT_LIMIT_EXCEEDED";
			case 0x24C: return "ERROR_FS_DRIVER_REQUIRED";
			case 0x24D: return "ERROR_CANNOT_LOAD_REGISTRY_FILE";
			case 0x24E: return "ERROR_DEBUG_ATTACH_FAILED";
			case 0x24F: return "ERROR_SYSTEM_PROCESS_TERMINATED";
			case 0x250: return "ERROR_DATA_NOT_ACCEPTED";
			case 0x251: return "ERROR_VDM_HARD_ERROR";
			case 0x252: return "ERROR_DRIVER_CANCEL_TIMEOUT";
			case 0x253: return "ERROR_REPLY_MESSAGE_MISMATCH";
			case 0x254: return "ERROR_LOST_WRITEBEHIND_DATA";
			case 0x255: return "ERROR_CLIENT_SERVER_PARAMETERS_INVALID";
			case 0x256: return "ERROR_NOT_TINY_STREAM";
			case 0x257: return "ERROR_STACK_OVERFLOW_READ";
			case 0x258: return "ERROR_CONVERT_TO_LARGE";
			case 0x259: return "ERROR_FOUND_OUT_OF_SCOPE";
			case 0x25A: return "ERROR_ALLOCATE_BUCKET";
			case 0x25B: return "ERROR_MARSHALL_OVERFLOW";
			case 0x25C: return "ERROR_INVALID_VARIANT";
			case 0x25D: return "ERROR_BAD_COMPRESSION_BUFFER";
			case 0x25E: return "ERROR_AUDIT_FAILED";
			case 0x25F: return "ERROR_TIMER_RESOLUTION_NOT_SET";
			case 0x260: return "ERROR_INSUFFICIENT_LOGON_INFO";
			case 0x261: return "ERROR_BAD_DLL_ENTRYPOINT";
			case 0x262: return "ERROR_BAD_SERVICE_ENTRYPOINT";
			case 0x263: return "ERROR_IP_ADDRESS_CONFLICT1";
			case 0x264: return "ERROR_IP_ADDRESS_CONFLICT2";
			case 0x265: return "ERROR_REGISTRY_QUOTA_LIMIT";
			case 0x266: return "ERROR_NO_CALLBACK_ACTIVE";
			case 0x267: return "ERROR_PWD_TOO_SHORT";
			case 0x268: return "ERROR_PWD_TOO_RECENT";
			case 0x269: return "ERROR_PWD_HISTORY_CONFLICT";
			case 0x26A: return "ERROR_UNSUPPORTED_COMPRESSION";
			case 0x26B: return "ERROR_INVALID_HW_PROFILE";
			case 0x26C: return "ERROR_INVALID_PLUGPLAY_DEVICE_PATH";
			case 0x26D: return "ERROR_QUOTA_LIST_INCONSISTENT";
			case 0x26E: return "ERROR_EVALUATION_EXPIRATION";
			case 0x26F: return "ERROR_ILLEGAL_DLL_RELOCATION";
			case 0x270: return "ERROR_DLL_INIT_FAILED_LOGOFF";
			case 0x271: return "ERROR_VALIDATE_CONTINUE";
			case 0x272: return "ERROR_NO_MORE_MATCHES";
			case 0x273: return "ERROR_RANGE_LIST_CONFLICT";
			case 0x274: return "ERROR_SERVER_SID_MISMATCH";
			case 0x275: return "ERROR_CANT_ENABLE_DENY_ONLY";
			case 0x276: return "ERROR_FLOAT_MULTIPLE_FAULTS";
			case 0x277: return "ERROR_FLOAT_MULTIPLE_TRAPS";
			case 0x278: return "ERROR_NOINTERFACE";
			case 0x279: return "ERROR_DRIVER_FAILED_SLEEP";
			case 0x27A: return "ERROR_CORRUPT_SYSTEM_FILE";
			case 0x27B: return "ERROR_COMMITMENT_MINIMUM";
			case 0x27C: return "ERROR_PNP_RESTART_ENUMERATION";
			case 0x27D: return "ERROR_SYSTEM_IMAGE_BAD_SIGNATURE";
			case 0x27E: return "ERROR_PNP_REBOOT_REQUIRED";
			case 0x27F: return "ERROR_INSUFFICIENT_POWER";
			case 0x280: return "ERROR_MULTIPLE_FAULT_VIOLATION";
			case 0x281: return "ERROR_SYSTEM_SHUTDOWN";
			case 0x282: return "ERROR_PORT_NOT_SET";
			case 0x283: return "ERROR_DS_VERSION_CHECK_FAILURE";
			case 0x284: return "ERROR_RANGE_NOT_FOUND";
			case 0x286: return "ERROR_NOT_SAFE_MODE_DRIVER";
			case 0x287: return "ERROR_FAILED_DRIVER_ENTRY";
			case 0x288: return "ERROR_DEVICE_ENUMERATION_ERROR";
			case 0x289: return "ERROR_MOUNT_POINT_NOT_RESOLVED";
			case 0x28A: return "ERROR_INVALID_DEVICE_OBJECT_PARAMETER";
			case 0x28B: return "ERROR_MCA_OCCURED";
			case 0x28C: return "ERROR_DRIVER_DATABASE_ERROR";
			case 0x28D: return "ERROR_SYSTEM_HIVE_TOO_LARGE";
			case 0x28E: return "ERROR_DRIVER_FAILED_PRIOR_UNLOAD";
			case 0x28F: return "ERROR_VOLSNAP_PREPARE_HIBERNATE";
			case 0x290: return "ERROR_HIBERNATION_FAILURE";
			case 0x291: return "ERROR_PWD_TOO_LONG";
			case 0x299: return "ERROR_FILE_SYSTEM_LIMITATION";
			case 0x29C: return "ERROR_ASSERTION_FAILURE";
			case 0x29D: return "ERROR_ACPI_ERROR";
			case 0x29E: return "ERROR_WOW_ASSERTION";
			case 0x29F: return "ERROR_PNP_BAD_MPS_TABLE";
			case 0x2A0: return "ERROR_PNP_TRANSLATION_FAILED";
			case 0x2A1: return "ERROR_PNP_IRQ_TRANSLATION_FAILED";
			case 0x2A2: return "ERROR_PNP_INVALID_ID";
			case 0x2A3: return "ERROR_WAKE_SYSTEM_DEBUGGER";
			case 0x2A4: return "ERROR_HANDLES_CLOSED";
			case 0x2A5: return "ERROR_EXTRANEOUS_INFORMATION";
			case 0x2A6: return "ERROR_RXACT_COMMIT_NECESSARY";
			case 0x2A7: return "ERROR_MEDIA_CHECK";
			case 0x2A8: return "ERROR_GUID_SUBSTITUTION_MADE";
			case 0x2A9: return "ERROR_STOPPED_ON_SYMLINK";
			case 0x2AA: return "ERROR_LONGJUMP";
			case 0x2AB: return "ERROR_PLUGPLAY_QUERY_VETOED";
			case 0x2AC: return "ERROR_UNWIND_CONSOLIDATE";
			case 0x2AD: return "ERROR_REGISTRY_HIVE_RECOVERED";
			case 0x2AE: return "ERROR_DLL_MIGHT_BE_INSECURE";
			case 0x2AF: return "ERROR_DLL_MIGHT_BE_INCOMPATIBLE";
			case 0x2B0: return "ERROR_DBG_EXCEPTION_NOT_HANDLED";
			case 0x2B1: return "ERROR_DBG_REPLY_LATER";
			case 0x2B2: return "ERROR_DBG_UNABLE_TO_PROVIDE_HANDLE";
			case 0x2B3: return "ERROR_DBG_TERMINATE_THREAD";
			case 0x2B4: return "ERROR_DBG_TERMINATE_PROCESS";
			case 0x2B5: return "ERROR_DBG_CONTROL_C";
			case 0x2B6: return "ERROR_DBG_PRINTEXCEPTION_C";
			case 0x2B7: return "ERROR_DBG_RIPEXCEPTION";
			case 0x2B8: return "ERROR_DBG_CONTROL_BREAK";
			case 0x2B9: return "ERROR_DBG_COMMAND_EXCEPTION";
			case 0x2BA: return "ERROR_OBJECT_NAME_EXISTS";
			case 0x2BB: return "ERROR_THREAD_WAS_SUSPENDED";
			case 0x2BC: return "ERROR_IMAGE_NOT_AT_BASE";
			case 0x2BD: return "ERROR_RXACT_STATE_CREATED";
			case 0x2BE: return "ERROR_SEGMENT_NOTIFICATION";
			case 0x2BF: return "ERROR_BAD_CURRENT_DIRECTORY";
			case 0x2C0: return "ERROR_FT_READ_RECOVERY_FROM_BACKUP";
			case 0x2C1: return "ERROR_FT_WRITE_RECOVERY";
			case 0x2C2: return "ERROR_IMAGE_MACHINE_TYPE_MISMATCH";
			case 0x2C3: return "ERROR_RECEIVE_PARTIAL";
			case 0x2C4: return "ERROR_RECEIVE_EXPEDITED";
			case 0x2C5: return "ERROR_RECEIVE_PARTIAL_EXPEDITED";
			case 0x2C6: return "ERROR_EVENT_DONE";
			case 0x2C7: return "ERROR_EVENT_PENDING";
			case 0x2C8: return "ERROR_CHECKING_FILE_SYSTEM";
			case 0x2C9: return "ERROR_FATAL_APP_EXIT";
			case 0x2CA: return "ERROR_PREDEFINED_HANDLE";
			case 0x2CB: return "ERROR_WAS_UNLOCKED";
			case 0x2CC: return "ERROR_SERVICE_NOTIFICATION";
			case 0x2CD: return "ERROR_WAS_LOCKED";
			case 0x2CE: return "ERROR_LOG_HARD_ERROR";
			case 0x2CF: return "ERROR_ALREADY_WIN32";
			case 0x2D0: return "ERROR_IMAGE_MACHINE_TYPE_MISMATCH_EXE";
			case 0x2D1: return "ERROR_NO_YIELD_PERFORMED";
			case 0x2D2: return "ERROR_TIMER_RESUME_IGNORED";
			case 0x2D3: return "ERROR_ARBITRATION_UNHANDLED";
			case 0x2D4: return "ERROR_CARDBUS_NOT_SUPPORTED";
			case 0x2D5: return "ERROR_MP_PROCESSOR_MISMATCH";
			case 0x2D6: return "ERROR_HIBERNATED";
			case 0x2D7: return "ERROR_RESUME_HIBERNATION";
			case 0x2D8: return "ERROR_FIRMWARE_UPDATED";
			case 0x2D9: return "ERROR_DRIVERS_LEAKING_LOCKED_PAGES";
			case 0x2DA: return "ERROR_WAKE_SYSTEM";
			case 0x2DB: return "ERROR_WAIT_1";
			case 0x2DC: return "ERROR_WAIT_2";
			case 0x2DD: return "ERROR_WAIT_3";
			case 0x2DE: return "ERROR_WAIT_63";
			case 0x2DF: return "ERROR_ABANDONED_WAIT_0";
			case 0x2E0: return "ERROR_ABANDONED_WAIT_63";
			case 0x2E1: return "ERROR_USER_APC";
			case 0x2E2: return "ERROR_KERNEL_APC";
			case 0x2E3: return "ERROR_ALERTED";
			case 0x2E4: return "ERROR_ELEVATION_REQUIRED";
			case 0x2E5: return "ERROR_REPARSE";
			case 0x2E6: return "ERROR_OPLOCK_BREAK_IN_PROGRESS";
			case 0x2E7: return "ERROR_VOLUME_MOUNTED";
			case 0x2E8: return "ERROR_RXACT_COMMITTED";
			case 0x2E9: return "ERROR_NOTIFY_CLEANUP";
			case 0x2EA: return "ERROR_PRIMARY_TRANSPORT_CONNECT_FAILED";
			case 0x2EB: return "ERROR_PAGE_FAULT_TRANSITION";
			case 0x2EC: return "ERROR_PAGE_FAULT_DEMAND_ZERO";
			case 0x2ED: return "ERROR_PAGE_FAULT_COPY_ON_WRITE";
			case 0x2EE: return "ERROR_PAGE_FAULT_GUARD_PAGE";
			case 0x2EF: return "ERROR_PAGE_FAULT_PAGING_FILE";
			case 0x2F0: return "ERROR_CACHE_PAGE_LOCKED";
			case 0x2F1: return "ERROR_CRASH_DUMP";
			case 0x2F2: return "ERROR_BUFFER_ALL_ZEROS";
			case 0x2F3: return "ERROR_REPARSE_OBJECT";
			case 0x2F4: return "ERROR_RESOURCE_REQUIREMENTS_CHANGED";
			case 0x2F5: return "ERROR_TRANSLATION_COMPLETE";
			case 0x2F6: return "ERROR_NOTHING_TO_TERMINATE";
			case 0x2F7: return "ERROR_PROCESS_NOT_IN_JOB";
			case 0x2F8: return "ERROR_PROCESS_IN_JOB";
			case 0x2F9: return "ERROR_VOLSNAP_HIBERNATE_READY";
			case 0x2FA: return "ERROR_FSFILTER_OP_COMPLETED_SUCCESSFULLY";
			case 0x2FB: return "ERROR_INTERRUPT_VECTOR_ALREADY_CONNECTED";
			case 0x2FC: return "ERROR_INTERRUPT_STILL_CONNECTED";
			case 0x2FD: return "ERROR_WAIT_FOR_OPLOCK";
			case 0x2FE: return "ERROR_DBG_EXCEPTION_HANDLED";
			case 0x2FF: return "ERROR_DBG_CONTINUE";
			case 0x300: return "ERROR_CALLBACK_POP_STACK";
			case 0x301: return "ERROR_COMPRESSION_DISABLED";
			case 0x302: return "ERROR_CANTFETCHBACKWARDS";
			case 0x303: return "ERROR_CANTSCROLLBACKWARDS";
			case 0x304: return "ERROR_ROWSNOTRELEASED";
			case 0x305: return "ERROR_BAD_ACCESSOR_FLAGS";
			case 0x306: return "ERROR_ERRORS_ENCOUNTERED";
			case 0x307: return "ERROR_NOT_CAPABLE";
			case 0x308: return "ERROR_REQUEST_OUT_OF_SEQUENCE";
			case 0x309: return "ERROR_VERSION_PARSE_ERROR";
			case 0x30A: return "ERROR_BADSTARTPOSITION";
			case 0x30B: return "ERROR_MEMORY_HARDWARE";
			case 0x30C: return "ERROR_DISK_REPAIR_DISABLED";
			case 0x30D: return "ERROR_INSUFFICIENT_RESOURCE_FOR_SPECIFIED_SHARED_SECTION_SIZE";
			case 0x30E: return "ERROR_SYSTEM_POWERSTATE_TRANSITION";
			case 0x30F: return "ERROR_SYSTEM_POWERSTATE_COMPLEX_TRANSITION";
			case 0x310: return "ERROR_MCA_EXCEPTION";
			case 0x311: return "ERROR_ACCESS_AUDIT_BY_POLICY";
			case 0x312: return "ERROR_ACCESS_DISABLED_NO_SAFER_UI_BY_POLICY";
			case 0x313: return "ERROR_ABANDON_HIBERFILE";
			case 0x314: return "ERROR_LOST_WRITEBEHIND_DATA_NETWORK_DISCONNECTED";
			case 0x315: return "ERROR_LOST_WRITEBEHIND_DATA_NETWORK_SERVER_ERROR";
			case 0x316: return "ERROR_LOST_WRITEBEHIND_DATA_LOCAL_DISK_ERROR";
			case 0x317: return "ERROR_BAD_MCFG_TABLE";
			case 0x318: return "ERROR_DISK_REPAIR_REDIRECTED";
			case 0x319: return "ERROR_DISK_REPAIR_UNSUCCESSFUL";
			case 0x31A: return "ERROR_CORRUPT_LOG_OVERFULL";
			case 0x31B: return "ERROR_CORRUPT_LOG_CORRUPTED";
			case 0x31C: return "ERROR_CORRUPT_LOG_UNAVAILABLE";
			case 0x31D: return "ERROR_CORRUPT_LOG_DELETED_FULL";
			case 0x31E: return "ERROR_CORRUPT_LOG_CLEARED";
			case 0x31F: return "ERROR_ORPHAN_NAME_EXHAUSTED";
			case 0x320: return "ERROR_OPLOCK_SWITCHED_TO_NEW_HANDLE";
			case 0x321: return "ERROR_CANNOT_GRANT_REQUESTED_OPLOCK";
			case 0x322: return "ERROR_CANNOT_BREAK_OPLOCK";
			case 0x323: return "ERROR_OPLOCK_HANDLE_CLOSED";
			case 0x324: return "ERROR_NO_ACE_CONDITION";
			case 0x325: return "ERROR_INVALID_ACE_CONDITION";
			case 0x326: return "ERROR_FILE_HANDLE_REVOKED";
			case 0x327: return "ERROR_IMAGE_AT_DIFFERENT_BASE";
			case 0x3E2: return "ERROR_EA_ACCESS_DENIED";
			case 0x3E3: return "ERROR_OPERATION_ABORTED";
			case 0x3E4: return "ERROR_IO_INCOMPLETE";
			case 0x3E5: return "ERROR_IO_PENDING";
			case 0x3E6: return "ERROR_NOACCESS";
			case 0x3E7: return "ERROR_SWAPERROR";
			case 0x3E9: return "ERROR_STACK_OVERFLOW";
			case 0x3EA: return "ERROR_INVALID_MESSAGE";
			case 0x3EB: return "ERROR_CAN_NOT_COMPLETE";
			case 0x3EC: return "ERROR_INVALID_FLAGS";
			case 0x3ED: return "ERROR_UNRECOGNIZED_VOLUME";
			case 0x3EE: return "ERROR_FILE_INVALID";
			case 0x3EF: return "ERROR_FULLSCREEN_MODE";
			case 0x3F0: return "ERROR_NO_TOKEN";
			case 0x3F1: return "ERROR_BADDB";
			case 0x3F2: return "ERROR_BADKEY";
			case 0x3F3: return "ERROR_CANTOPEN";
			case 0x3F4: return "ERROR_CANTREAD";
			case 0x3F5: return "ERROR_CANTWRITE";
			case 0x3F6: return "ERROR_REGISTRY_RECOVERED";
			case 0x3F7: return "ERROR_REGISTRY_CORRUPT";
			case 0x3F8: return "ERROR_REGISTRY_IO_FAILED";
			case 0x3F9: return "ERROR_NOT_REGISTRY_FILE";
			case 0x3FA: return "ERROR_KEY_DELETED";
			case 0x3FB: return "ERROR_NO_LOG_SPACE";
			case 0x3FC: return "ERROR_KEY_HAS_CHILDREN";
			case 0x3FD: return "ERROR_CHILD_MUST_BE_VOLATILE";
			case 0x3FE: return "ERROR_NOTIFY_ENUM_DIR";
			case 0x41B: return "ERROR_DEPENDENT_SERVICES_RUNNING";
			case 0x41C: return "ERROR_INVALID_SERVICE_CONTROL";
			case 0x41D: return "ERROR_SERVICE_REQUEST_TIMEOUT";
			case 0x41E: return "ERROR_SERVICE_NO_THREAD";
			case 0x41F: return "ERROR_SERVICE_DATABASE_LOCKED";
			case 0x420: return "ERROR_SERVICE_ALREADY_RUNNING";
			case 0x421: return "ERROR_INVALID_SERVICE_ACCOUNT";
			case 0x422: return "ERROR_SERVICE_DISABLED";
			case 0x423: return "ERROR_CIRCULAR_DEPENDENCY";
			case 0x424: return "ERROR_SERVICE_DOES_NOT_EXIST";
			case 0x425: return "ERROR_SERVICE_CANNOT_ACCEPT_CTRL";
			case 0x426: return "ERROR_SERVICE_NOT_ACTIVE";
			case 0x427: return "ERROR_FAILED_SERVICE_CONTROLLER_CONNECT";
			case 0x428: return "ERROR_EXCEPTION_IN_SERVICE";
			case 0x429: return "ERROR_DATABASE_DOES_NOT_EXIST";
			case 0x42A: return "ERROR_SERVICE_SPECIFIC_ERROR";
			case 0x42B: return "ERROR_PROCESS_ABORTED";
			case 0x42C: return "ERROR_SERVICE_DEPENDENCY_FAIL";
			case 0x42D: return "ERROR_SERVICE_LOGON_FAILED";
			case 0x42E: return "ERROR_SERVICE_START_HANG";
			case 0x42F: return "ERROR_INVALID_SERVICE_LOCK";
			case 0x430: return "ERROR_SERVICE_MARKED_FOR_DELETE";
			case 0x431: return "ERROR_SERVICE_EXISTS";
			case 0x432: return "ERROR_ALREADY_RUNNING_LKG";
			case 0x433: return "ERROR_SERVICE_DEPENDENCY_DELETED";
			case 0x434: return "ERROR_BOOT_ALREADY_ACCEPTED";
			case 0x435: return "ERROR_SERVICE_NEVER_STARTED";
			case 0x436: return "ERROR_DUPLICATE_SERVICE_NAME";
			case 0x437: return "ERROR_DIFFERENT_SERVICE_ACCOUNT";
			case 0x438: return "ERROR_CANNOT_DETECT_DRIVER_FAILURE";
			case 0x439: return "ERROR_CANNOT_DETECT_PROCESS_ABORT";
			case 0x43A: return "ERROR_NO_RECOVERY_PROGRAM";
			case 0x43B: return "ERROR_SERVICE_NOT_IN_EXE";
			case 0x43C: return "ERROR_NOT_SAFEBOOT_SERVICE";
			case 0x44C: return "ERROR_END_OF_MEDIA";
			case 0x44D: return "ERROR_FILEMARK_DETECTED";
			case 0x44E: return "ERROR_BEGINNING_OF_MEDIA";
			case 0x44F: return "ERROR_SETMARK_DETECTED";
			case 0x450: return "ERROR_NO_DATA_DETECTED";
			case 0x451: return "ERROR_PARTITION_FAILURE";
			case 0x452: return "ERROR_INVALID_BLOCK_LENGTH";
			case 0x453: return "ERROR_DEVICE_NOT_PARTITIONED";
			case 0x454: return "ERROR_UNABLE_TO_LOCK_MEDIA";
			case 0x455: return "ERROR_UNABLE_TO_UNLOAD_MEDIA";
			case 0x456: return "ERROR_MEDIA_CHANGED";
			case 0x457: return "ERROR_BUS_RESET";
			case 0x458: return "ERROR_NO_MEDIA_IN_DRIVE";
			case 0x459: return "ERROR_NO_UNICODE_TRANSLATION";
			case 0x45A: return "ERROR_DLL_INIT_FAILED";
			case 0x45B: return "ERROR_SHUTDOWN_IN_PROGRESS";
			case 0x45C: return "ERROR_NO_SHUTDOWN_IN_PROGRESS";
			case 0x45D: return "ERROR_IO_DEVICE";
			case 0x45E: return "ERROR_SERIAL_NO_DEVICE";
			case 0x45F: return "ERROR_IRQ_BUSY";
			case 0x460: return "ERROR_MORE_WRITES";
			case 0x461: return "ERROR_COUNTER_TIMEOUT";
			case 0x462: return "ERROR_FLOPPY_ID_MARK_NOT_FOUND";
			case 0x463: return "ERROR_FLOPPY_WRONG_CYLINDER";
			case 0x464: return "ERROR_FLOPPY_UNKNOWN_ERROR";
			case 0x465: return "ERROR_FLOPPY_BAD_REGISTERS";
			case 0x466: return "ERROR_DISK_RECALIBRATE_FAILED";
			case 0x467: return "ERROR_DISK_OPERATION_FAILED";
			case 0x468: return "ERROR_DISK_RESET_FAILED";
			case 0x469: return "ERROR_EOM_OVERFLOW";
			case 0x46A: return "ERROR_NOT_ENOUGH_SERVER_MEMORY";
			case 0x46B: return "ERROR_POSSIBLE_DEADLOCK";
			case 0x46C: return "ERROR_MAPPED_ALIGNMENT";
			case 0x474: return "ERROR_SET_POWER_STATE_VETOED";
			case 0x475: return "ERROR_SET_POWER_STATE_FAILED";
			case 0x476: return "ERROR_TOO_MANY_LINKS";
			case 0x47E: return "ERROR_OLD_WIN_VERSION";
			case 0x47F: return "ERROR_APP_WRONG_OS";
			case 0x480: return "ERROR_SINGLE_INSTANCE_APP";
			case 0x481: return "ERROR_RMODE_APP";
			case 0x482: return "ERROR_INVALID_DLL";
			case 0x483: return "ERROR_NO_ASSOCIATION";
			case 0x484: return "ERROR_DDE_FAIL";
			case 0x485: return "ERROR_DLL_NOT_FOUND";
			case 0x486: return "ERROR_NO_MORE_USER_HANDLES";
			case 0x487: return "ERROR_MESSAGE_SYNC_ONLY";
			case 0x488: return "ERROR_SOURCE_ELEMENT_EMPTY";
			case 0x489: return "ERROR_DESTINATION_ELEMENT_FULL";
			case 0x48A: return "ERROR_ILLEGAL_ELEMENT_ADDRESS";
			case 0x48B: return "ERROR_MAGAZINE_NOT_PRESENT";
			case 0x48C: return "ERROR_DEVICE_REINITIALIZATION_NEEDED";
			case 0x48D: return "ERROR_DEVICE_REQUIRES_CLEANING";
			case 0x48E: return "ERROR_DEVICE_DOOR_OPEN";
			case 0x48F: return "ERROR_DEVICE_NOT_CONNECTED";
			case 0x490: return "ERROR_NOT_FOUND";
			case 0x491: return "ERROR_NO_MATCH";
			case 0x492: return "ERROR_SET_NOT_FOUND";
			case 0x493: return "ERROR_POINT_NOT_FOUND";
			case 0x494: return "ERROR_NO_TRACKING_SERVICE";
			case 0x495: return "ERROR_NO_VOLUME_ID";
			case 0x497: return "ERROR_UNABLE_TO_REMOVE_REPLACED";
			case 0x498: return "ERROR_UNABLE_TO_MOVE_REPLACEMENT";
			case 0x499: return "ERROR_UNABLE_TO_MOVE_REPLACEMENT_2";
			case 0x49A: return "ERROR_JOURNAL_DELETE_IN_PROGRESS";
			case 0x49B: return "ERROR_JOURNAL_NOT_ACTIVE";
			case 0x49C: return "ERROR_POTENTIAL_FILE_FOUND";
			case 0x49D: return "ERROR_JOURNAL_ENTRY_DELETED";
			case 0x4A6: return "ERROR_SHUTDOWN_IS_SCHEDULED";
			case 0x4A7: return "ERROR_SHUTDOWN_USERS_LOGGED_ON";
			case 0x4B0: return "ERROR_BAD_DEVICE";
			case 0x4B1: return "ERROR_CONNECTION_UNAVAIL";
			case 0x4B2: return "ERROR_DEVICE_ALREADY_REMEMBERED";
			case 0x4B3: return "ERROR_NO_NET_OR_BAD_PATH";
			case 0x4B4: return "ERROR_BAD_PROVIDER";
			case 0x4B5: return "ERROR_CANNOT_OPEN_PROFILE";
			case 0x4B6: return "ERROR_BAD_PROFILE";
			case 0x4B7: return "ERROR_NOT_CONTAINER";
			case 0x4B8: return "ERROR_EXTENDED_ERROR";
			case 0x4B9: return "ERROR_INVALID_GROUPNAME";
			case 0x4BA: return "ERROR_INVALID_COMPUTERNAME";
			case 0x4BB: return "ERROR_INVALID_EVENTNAME";
			case 0x4BC: return "ERROR_INVALID_DOMAINNAME";
			case 0x4BD: return "ERROR_INVALID_SERVICENAME";
			case 0x4BE: return "ERROR_INVALID_NETNAME";
			case 0x4BF: return "ERROR_INVALID_SHARENAME";
			case 0x4C0: return "ERROR_INVALID_PASSWORDNAME";
			case 0x4C1: return "ERROR_INVALID_MESSAGENAME";
			case 0x4C2: return "ERROR_INVALID_MESSAGEDEST";
			case 0x4C3: return "ERROR_SESSION_CREDENTIAL_CONFLICT";
			case 0x4C4: return "ERROR_REMOTE_SESSION_LIMIT_EXCEEDED";
			case 0x4C5: return "ERROR_DUP_DOMAINNAME";
			case 0x4C6: return "ERROR_NO_NETWORK";
			case 0x4C7: return "ERROR_CANCELLED";
			case 0x4C8: return "ERROR_USER_MAPPED_FILE";
			case 0x4C9: return "ERROR_CONNECTION_REFUSED";
			case 0x4CA: return "ERROR_GRACEFUL_DISCONNECT";
			case 0x4CB: return "ERROR_ADDRESS_ALREADY_ASSOCIATED";
			case 0x4CC: return "ERROR_ADDRESS_NOT_ASSOCIATED";
			case 0x4CD: return "ERROR_CONNECTION_INVALID";
			case 0x4CE: return "ERROR_CONNECTION_ACTIVE";
			case 0x4CF: return "ERROR_NETWORK_UNREACHABLE";
			case 0x4D0: return "ERROR_HOST_UNREACHABLE";
			case 0x4D1: return "ERROR_PROTOCOL_UNREACHABLE";
			case 0x4D2: return "ERROR_PORT_UNREACHABLE";
			case 0x4D3: return "ERROR_REQUEST_ABORTED";
			case 0x4D4: return "ERROR_CONNECTION_ABORTED";
			case 0x4D5: return "ERROR_RETRY";
			case 0x4D6: return "ERROR_CONNECTION_COUNT_LIMIT";
			case 0x4D7: return "ERROR_LOGIN_TIME_RESTRICTION";
			case 0x4D8: return "ERROR_LOGIN_WKSTA_RESTRICTION";
			case 0x4D9: return "ERROR_INCORRECT_ADDRESS";
			case 0x4DA: return "ERROR_ALREADY_REGISTERED";
			case 0x4DB: return "ERROR_SERVICE_NOT_FOUND";
			case 0x4DC: return "ERROR_NOT_AUTHENTICATED";
			case 0x4DD: return "ERROR_NOT_LOGGED_ON";
			case 0x4DE: return "ERROR_CONTINUE";
			case 0x4DF: return "ERROR_ALREADY_INITIALIZED";
			case 0x4E0: return "ERROR_NO_MORE_DEVICES";
			case 0x4E1: return "ERROR_NO_SUCH_SITE";
			case 0x4E2: return "ERROR_DOMAIN_CONTROLLER_EXISTS";
			case 0x4E3: return "ERROR_ONLY_IF_CONNECTED";
			case 0x4E4: return "ERROR_OVERRIDE_NOCHANGES";
			case 0x4E5: return "ERROR_BAD_USER_PROFILE";
			case 0x4E6: return "ERROR_NOT_SUPPORTED_ON_SBS";
			case 0x4E7: return "ERROR_SERVER_SHUTDOWN_IN_PROGRESS";
			case 0x4E8: return "ERROR_HOST_DOWN";
			case 0x4E9: return "ERROR_NON_ACCOUNT_SID";
			case 0x4EA: return "ERROR_NON_DOMAIN_SID";
			case 0x4EB: return "ERROR_APPHELP_BLOCK";
			case 0x4EC: return "ERROR_ACCESS_DISABLED_BY_POLICY";
			case 0x4ED: return "ERROR_REG_NAT_CONSUMPTION";
			case 0x4EE: return "ERROR_CSCSHARE_OFFLINE";
			case 0x4EF: return "ERROR_PKINIT_FAILURE";
			case 0x4F0: return "ERROR_SMARTCARD_SUBSYSTEM_FAILURE";
			case 0x4F1: return "ERROR_DOWNGRADE_DETECTED";
			case 0x4F7: return "ERROR_MACHINE_LOCKED";
			case 0x4F9: return "ERROR_CALLBACK_SUPPLIED_INVALID_DATA";
			case 0x4FA: return "ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED";
			case 0x4FB: return "ERROR_DRIVER_BLOCKED";
			case 0x4FC: return "ERROR_INVALID_IMPORT_OF_NON_DLL";
			case 0x4FD: return "ERROR_ACCESS_DISABLED_WEBBLADE";
			case 0x4FE: return "ERROR_ACCESS_DISABLED_WEBBLADE_TAMPER";
			case 0x4FF: return "ERROR_RECOVERY_FAILURE";
			case 0x500: return "ERROR_ALREADY_FIBER";
			case 0x501: return "ERROR_ALREADY_THREAD";
			case 0x502: return "ERROR_STACK_BUFFER_OVERRUN";
			case 0x503: return "ERROR_PARAMETER_QUOTA_EXCEEDED";
			case 0x504: return "ERROR_DEBUGGER_INACTIVE";
			case 0x505: return "ERROR_DELAY_LOAD_FAILED";
			case 0x506: return "ERROR_VDM_DISALLOWED";
			case 0x507: return "ERROR_UNIDENTIFIED_ERROR";
			case 0x508: return "ERROR_INVALID_CRUNTIME_PARAMETER";
			case 0x509: return "ERROR_BEYOND_VDL";
			case 0x50A: return "ERROR_INCOMPATIBLE_SERVICE_SID_TYPE";
			case 0x50B: return "ERROR_DRIVER_PROCESS_TERMINATED";
			case 0x50C: return "ERROR_IMPLEMENTATION_LIMIT";
			case 0x50D: return "ERROR_PROCESS_IS_PROTECTED";
			case 0x50E: return "ERROR_SERVICE_NOTIFY_CLIENT_LAGGING";
			case 0x50F: return "ERROR_DISK_QUOTA_EXCEEDED";
			case 0x510: return "ERROR_CONTENT_BLOCKED";
			case 0x511: return "ERROR_INCOMPATIBLE_SERVICE_PRIVILEGE";
			case 0x512: return "ERROR_APP_HANG";
			case 0x513: return "ERROR_INVALID_LABEL";
		}
		return "ERROR_UNKNOWN";
	}

	const char* getErrorDescription(DWORD reason) {
		switch(reason) {
			case 0x0: return "The operation completed successfully.";
			case 0x1: return "Incorrect function.";
			case 0x2: return "The system cannot find the file specified.";
			case 0x3: return "The system cannot find the path specified.";
			case 0x4: return "The system cannot open the file.";
			case 0x5: return "Access is denied.";
			case 0x6: return "The handle is invalid.";
			case 0x7: return "The storage control blocks were destroyed.";
			case 0x8: return "Not enough storage is available to process this command.";
			case 0x9: return "The storage control block address is invalid.";
			case 0xA: return "The environment is incorrect.";
			case 0xB: return "An attempt was made to load a program with an incorrect format.";
			case 0xC: return "The access code is invalid.";
			case 0xD: return "The data is invalid.";
			case 0xE: return "Not enough storage is available to complete this operation.";
			case 0xF: return "The system cannot find the drive specified.";
			case 0x10: return "The directory cannot be removed.";
			case 0x11: return "The system cannot move the file to a different disk drive.";
			case 0x12: return "There are no more files.";
			case 0x13: return "The media is write protected.";
			case 0x14: return "The system cannot find the device specified.";
			case 0x15: return "The device is not ready.";
			case 0x16: return "The device does not recognize the command.";
			case 0x17: return "Data error (cyclic redundancy check).";
			case 0x18: return "The program issued a command but the command length is incorrect.";
			case 0x19: return "The drive cannot locate a specific area or track on the disk.";
			case 0x1A: return "The specified disk or diskette cannot be accessed.";
			case 0x1B: return "The drive cannot find the sector requested.";
			case 0x1C: return "The printer is out of paper.";
			case 0x1D: return "The system cannot write to the specified device.";
			case 0x1E: return "The system cannot read from the specified device.";
			case 0x1F: return "A device attached to the system is not functioning.";
			case 0x20: return "The process cannot access the file because it is being used by another process.";
			case 0x21: return "The process cannot access the file because another process has locked a portion of the file.";
			case 0x22: return "The wrong diskette is in the drive. Insert %2 (Volume Serial Number: %3) into drive %1.";
			case 0x24: return "Too many files opened for sharing.";
			case 0x26: return "Reached the end of the file.";
			case 0x27: return "The disk is full.";
			case 0x32: return "The request is not supported.";
			case 0x33: return "Windows cannot find the network path. Verify that the network path is correct and the destination computer is not busy or turned off. If Windows still cannot find the network path, contact your network administrator.";
			case 0x34: return "You were not connected because a duplicate name exists on the network. If joining a domain, go to System in Control Panel to change the computer name and try again. If joining a workgroup, choose another workgroup name.";
			case 0x35: return "The network path was not found.";
			case 0x36: return "The network is busy.";
			case 0x37: return "The specified network resource or device is no longer available.";
			case 0x38: return "The network BIOS command limit has been reached.";
			case 0x39: return "A network adapter hardware error occurred.";
			case 0x3A: return "The specified server cannot perform the requested operation.";
			case 0x3B: return "An unexpected network error occurred.";
			case 0x3C: return "The remote adapter is not compatible.";
			case 0x3D: return "The printer queue is full.";
			case 0x3E: return "Space to store the file waiting to be printed is not available on the server.";
			case 0x3F: return "Your file waiting to be printed was deleted.";
			case 0x40: return "The specified network name is no longer available.";
			case 0x41: return "Network access is denied.";
			case 0x42: return "The network resource type is not correct.";
			case 0x43: return "The network name cannot be found.";
			case 0x44: return "The name limit for the local computer network adapter card was exceeded.";
			case 0x45: return "The network BIOS session limit was exceeded.";
			case 0x46: return "The remote server has been paused or is in the process of being started.";
			case 0x47: return "No more connections can be made to this remote computer at this time because there are already as many connections as the computer can accept.";
			case 0x48: return "The specified printer or disk device has been paused.";
			case 0x50: return "The file exists.";
			case 0x52: return "The directory or file cannot be created.";
			case 0x53: return "Fail on INT 24.";
			case 0x54: return "Storage to process this request is not available.";
			case 0x55: return "The local device name is already in use.";
			case 0x56: return "The specified network password is not correct.";
			case 0x57: return "The parameter is incorrect.";
			case 0x58: return "A write fault occurred on the network.";
			case 0x59: return "The system cannot start another process at this time.";
			case 0x64: return "Cannot create another system semaphore.";
			case 0x65: return "The exclusive semaphore is owned by another process.";
			case 0x66: return "The semaphore is set and cannot be closed.";
			case 0x67: return "The semaphore cannot be set again.";
			case 0x68: return "Cannot request exclusive semaphores at interrupt time.";
			case 0x69: return "The previous ownership of this semaphore has ended.";
			case 0x6A: return "Insert the diskette for drive %1.";
			case 0x6B: return "The program stopped because an alternate diskette was not inserted.";
			case 0x6C: return "The disk is in use or locked by another process.";
			case 0x6D: return "The pipe has been ended.";
			case 0x6E: return "The system cannot open the device or file specified.";
			case 0x6F: return "The file name is too long.";
			case 0x70: return "There is not enough space on the disk.";
			case 0x71: return "No more internal file identifiers available.";
			case 0x72: return "The target internal file identifier is incorrect.";
			case 0x75: return "The IOCTL call made by the application program is not correct.";
			case 0x76: return "The verify-on-write switch parameter value is not correct.";
			case 0x77: return "The system does not support the command requested.";
			case 0x78: return "This function is not supported on this system.";
			case 0x79: return "The semaphore timeout period has expired.";
			case 0x7A: return "The data area passed to a system call is too small.";
			case 0x7B: return "The filename, directory name, or volume label syntax is incorrect.";
			case 0x7C: return "The system call level is not correct.";
			case 0x7D: return "The disk has no volume label.";
			case 0x7E: return "The specified module could not be found.";
			case 0x7F: return "The specified procedure could not be found.";
			case 0x80: return "There are no child processes to wait for.";
			case 0x81: return "The %1 application cannot be run in Win32 mode.";
			case 0x82: return "Attempt to use a file handle to an open disk partition for an operation other than raw disk I/O.";
			case 0x83: return "An attempt was made to move the file pointer before the beginning of the file.";
			case 0x84: return "The file pointer cannot be set on the specified device or file.";
			case 0x85: return "A JOIN or SUBST command cannot be used for a drive that contains previously joined drives.";
			case 0x86: return "An attempt was made to use a JOIN or SUBST command on a drive that has already been joined.";
			case 0x87: return "An attempt was made to use a JOIN or SUBST command on a drive that has already been substituted.";
			case 0x88: return "The system tried to delete the JOIN of a drive that is not joined.";
			case 0x89: return "The system tried to delete the substitution of a drive that is not substituted.";
			case 0x8A: return "The system tried to join a drive to a directory on a joined drive.";
			case 0x8B: return "The system tried to substitute a drive to a directory on a substituted drive.";
			case 0x8C: return "The system tried to join a drive to a directory on a substituted drive.";
			case 0x8D: return "The system tried to SUBST a drive to a directory on a joined drive.";
			case 0x8E: return "The system cannot perform a JOIN or SUBST at this time.";
			case 0x8F: return "The system cannot join or substitute a drive to or for a directory on the same drive.";
			case 0x90: return "The directory is not a subdirectory of the root directory.";
			case 0x91: return "The directory is not empty.";
			case 0x92: return "The path specified is being used in a substitute.";
			case 0x93: return "Not enough resources are available to process this command.";
			case 0x94: return "The path specified cannot be used at this time.";
			case 0x95: return "An attempt was made to join or substitute a drive for which a directory on the drive is the target of a previous substitute.";
			case 0x96: return "System trace information was not specified in your CONFIG.SYS file, or tracing is disallowed.";
			case 0x97: return "The number of specified semaphore events for DosMuxSemWait is not correct.";
			case 0x98: return "DosMuxSemWait did not execute; too many semaphores are already set.";
			case 0x99: return "The DosMuxSemWait list is not correct.";
			case 0x9A: return "The volume label you entered exceeds the label character limit of the target file system.";
			case 0x9B: return "Cannot create another thread.";
			case 0x9C: return "The recipient process has refused the signal.";
			case 0x9D: return "The segment is already discarded and cannot be locked.";
			case 0x9E: return "The segment is already unlocked.";
			case 0x9F: return "The address for the thread ID is not correct.";
			case 0xA0: return "One or more arguments are not correct.";
			case 0xA1: return "The specified path is invalid.";
			case 0xA2: return "A signal is already pending.";
			case 0xA4: return "No more threads can be created in the system.";
			case 0xA7: return "Unable to lock a region of a file.";
			case 0xAA: return "The requested resource is in use.";
			case 0xAB: return "Device's command support detection is in progress.";
			case 0xAD: return "A lock request was not outstanding for the supplied cancel region.";
			case 0xAE: return "The file system does not support atomic changes to the lock type.";
			case 0xB4: return "The system detected a segment number that was not correct.";
			case 0xB6: return "The operating system cannot run %1.";
			case 0xB7: return "Cannot create a file when that file already exists.";
			case 0xBA: return "The flag passed is not correct.";
			case 0xBB: return "The specified system semaphore name was not found.";
			case 0xBC: return "The operating system cannot run %1.";
			case 0xBD: return "The operating system cannot run %1.";
			case 0xBE: return "The operating system cannot run %1.";
			case 0xBF: return "Cannot run %1 in Win32 mode.";
			case 0xC0: return "The operating system cannot run %1.";
			case 0xC1: return "%1 is not a valid Win32 application.";
			case 0xC2: return "The operating system cannot run %1.";
			case 0xC3: return "The operating system cannot run %1.";
			case 0xC4: return "The operating system cannot run this application program.";
			case 0xC5: return "The operating system is not presently configured to run this application.";
			case 0xC6: return "The operating system cannot run %1.";
			case 0xC7: return "The operating system cannot run this application program.";
			case 0xC8: return "The code segment cannot be greater than or equal to 64K.";
			case 0xC9: return "The operating system cannot run %1.";
			case 0xCA: return "The operating system cannot run %1.";
			case 0xCB: return "The system could not find the environment option that was entered.";
			case 0xCD: return "No process in the command subtree has a signal handler.";
			case 0xCE: return "The filename or extension is too long.";
			case 0xCF: return "The ring 2 stack is in use.";
			case 0xD0: return "The global filename characters, * or ?, are entered incorrectly or too many global filename characters are specified.";
			case 0xD1: return "The signal being posted is not correct.";
			case 0xD2: return "The signal handler cannot be set.";
			case 0xD4: return "The segment is locked and cannot be reallocated.";
			case 0xD6: return "Too many dynamic-link modules are attached to this program or dynamic-link module.";
			case 0xD7: return "Cannot nest calls to LoadModule.";
			case 0xD8: return "This version of %1 is not compatible with the version of Windows you're running. Check your computer's system information and then contact the software publisher.";
			case 0xD9: return "The image file %1 is signed, unable to modify.";
			case 0xDA: return "The image file %1 is strong signed, unable to modify.";
			case 0xDC: return "This file is checked out or locked for editing by another user.";
			case 0xDD: return "The file must be checked out before saving changes.";
			case 0xDE: return "The file type being saved or retrieved has been blocked.";
			case 0xDF: return "The file size exceeds the limit allowed and cannot be saved.";
			case 0xE0: return "Access Denied. Before opening files in this location, you must first add the web site to your trusted sites list, browse to the web site, and select the option to login automatically.";
			case 0xE1: return "Operation did not complete successfully because the file contains a virus or potentially unwanted software.";
			case 0xE2: return "This file contains a virus or potentially unwanted software and cannot be opened. Due to the nature of this virus or potentially unwanted software, the file has been removed from this location.";
			case 0xE5: return "The pipe is local.";
			case 0xE6: return "The pipe state is invalid.";
			case 0xE7: return "All pipe instances are busy.";
			case 0xE8: return "The pipe is being closed.";
			case 0xE9: return "No process is on the other end of the pipe.";
			case 0xEA: return "More data is available.";
			case 0xF0: return "The session was canceled.";
			case 0xFE: return "The specified extended attribute name was invalid.";
			case 0xFF: return "The extended attributes are inconsistent.";
			case 0x103: return "No more data is available.";
			case 0x10A: return "The copy functions cannot be used.";
			case 0x10B: return "The directory name is invalid.";
			case 0x113: return "The extended attributes did not fit in the buffer.";
			case 0x114: return "The extended attribute file on the mounted file system is corrupt.";
			case 0x115: return "The extended attribute table file is full.";
			case 0x116: return "The specified extended attribute handle is invalid.";
			case 0x11A: return "The mounted file system does not support extended attributes.";
			case 0x120: return "Attempt to release mutex not owned by caller.";
			case 0x12A: return "Too many posts were made to a semaphore.";
			case 0x12B: return "Only part of a ReadProcessMemory or WriteProcessMemory request was completed.";
			case 0x12C: return "The oplock request is denied.";
			case 0x12D: return "An invalid oplock acknowledgment was received by the system.";
			case 0x12E: return "The volume is too fragmented to complete this operation.";
			case 0x12F: return "The file cannot be opened because it is in the process of being deleted.";
			case 0x130: return "Short name settings may not be changed on this volume due to the global registry setting.";
			case 0x131: return "Short names are not enabled on this volume.";
			case 0x132: return "The security stream for the given volume is in an inconsistent state. Please run CHKDSK on the volume.";
			case 0x133: return "A requested file lock operation cannot be processed due to an invalid byte range.";
			case 0x134: return "The subsystem needed to support the image type is not present.";
			case 0x135: return "The specified file already has a notification GUID associated with it.";
			case 0x136: return "An invalid exception handler routine has been detected.";
			case 0x137: return "Duplicate privileges were specified for the token.";
			case 0x138: return "No ranges for the specified operation were able to be processed.";
			case 0x139: return "Operation is not allowed on a file system internal file.";
			case 0x13A: return "The physical resources of this disk have been exhausted.";
			case 0x13B: return "The token representing the data is invalid.";
			case 0x13C: return "The device does not support the command feature.";
			case 0x13D: return "The system cannot find message text for message number 0x%1 in the message file for %2.";
			case 0x13E: return "The scope specified was not found.";
			case 0x13F: return "The Central Access Policy specified is not defined on the target machine.";
			case 0x140: return "The Central Access Policy obtained from Active Directory is invalid.";
			case 0x141: return "The device is unreachable.";
			case 0x142: return "The target device has insufficient resources to complete the operation.";
			case 0x143: return "A data integrity checksum error occurred. Data in the file stream is corrupt.";
			case 0x144: return "An attempt was made to modify both a KERNEL and normal Extended Attribute (EA) in the same operation.";
			case 0x146: return "Device does not support file-level TRIM.";
			case 0x147: return "The command specified a data offset that does not align to the device's granularity/alignment.";
			case 0x148: return "The command specified an invalid field in its parameter list.";
			case 0x149: return "An operation is currently in progress with the device.";
			case 0x14A: return "An attempt was made to send down the command via an invalid path to the target device.";
			case 0x14B: return "The command specified a number of descriptors that exceeded the maximum supported by the device.";
			case 0x14C: return "Scrub is disabled on the specified file.";
			case 0x14D: return "The storage device does not provide redundancy.";
			case 0x14E: return "An operation is not supported on a resident file.";
			case 0x14F: return "An operation is not supported on a compressed file.";
			case 0x150: return "An operation is not supported on a directory.";
			case 0x151: return "The specified copy of the requested data could not be read.";
			case 0x15E: return "No action was taken as a system reboot is required.";
			case 0x15F: return "The shutdown operation failed.";
			case 0x160: return "The restart operation failed.";
			case 0x161: return "The maximum number of sessions has been reached.";
			case 0x190: return "The thread is already in background processing mode.";
			case 0x191: return "The thread is not in background processing mode.";
			case 0x192: return "The process is already in background processing mode.";
			case 0x193: return "The process is not in background processing mode.";
			case 0x1E7: return "Attempt to access invalid address.";
			case 0x1F4: return "User profile cannot be loaded.";
			case 0x216: return "Arithmetic result exceeded 32 bits.";
			case 0x217: return "There is a process on other end of the pipe.";
			case 0x218: return "Waiting for a process to open the other end of the pipe.";
			case 0x219: return "Application verifier has found an error in the current process.";
			case 0x21A: return "An error occurred in the ABIOS subsystem.";
			case 0x21B: return "A warning occurred in the WX86 subsystem.";
			case 0x21C: return "An error occurred in the WX86 subsystem.";
			case 0x21D: return "An attempt was made to cancel or set a timer that has an associated APC and the subject thread is not the thread that originally set the timer with an associated APC routine.";
			case 0x21E: return "Unwind exception code.";
			case 0x21F: return "An invalid or unaligned stack was encountered during an unwind operation.";
			case 0x220: return "An invalid unwind target was encountered during an unwind operation.";
			case 0x221: return "Invalid Object Attributes specified to NtCreatePort or invalid Port Attributes specified to NtConnectPort";
			case 0x222: return "Length of message passed to NtRequestPort or NtRequestWaitReplyPort was longer than the maximum message allowed by the port.";
			case 0x223: return "An attempt was made to lower a quota limit below the current usage.";
			case 0x224: return "An attempt was made to attach to a device that was already attached to another device.";
			case 0x225: return "An attempt was made to execute an instruction at an unaligned address and the host system does not support unaligned instruction references.";
			case 0x226: return "Profiling not started.";
			case 0x227: return "Profiling not stopped.";
			case 0x228: return "The passed ACL did not contain the minimum required information.";
			case 0x229: return "The number of active profiling objects is at the maximum and no more may be started.";
			case 0x22A: return "Used to indicate that an operation cannot continue without blocking for I/O.";
			case 0x22B: return "Indicates that a thread attempted to terminate itself by default (called NtTerminateThread with <strong>nullptr</strong>) and it was the last thread in the current process.";
			case 0x22C: return "If an MM error is returned which is not defined in the standard FsRtl filter, it is converted to one of the following errors which is guaranteed to be in the filter. In this case information is lost, however, the filter correctly handles the exception.";
			case 0x22D: return "If an MM error is returned which is not defined in the standard FsRtl filter, it is converted to one of the following errors which is guaranteed to be in the filter. In this case information is lost, however, the filter correctly handles the exception.";
			case 0x22E: return "If an MM error is returned which is not defined in the standard FsRtl filter, it is converted to one of the following errors which is guaranteed to be in the filter. In this case information is lost, however, the filter correctly handles the exception.";
			case 0x22F: return "A malformed function table was encountered during an unwind operation.";
			case 0x230: return "Indicates that an attempt was made to assign protection to a file system file or directory and one of the SIDs in the security descriptor could not be translated into a GUID that could be stored by the file system. This causes the protection attempt to fail, which may cause a file creation attempt to fail.";
			case 0x231: return "Indicates that an attempt was made to grow an LDT by setting its size, or that the size was not an even number of selectors.";
			case 0x233: return "Indicates that the starting value for the LDT information was not an integral multiple of the selector size.";
			case 0x234: return "Indicates that the user supplied an invalid descriptor when trying to set up Ldt descriptors.";
			case 0x235: return "Indicates a process has too many threads to perform the requested action. For example, assignment of a primary token may only be performed when a process has zero or one threads.";
			case 0x236: return "An attempt was made to operate on a thread within a specific process, but the thread specified is not in the process specified.";
			case 0x237: return "Page file quota was exceeded.";
			case 0x238: return "The Netlogon service cannot start because another Netlogon service running in the domain conflicts with the specified role.";
			case 0x239: return "The SAM database on a Windows Server is significantly out of synchronization with the copy on the Domain Controller. A complete synchronization is required.";
			case 0x23A: return "The NtCreateFile API failed. This error should never be returned to an application, it is a place holder for the Windows Lan Manager Redirector to use in its internal error mapping routines.";
			case 0x23B: return "{Privilege Failed} The I/O permissions for the process could not be changed.";
			case 0x23C: return "{Application Exit by CTRL+C} The application terminated as a result of a CTRL+C.";
			case 0x23D: return "{Missing System File} The required system file %hs is bad or missing.";
			case 0x23E: return "{Application Error} The exception %s (0x%08lx) occurred in the application at location 0x%08lx.";
			case 0x23F: return "{Application Error} The application was unable to start correctly (0x%lx). Click OK to close the application.";
			case 0x240: return "{Unable to Create Paging File} The creation of the paging file %hs failed (%lx). The requested size was %ld.";
			case 0x241: return "Windows cannot verify the digital signature for this file. A recent hardware or software change might have installed a file that is signed incorrectly or damaged, or that might be malicious software from an unknown source.";
			case 0x242: return "{No Paging File Specified} No paging file was specified in the system configuration.";
			case 0x243: return "{EXCEPTION} A real-mode application issued a floating-point instruction and floating-point hardware is not present.";
			case 0x244: return "An event pair synchronization operation was performed using the thread specific client/server event pair object, but no event pair object was associated with the thread.";
			case 0x245: return "A Windows Server has an incorrect configuration.";
			case 0x246: return "An illegal character was encountered. For a multi-byte character set this includes a lead byte without a succeeding trail byte. For the Unicode character set this includes the characters 0xFFFF and 0xFFFE.";
			case 0x247: return "The Unicode character is not defined in the Unicode character set installed on the system.";
			case 0x248: return "The paging file cannot be created on a floppy diskette.";
			case 0x249: return "The system BIOS failed to connect a system interrupt to the device or bus for which the device is connected.";
			case 0x24A: return "This operation is only allowed for the Primary Domain Controller of the domain.";
			case 0x24B: return "An attempt was made to acquire a mutant such that its maximum count would have been exceeded.";
			case 0x24C: return "A volume has been accessed for which a file system driver is required that has not yet been loaded.";
			case 0x24D: return "{Registry File Failure} The registry cannot load the hive (file): %hs or its log or alternate. It is corrupt, absent, or not writable.";
			case 0x24E: return "{Unexpected Failure in <a href=msdn.microsoft.com/en-us/library/windows/desktop/ms679295(v=vs.85).aspx><strong xmlns=www.w3.org/1999/xhtml>DebugActiveProcess</strong></a>} An unexpected failure occurred while processing a <strong>DebugActiveProcess</strong> API request. You may choose OK to terminate the process, or Cancel to ignore the error.";
			case 0x24F: return "{Fatal System Error} The %hs system process terminated unexpectedly with a status of 0x%08x (0x%08x 0x%08x). The system has been shut down.";
			case 0x250: return "{Data Not Accepted} The TDI client could not handle the data received during an indication.";
			case 0x251: return "NTVDM encountered a hard error.";
			case 0x252: return "{Cancel Timeout} The driver %hs failed to complete a cancelled I/O request in the allotted time.";
			case 0x253: return "{Reply Message Mismatch} An attempt was made to reply to an LPC message, but the thread specified by the client ID in the message was not waiting on that message.";
			case 0x254: return "{Delayed Write Failed} Windows was unable to save all the data for the file %hs. The data has been lost. This error may be caused by a failure of your computer hardware or network connection. Please try to save this file elsewhere.";
			case 0x255: return "The parameter(s) passed to the server in the client/server shared memory window were invalid. Too much data may have been put in the shared memory window.";
			case 0x256: return "The stream is not a tiny stream.";
			case 0x257: return "The request must be handled by the stack overflow code.";
			case 0x258: return "Internal OFS status codes indicating how an allocation operation is handled. Either it is retried after the containing onode is moved or the extent stream is converted to a large stream.";
			case 0x259: return "The attempt to find the object found an object matching by ID on the volume but it is out of the scope of the handle used for the operation.";
			case 0x25A: return "The bucket array must be grown. Retry transaction after doing so.";
			case 0x25B: return "The user/kernel marshalling buffer has overflowed.";
			case 0x25C: return "The supplied variant structure contains invalid data.";
			case 0x25D: return "The specified buffer contains ill-formed data.";
			case 0x25E: return "{Audit Failed} An attempt to generate a security audit failed.";
			case 0x25F: return "The timer resolution was not previously set by the current process.";
			case 0x260: return "There is insufficient account information to log you on.";
			case 0x261: return "{Invalid DLL Entrypoint} The dynamic link library %hs is not written correctly. The stack pointer has been left in an inconsistent state. The entrypoint should be declared as WINAPI or STDCALL. Select YES to fail the DLL load. Select NO to continue execution. Selecting NO may cause the application to operate incorrectly.";
			case 0x262: return "{Invalid Service Callback Entrypoint} The %hs service is not written correctly. The stack pointer has been left in an inconsistent state. The callback entrypoint should be declared as WINAPI or STDCALL. Selecting OK will cause the service to continue operation. However, the service process may operate incorrectly.";
			case 0x263: return "There is an IP address conflict with another system on the network.";
			case 0x264: return "There is an IP address conflict with another system on the network.";
			case 0x265: return "{Low On Registry Space} The system has reached the maximum size allowed for the system part of the registry. Additional storage requests will be ignored.";
			case 0x266: return "A callback return system service cannot be executed when no callback is active.";
			case 0x267: return "The password provided is too short to meet the policy of your user account. Please choose a longer password.";
			case 0x268: return "The policy of your user account does not allow you to change passwords too frequently. This is done to prevent users from changing back to a familiar, but potentially discovered, password. If you feel your password has been compromised then please contact your administrator immediately to have a new one assigned.";
			case 0x269: return "You have attempted to change your password to one that you have used in the past. The policy of your user account does not allow this. Please select a password that you have not previously used.";
			case 0x26A: return "The specified compression format is unsupported.";
			case 0x26B: return "The specified hardware profile configuration is invalid.";
			case 0x26C: return "The specified Plug and Play registry device path is invalid.";
			case 0x26D: return "The specified quota list is internally inconsistent with its descriptor.";
			case 0x26E: return "{Windows Evaluation Notification} The evaluation period for this installation of Windows has expired. This system will shutdown in 1 hour. To restore access to this installation of Windows, please upgrade this installation using a licensed distribution of this product.";
			case 0x26F: return "{Illegal System DLL Relocation} The system DLL %hs was relocated in memory. The application will not run properly. The relocation occurred because the DLL %hs occupied an address range reserved for Windows system DLLs. The vendor supplying the DLL should be contacted for a new DLL.";
			case 0x270: return "{DLL Initialization Failed} The application failed to initialize because the window station is shutting down.";
			case 0x271: return "The validation process needs to continue on to the next step.";
			case 0x272: return "There are no more matches for the current index enumeration.";
			case 0x273: return "The range could not be added to the range list because of a conflict.";
			case 0x274: return "The server process is running under a SID different than that required by client.";
			case 0x275: return "A group marked use for deny only cannot be enabled.";
			case 0x276: return "{EXCEPTION} Multiple floating point faults.";
			case 0x277: return "{EXCEPTION} Multiple floating point traps.";
			case 0x278: return "The requested interface is not supported.";
			case 0x279: return "{System Standby Failed} The driver %hs does not support standby mode. Updating this driver may allow the system to go to standby mode.";
			case 0x27A: return "The system file %1 has become corrupt and has been replaced.";
			case 0x27B: return "{Virtual Memory Minimum Too Low} Your system is low on virtual memory. Windows is increasing the size of your virtual memory paging file. During this process, memory requests for some applications may be denied. For more information, see Help.";
			case 0x27C: return "A device was removed so enumeration must be restarted.";
			case 0x27D: return "{Fatal System Error} The system image %s is not properly signed. The file has been replaced with the signed file. The system has been shut down.";
			case 0x27E: return "Device will not start without a reboot.";
			case 0x27F: return "There is not enough power to complete the requested operation.";
			case 0x280: return "ERROR_MULTIPLE_FAULT_VIOLATION";
			case 0x281: return "The system is in the process of shutting down.";
			case 0x282: return "An attempt to remove a processes DebugPort was made, but a port was not already associated with the process.";
			case 0x283: return "This version of Windows is not compatible with the behavior version of directory forest, domain or domain controller.";
			case 0x284: return "The specified range could not be found in the range list.";
			case 0x286: return "The driver was not loaded because the system is booting into safe mode.";
			case 0x287: return "The driver was not loaded because it failed its initialization call.";
			case 0x288: return "The \"%hs\" encountered an error while applying power or reading the device configuration. This may be caused by a failure of your hardware or by a poor connection.";
			case 0x289: return "The create operation failed because the name contained at least one mount point which resolves to a volume to which the specified device object is not attached.";
			case 0x28A: return "The device object parameter is either not a valid device object or is not attached to the volume specified by the file name.";
			case 0x28B: return "A Machine Check Error has occurred. Please check the system eventlog for additional information.";
			case 0x28C: return "There was error [%2] processing the driver database.";
			case 0x28D: return "System hive size has exceeded its limit.";
			case 0x28E: return "The driver could not be loaded because a previous version of the driver is still in memory.";
			case 0x28F: return "{Volume Shadow Copy Service} Please wait while the Volume Shadow Copy Service prepares volume %hs for hibernation.";
			case 0x290: return "The system has failed to hibernate (The error code is %hs). Hibernation will be disabled until the system is restarted.";
			case 0x291: return "The password provided is too long to meet the policy of your user account. Please choose a shorter password.";
			case 0x299: return "The requested operation could not be completed due to a file system limitation.";
			case 0x29C: return "An assertion failure has occurred.";
			case 0x29D: return "An error occurred in the ACPI subsystem.";
			case 0x29E: return "WOW Assertion Error.";
			case 0x29F: return "A device is missing in the system BIOS MPS table. This device will not be used. Please contact your system vendor for system BIOS update.";
			case 0x2A0: return "A translator failed to translate resources.";
			case 0x2A1: return "A IRQ translator failed to translate resources.";
			case 0x2A2: return "Driver %2 returned invalid ID for a child device (%3).";
			case 0x2A3: return "{Kernel Debugger Awakened} the system debugger was awakened by an interrupt.";
			case 0x2A4: return "{Handles Closed} Handles to objects have been automatically closed as a result of the requested operation.";
			case 0x2A5: return "{Too Much Information} The specified access control list (ACL) contained more information than was expected.";
			case 0x2A6: return "This warning level status indicates that the transaction state already exists for the registry sub-tree, but that a transaction commit was previously aborted. The commit has NOT been completed, but has not been rolled back either (so it may still be committed if desired).";
			case 0x2A7: return "{Media Changed} The media may have changed.";
			case 0x2A8: return "{GUID Substitution} During the translation of a global identifier (GUID) to a Windows security ID (SID), no administratively-defined GUID prefix was found. A substitute prefix was used, which will not compromise system security. However, this may provide a more restrictive access than intended.";
			case 0x2A9: return "The create operation stopped after reaching a symbolic link.";
			case 0x2AA: return "A long jump has been executed.";
			case 0x2AB: return "The Plug and Play query operation was not successful.";
			case 0x2AC: return "A frame consolidation has been executed.";
			case 0x2AD: return "{Registry Hive Recovered} Registry hive (file): %hs was corrupted and it has been recovered. Some data might have been lost.";
			case 0x2AE: return "The application is attempting to run executable code from the module %hs. This may be insecure. An alternative, %hs, is available. Should the application use the secure module %hs?";
			case 0x2AF: return "The application is loading executable code from the module %hs. This is secure, but may be incompatible with previous releases of the operating system. An alternative, %hs, is available. Should the application use the secure module %hs?";
			case 0x2B0: return "Debugger did not handle the exception.";
			case 0x2B1: return "Debugger will reply later.";
			case 0x2B2: return "Debugger cannot provide handle.";
			case 0x2B3: return "Debugger terminated thread.";
			case 0x2B4: return "Debugger terminated process.";
			case 0x2B5: return "Debugger got control C.";
			case 0x2B6: return "Debugger printed exception on control C.";
			case 0x2B7: return "Debugger received RIP exception.";
			case 0x2B8: return "Debugger received control break.";
			case 0x2B9: return "Debugger command communication exception.";
			case 0x2BA: return "{Object Exists} An attempt was made to create an object and the object name already existed.";
			case 0x2BB: return "{Thread Suspended} A thread termination occurred while the thread was suspended. The thread was resumed, and termination proceeded.";
			case 0x2BC: return "{Image Relocated} An image file could not be mapped at the address specified in the image file. Local fixups must be performed on this image.";
			case 0x2BD: return "This informational level status indicates that a specified registry sub-tree transaction state did not yet exist and had to be created.";
			case 0x2BE: return "{Segment Load} A virtual DOS machine (VDM) is loading, unloading, or moving an MS-DOS or Win16 program segment image. An exception is raised so a debugger can load, unload or track symbols and breakpoints within these 16-bit segments.";
			case 0x2BF: return "{Invalid Current Directory} The process cannot switch to the startup current directory %hs. Select OK to set current directory to %hs, or select CANCEL to exit.";
			case 0x2C0: return "{Redundant Read} To satisfy a read request, the NT fault-tolerant file system successfully read the requested data from a redundant copy. This was done because the file system encountered a failure on a member of the fault-tolerant volume, but was unable to reassign the failing area of the device.";
			case 0x2C1: return "{Redundant Write} To satisfy a write request, the NT fault-tolerant file system successfully wrote a redundant copy of the information. This was done because the file system encountered a failure on a member of the fault-tolerant volume, but was not able to reassign the failing area of the device.";
			case 0x2C2: return "{Machine Type Mismatch} The image file %hs is valid, but is for a machine type other than the current machine. Select OK to continue, or CANCEL to fail the DLL load.";
			case 0x2C3: return "{Partial Data Received} The network transport returned partial data to its client. The remaining data will be sent later.";
			case 0x2C4: return "{Expedited Data Received} The network transport returned data to its client that was marked as expedited by the remote system.";
			case 0x2C5: return "{Partial Expedited Data Received} The network transport returned partial data to its client and this data was marked as expedited by the remote system. The remaining data will be sent later.";
			case 0x2C6: return "{TDI Event Done} The TDI indication has completed successfully.";
			case 0x2C7: return "{TDI Event Pending} The TDI indication has entered the pending state.";
			case 0x2C8: return "Checking file system on %wZ.";
			case 0x2C9: return "{Fatal Application Exit} %hs.";
			case 0x2CA: return "The specified registry key is referenced by a predefined handle.";
			case 0x2CB: return "{Page Unlocked} The page protection of a locked page was changed to 'No Access' and the page was unlocked from memory and from the process.";
			case 0x2CC: return "%hs";
			case 0x2CD: return "{Page Locked} One of the pages to lock was already locked.";
			case 0x2CE: return "Application popup: %1 : %2";
			case 0x2CF: return "ERROR_ALREADY_WIN32";
			case 0x2D0: return "{Machine Type Mismatch} The image file %hs is valid, but is for a machine type other than the current machine.";
			case 0x2D1: return "A yield execution was performed and no thread was available to run.";
			case 0x2D2: return "The resumable flag to a timer API was ignored.";
			case 0x2D3: return "The arbiter has deferred arbitration of these resources to its parent.";
			case 0x2D4: return "The inserted CardBus device cannot be started because of a configuration error on \"%hs\".";
			case 0x2D5: return "The CPUs in this multiprocessor system are not all the same revision level. To use all processors the operating system restricts itself to the features of the least capable processor in the system. Should problems occur with this system, contact the CPU manufacturer to see if this mix of processors is supported.";
			case 0x2D6: return "The system was put into hibernation.";
			case 0x2D7: return "The system was resumed from hibernation.";
			case 0x2D8: return "Windows has detected that the system firmware (BIOS) was updated [previous firmware date = %2, current firmware date %3].";
			case 0x2D9: return "A device driver is leaking locked I/O pages causing system degradation. The system has automatically enabled tracking code in order to try and catch the culprit.";
			case 0x2DA: return "The system has awoken.";
			case 0x2DB: return "ERROR_WAIT_1";
			case 0x2DC: return "ERROR_WAIT_2";
			case 0x2DD: return "ERROR_WAIT_3";
			case 0x2DE: return "ERROR_WAIT_63";
			case 0x2DF: return "ERROR_ABANDONED_WAIT_0";
			case 0x2E0: return "ERROR_ABANDONED_WAIT_63";
			case 0x2E1: return "ERROR_USER_APC";
			case 0x2E2: return "ERROR_KERNEL_APC";
			case 0x2E3: return "ERROR_ALERTED";
			case 0x2E4: return "The requested operation requires elevation.";
			case 0x2E5: return "A reparse should be performed by the Object Manager since the name of the file resulted in a symbolic link.";
			case 0x2E6: return "An open/create operation completed while an oplock break is underway.";
			case 0x2E7: return "A new volume has been mounted by a file system.";
			case 0x2E8: return "This success level status indicates that the transaction state already exists for the registry sub-tree, but that a transaction commit was previously aborted. The commit has now been completed.";
			case 0x2E9: return "This indicates that a notify change request has been completed due to closing the handle which made the notify change request.";
			case 0x2EA: return "{Connect Failure on Primary Transport} An attempt was made to connect to the remote server %hs on the primary transport, but the connection failed. The computer WAS able to connect on a secondary transport.";
			case 0x2EB: return "Page fault was a transition fault.";
			case 0x2EC: return "Page fault was a demand zero fault.";
			case 0x2ED: return "Page fault was a demand zero fault.";
			case 0x2EE: return "Page fault was a demand zero fault.";
			case 0x2EF: return "Page fault was satisfied by reading from a secondary storage device.";
			case 0x2F0: return "Cached page was locked during operation.";
			case 0x2F1: return "Crash dump exists in paging file.";
			case 0x2F2: return "Specified buffer contains all zeros.";
			case 0x2F3: return "A reparse should be performed by the Object Manager since the name of the file resulted in a symbolic link.";
			case 0x2F4: return "The device has succeeded a query-stop and its resource requirements have changed.";
			case 0x2F5: return "The translator has translated these resources into the global space and no further translations should be performed.";
			case 0x2F6: return "A process being terminated has no threads to terminate.";
			case 0x2F7: return "The specified process is not part of a job.";
			case 0x2F8: return "The specified process is part of a job.";
			case 0x2F9: return "{Volume Shadow Copy Service} The system is now ready for hibernation.";
			case 0x2FA: return "A file system or file system filter driver has successfully completed an FsFilter operation.";
			case 0x2FB: return "The specified interrupt vector was already connected.";
			case 0x2FC: return "The specified interrupt vector is still connected.";
			case 0x2FD: return "An operation is blocked waiting for an oplock.";
			case 0x2FE: return "Debugger handled exception.";
			case 0x2FF: return "Debugger continued.";
			case 0x300: return "An exception occurred in a user mode callback and the kernel callback frame should be removed.";
			case 0x301: return "Compression is disabled for this volume.";
			case 0x302: return "The data provider cannot fetch backwards through a result set.";
			case 0x303: return "The data provider cannot scroll backwards through a result set.";
			case 0x304: return "The data provider requires that previously fetched data is released before asking for more data.";
			case 0x305: return "The data provider was not able to interpret the flags set for a column binding in an accessor.";
			case 0x306: return "One or more errors occurred while processing the request.";
			case 0x307: return "The implementation is not capable of performing the request.";
			case 0x308: return "The client of a component requested an operation which is not valid given the state of the component instance.";
			case 0x309: return "A version number could not be parsed.";
			case 0x30A: return "The iterator's start position is invalid.";
			case 0x30B: return "The hardware has reported an uncorrectable memory error.";
			case 0x30C: return "The attempted operation required self healing to be enabled.";
			case 0x30D: return "The Desktop heap encountered an error while allocating session memory. There is more information in the system event log.";
			case 0x30E: return "The system power state is transitioning from %2 to %3.";
			case 0x30F: return "The system power state is transitioning from %2 to %3 but could enter %4.";
			case 0x310: return "A thread is getting dispatched with MCA EXCEPTION because of MCA.";
			case 0x311: return "Access to %1 is monitored by policy rule %2.";
			case 0x312: return "Access to %1 has been restricted by your Administrator by policy rule %2.";
			case 0x313: return "A valid hibernation file has been invalidated and should be abandoned.";
			case 0x314: return "{Delayed Write Failed} Windows was unable to save all the data for the file %hs; the data has been lost. This error may be caused by network connectivity issues. Please try to save this file elsewhere.";
			case 0x315: return "{Delayed Write Failed} Windows was unable to save all the data for the file %hs; the data has been lost. This error was returned by the server on which the file exists. Please try to save this file elsewhere.";
			case 0x316: return "{Delayed Write Failed} Windows was unable to save all the data for the file %hs; the data has been lost. This error may be caused if the device has been removed or the media is write-protected.";
			case 0x317: return "The resources required for this device conflict with the MCFG table.";
			case 0x318: return "The volume repair could not be performed while it is online. Please schedule to take the volume offline so that it can be repaired.";
			case 0x319: return "The volume repair was not successful.";
			case 0x31A: return "One of the volume corruption logs is full. Further corruptions that may be detected won't be logged.";
			case 0x31B: return "One of the volume corruption logs is internally corrupted and needs to be recreated. The volume may contain undetected corruptions and must be scanned.";
			case 0x31C: return "One of the volume corruption logs is unavailable for being operated on.";
			case 0x31D: return "One of the volume corruption logs was deleted while still having corruption records in them. The volume contains detected corruptions and must be scanned.";
			case 0x31E: return "One of the volume corruption logs was cleared by chkdsk and no longer contains real corruptions.";
			case 0x31F: return "Orphaned files exist on the volume but could not be recovered because no more new names could be created in the recovery directory. Files must be moved from the recovery directory.";
			case 0x320: return "The oplock that was associated with this handle is now associated with a different handle.";
			case 0x321: return "An oplock of the requested level cannot be granted. An oplock of a lower level may be available.";
			case 0x322: return "The operation did not complete successfully because it would cause an oplock to be broken. The caller has requested that existing oplocks not be broken.";
			case 0x323: return "The handle with which this oplock was associated has been closed. The oplock is now broken.";
			case 0x324: return "The specified access control entry (ACE) does not contain a condition.";
			case 0x325: return "The specified access control entry (ACE) contains an invalid condition.";
			case 0x326: return "Access to the specified file handle has been revoked.";
			case 0x327: return "An image file was mapped at a different address from the one specified in the image file but fixups will still be automatically performed on the image.";
			case 0x3E2: return "Access to the extended attribute was denied.";
			case 0x3E3: return "The I/O operation has been aborted because of either a thread exit or an application request.";
			case 0x3E4: return "Overlapped I/O event is not in a signaled state.";
			case 0x3E5: return "Overlapped I/O operation is in progress.";
			case 0x3E6: return "Invalid access to memory location.";
			case 0x3E7: return "Error performing inpage operation.";
			case 0x3E9: return "Recursion too deep; the stack overflowed.";
			case 0x3EA: return "The window cannot act on the sent message.";
			case 0x3EB: return "Cannot complete this function.";
			case 0x3EC: return "Invalid flags.";
			case 0x3ED: return "The volume does not contain a recognized file system. Please make sure that all required file system drivers are loaded and that the volume is not corrupted.";
			case 0x3EE: return "The volume for a file has been externally altered so that the opened file is no longer valid.";
			case 0x3EF: return "The requested operation cannot be performed in full-screen mode.";
			case 0x3F0: return "An attempt was made to reference a token that does not exist.";
			case 0x3F1: return "The configuration registry database is corrupt.";
			case 0x3F2: return "The configuration registry key is invalid.";
			case 0x3F3: return "The configuration registry key could not be opened.";
			case 0x3F4: return "The configuration registry key could not be read.";
			case 0x3F5: return "The configuration registry key could not be written.";
			case 0x3F6: return "One of the files in the registry database had to be recovered by use of a log or alternate copy. The recovery was successful.";
			case 0x3F7: return "The registry is corrupted. The structure of one of the files containing registry data is corrupted, or the system's memory image of the file is corrupted, or the file could not be recovered because the alternate copy or log was absent or corrupted.";
			case 0x3F8: return "An I/O operation initiated by the registry failed unrecoverably. The registry could not read in, or write out, or flush, one of the files that contain the system's image of the registry.";
			case 0x3F9: return "The system has attempted to load or restore a file into the registry, but the specified file is not in a registry file format.";
			case 0x3FA: return "Illegal operation attempted on a registry key that has been marked for deletion.";
			case 0x3FB: return "System could not allocate the required space in a registry log.";
			case 0x3FC: return "Cannot create a symbolic link in a registry key that already has subkeys or values.";
			case 0x3FD: return "Cannot create a stable subkey under a volatile parent key.";
			case 0x3FE: return "A notify change request is being completed and the information is not being returned in the caller's buffer. The caller now needs to enumerate the files to find the changes.";
			case 0x41B: return "A stop control has been sent to a service that other running services are dependent on.";
			case 0x41C: return "The requested control is not valid for this service.";
			case 0x41D: return "The service did not respond to the start or control request in a timely fashion.";
			case 0x41E: return "A thread could not be created for the service.";
			case 0x41F: return "The service database is locked.";
			case 0x420: return "An instance of the service is already running.";
			case 0x421: return "The account name is invalid or does not exist, or the password is invalid for the account name specified.";
			case 0x422: return "The service cannot be started, either because it is disabled or because it has no enabled devices associated with it.";
			case 0x423: return "Circular service dependency was specified.";
			case 0x424: return "The specified service does not exist as an installed service.";
			case 0x425: return "The service cannot accept control messages at this time.";
			case 0x426: return "The service has not been started.";
			case 0x427: return "The service process could not connect to the service controller.";
			case 0x428: return "An exception occurred in the service when handling the control request.";
			case 0x429: return "The database specified does not exist.";
			case 0x42A: return "The service has returned a service-specific error code.";
			case 0x42B: return "The process terminated unexpectedly.";
			case 0x42C: return "The dependency service or group failed to start.";
			case 0x42D: return "The service did not start due to a logon failure.";
			case 0x42E: return "After starting, the service hung in a start-pending state.";
			case 0x42F: return "The specified service database lock is invalid.";
			case 0x430: return "The specified service has been marked for deletion.";
			case 0x431: return "The specified service already exists.";
			case 0x432: return "The system is currently running with the last-known-good configuration.";
			case 0x433: return "The dependency service does not exist or has been marked for deletion.";
			case 0x434: return "The current boot has already been accepted for use as the last-known-good control set.";
			case 0x435: return "No attempts to start the service have been made since the last boot.";
			case 0x436: return "The name is already in use as either a service name or a service display name.";
			case 0x437: return "The account specified for this service is different from the account specified for other services running in the same process.";
			case 0x438: return "Failure actions can only be set for Win32 services, not for drivers.";
			case 0x439: return "This service runs in the same process as the service control manager. Therefore, the service control manager cannot take action if this service's process terminates unexpectedly.";
			case 0x43A: return "No recovery program has been configured for this service.";
			case 0x43B: return "The executable program that this service is configured to run in does not implement the service.";
			case 0x43C: return "This service cannot be started in Safe Mode.";
			case 0x44C: return "The physical end of the tape has been reached.";
			case 0x44D: return "A tape access reached a filemark.";
			case 0x44E: return "The beginning of the tape or a partition was encountered.";
			case 0x44F: return "A tape access reached the end of a set of files.";
			case 0x450: return "No more data is on the tape.";
			case 0x451: return "Tape could not be partitioned.";
			case 0x452: return "When accessing a new tape of a multivolume partition, the current block size is incorrect.";
			case 0x453: return "Tape partition information could not be found when loading a tape.";
			case 0x454: return "Unable to lock the media eject mechanism.";
			case 0x455: return "Unable to unload the media.";
			case 0x456: return "The media in the drive may have changed.";
			case 0x457: return "The I/O bus was reset.";
			case 0x458: return "No media in drive.";
			case 0x459: return "No mapping for the Unicode character exists in the target multi-byte code page.";
			case 0x45A: return "A dynamic link library (DLL) initialization routine failed.";
			case 0x45B: return "A system shutdown is in progress.";
			case 0x45C: return "Unable to abort the system shutdown because no shutdown was in progress.";
			case 0x45D: return "The request could not be performed because of an I/O device error.";
			case 0x45E: return "No serial device was successfully initialized. The serial driver will unload.";
			case 0x45F: return "Unable to open a device that was sharing an interrupt request (IRQ) with other devices. At least one other device that uses that IRQ was already opened.";
			case 0x460: return "A serial I/O operation was completed by another write to the serial port. The IOCTL_SERIAL_XOFF_COUNTER reached zero.)";
			case 0x461: return "A serial I/O operation completed because the timeout period expired. The IOCTL_SERIAL_XOFF_COUNTER did not reach zero.)";
			case 0x462: return "No ID address mark was found on the floppy disk.";
			case 0x463: return "Mismatch between the floppy disk sector ID field and the floppy disk controller track address.";
			case 0x464: return "The floppy disk controller reported an error that is not recognized by the floppy disk driver.";
			case 0x465: return "The floppy disk controller returned inconsistent results in its registers.";
			case 0x466: return "While accessing the hard disk, a recalibrate operation failed, even after retries.";
			case 0x467: return "While accessing the hard disk, a disk operation failed even after retries.";
			case 0x468: return "While accessing the hard disk, a disk controller reset was needed, but even that failed.";
			case 0x469: return "Physical end of tape encountered.";
			case 0x46A: return "Not enough server storage is available to process this command.";
			case 0x46B: return "A potential deadlock condition has been detected.";
			case 0x46C: return "The base address or the file offset specified does not have the proper alignment.";
			case 0x474: return "An attempt to change the system power state was vetoed by another application or driver.";
			case 0x475: return "The system BIOS failed an attempt to change the system power state.";
			case 0x476: return "An attempt was made to create more links on a file than the file system supports.";
			case 0x47E: return "The specified program requires a newer version of Windows.";
			case 0x47F: return "The specified program is not a Windows or MS-DOS program.";
			case 0x480: return "Cannot start more than one instance of the specified program.";
			case 0x481: return "The specified program was written for an earlier version of Windows.";
			case 0x482: return "One of the library files needed to run this application is damaged.";
			case 0x483: return "No application is associated with the specified file for this operation.";
			case 0x484: return "An error occurred in sending the command to the application.";
			case 0x485: return "One of the library files needed to run this application cannot be found.";
			case 0x486: return "The current process has used all of its system allowance of handles for Window Manager objects.";
			case 0x487: return "The message can be used only with synchronous operations.";
			case 0x488: return "The indicated source element has no media.";
			case 0x489: return "The indicated destination element already contains media.";
			case 0x48A: return "The indicated element does not exist.";
			case 0x48B: return "The indicated element is part of a magazine that is not present.";
			case 0x48C: return "The indicated device requires reinitialization due to hardware errors.";
			case 0x48D: return "The device has indicated that cleaning is required before further operations are attempted.";
			case 0x48E: return "The device has indicated that its door is open.";
			case 0x48F: return "The device is not connected.";
			case 0x490: return "Element not found.";
			case 0x491: return "There was no match for the specified key in the index.";
			case 0x492: return "The property set specified does not exist on the object.";
			case 0x493: return "The point passed to GetMouseMovePoints is not in the buffer.";
			case 0x494: return "The tracking (workstation) service is not running.";
			case 0x495: return "The Volume ID could not be found.";
			case 0x497: return "Unable to remove the file to be replaced.";
			case 0x498: return "Unable to move the replacement file to the file to be replaced. The file to be replaced has retained its original name.";
			case 0x499: return "Unable to move the replacement file to the file to be replaced. The file to be replaced has been renamed using the backup name.";
			case 0x49A: return "The volume change journal is being deleted.";
			case 0x49B: return "The volume change journal is not active.";
			case 0x49C: return "A file was found, but it may not be the correct file.";
			case 0x49D: return "The journal entry has been deleted from the journal.";
			case 0x4A6: return "A system shutdown has already been scheduled.";
			case 0x4A7: return "The system shutdown cannot be initiated because there are other users logged on to the computer.";
			case 0x4B0: return "The specified device name is invalid.";
			case 0x4B1: return "The device is not currently connected but it is a remembered connection.";
			case 0x4B2: return "The local device name has a remembered connection to another network resource.";
			case 0x4B3: return "The network path was either typed incorrectly, does not exist, or the network provider is not currently available. Please try retyping the path or contact your network administrator.";
			case 0x4B4: return "The specified network provider name is invalid.";
			case 0x4B5: return "Unable to open the network connection profile.";
			case 0x4B6: return "The network connection profile is corrupted.";
			case 0x4B7: return "Cannot enumerate a noncontainer.";
			case 0x4B8: return "An extended error has occurred.";
			case 0x4B9: return "The format of the specified group name is invalid.";
			case 0x4BA: return "The format of the specified computer name is invalid.";
			case 0x4BB: return "The format of the specified event name is invalid.";
			case 0x4BC: return "The format of the specified domain name is invalid.";
			case 0x4BD: return "The format of the specified service name is invalid.";
			case 0x4BE: return "The format of the specified network name is invalid.";
			case 0x4BF: return "The format of the specified share name is invalid.";
			case 0x4C0: return "The format of the specified password is invalid.";
			case 0x4C1: return "The format of the specified message name is invalid.";
			case 0x4C2: return "The format of the specified message destination is invalid.";
			case 0x4C3: return "Multiple connections to a server or shared resource by the same user, using more than one user name, are not allowed. Disconnect all previous connections to the server or shared resource and try again.";
			case 0x4C4: return "An attempt was made to establish a session to a network server, but there are already too many sessions established to that server.";
			case 0x4C5: return "The workgroup or domain name is already in use by another computer on the network.";
			case 0x4C6: return "The network is not present or not started.";
			case 0x4C7: return "The operation was canceled by the user.";
			case 0x4C8: return "The requested operation cannot be performed on a file with a user-mapped section open.";
			case 0x4C9: return "The remote computer refused the network connection.";
			case 0x4CA: return "The network connection was gracefully closed.";
			case 0x4CB: return "The network transport endpoint already has an address associated with it.";
			case 0x4CC: return "An address has not yet been associated with the network endpoint.";
			case 0x4CD: return "An operation was attempted on a nonexistent network connection.";
			case 0x4CE: return "An invalid operation was attempted on an active network connection.";
			case 0x4CF: return "The network location cannot be reached. For information about network troubleshooting, see Windows Help.";
			case 0x4D0: return "The network location cannot be reached. For information about network troubleshooting, see Windows Help.";
			case 0x4D1: return "The network location cannot be reached. For information about network troubleshooting, see Windows Help.";
			case 0x4D2: return "No service is operating at the destination network endpoint on the remote system.";
			case 0x4D3: return "The request was aborted.";
			case 0x4D4: return "The network connection was aborted by the local system.";
			case 0x4D5: return "The operation could not be completed. A retry should be performed.";
			case 0x4D6: return "A connection to the server could not be made because the limit on the number of concurrent connections for this account has been reached.";
			case 0x4D7: return "Attempting to log in during an unauthorized time of day for this account.";
			case 0x4D8: return "The account is not authorized to log in from this station.";
			case 0x4D9: return "The network address could not be used for the operation requested.";
			case 0x4DA: return "The service is already registered.";
			case 0x4DB: return "The specified service does not exist.";
			case 0x4DC: return "The operation being requested was not performed because the user has not been authenticated.";
			case 0x4DD: return "The operation being requested was not performed because the user has not logged on to the network. The specified service does not exist.";
			case 0x4DE: return "Continue with work in progress.";
			case 0x4DF: return "An attempt was made to perform an initialization operation when initialization has already been completed.";
			case 0x4E0: return "No more local devices.";
			case 0x4E1: return "The specified site does not exist.";
			case 0x4E2: return "A domain controller with the specified name already exists.";
			case 0x4E3: return "This operation is supported only when you are connected to the server.";
			case 0x4E4: return "The group policy framework should call the extension even if there are no changes.";
			case 0x4E5: return "The specified user does not have a valid profile.";
			case 0x4E6: return "This operation is not supported on a computer running Windows Server 2003 for Small Business Server.";
			case 0x4E7: return "The server machine is shutting down.";
			case 0x4E8: return "The remote system is not available. For information about network troubleshooting, see Windows Help.";
			case 0x4E9: return "The security identifier provided is not from an account domain.";
			case 0x4EA: return "The security identifier provided does not have a domain component.";
			case 0x4EB: return "AppHelp dialog canceled thus preventing the application from starting.";
			case 0x4EC: return "This program is blocked by group policy. For more information, contact your system administrator.";
			case 0x4ED: return "A program attempt to use an invalid register value. Normally caused by an uninitialized register. This error is Itanium specific.";
			case 0x4EE: return "The share is currently offline or does not exist.";
			case 0x4EF: return "The Kerberos protocol encountered an error while validating the KDC certificate during smartcard logon. There is more information in the system event log.";
			case 0x4F0: return "The Kerberos protocol encountered an error while attempting to utilize the smartcard subsystem.";
			case 0x4F1: return "The system cannot contact a domain controller to service the authentication request. Please try again later.";
			case 0x4F7: return "The machine is locked and cannot be shut down without the force option.";
			case 0x4F9: return "An application-defined callback gave invalid data when called.";
			case 0x4FA: return "The group policy framework should call the extension in the synchronous foreground policy refresh.";
			case 0x4FB: return "This driver has been blocked from loading.";
			case 0x4FC: return "A dynamic link library (DLL) referenced a module that was neither a DLL nor the process's executable image.";
			case 0x4FD: return "Windows cannot open this program since it has been disabled.";
			case 0x4FE: return "Windows cannot open this program because the license enforcement system has been tampered with or become corrupted.";
			case 0x4FF: return "A transaction recover failed.";
			case 0x500: return "The current thread has already been converted to a fiber.";
			case 0x501: return "The current thread has already been converted from a fiber.";
			case 0x502: return "The system detected an overrun of a stack-based buffer in this application. This overrun could potentially allow a malicious user to gain control of this application.";
			case 0x503: return "Data present in one of the parameters is more than the function can operate on.";
			case 0x504: return "An attempt to do an operation on a debug object failed because the object is in the process of being deleted.";
			case 0x505: return "An attempt to delay-load a .dll or get a function address in a delay-loaded .dll failed.";
			case 0x506: return "%1 is a 16-bit application. You do not have permissions to execute 16-bit applications. Check your permissions with your system administrator.";
			case 0x507: return "Insufficient information exists to identify the cause of failure.";
			case 0x508: return "The parameter passed to a C runtime function is incorrect.";
			case 0x509: return "The operation occurred beyond the valid data length of the file.";
			case 0x50A: return "The service start failed since one or more services in the same process have an incompatible service SID type  setting. A service with restricted service SID type can only coexist in the same process with other services  with a restricted SID type. If the service SID type for this service was just configured, the hosting process  must be restarted in order to start this service.";
			case 0x50B: return "The process hosting the driver for this device has been terminated.";
			case 0x50C: return "An operation attempted to exceed an implementation-defined limit.";
			case 0x50D: return "Either the target process, or the target thread's containing process, is a protected process.";
			case 0x50E: return "The service notification client is lagging too far behind the current state of services in the machine.";
			case 0x50F: return "The requested file operation failed because the storage quota was exceeded. To free up disk space, move files to a different location or delete unnecessary files. For more information, contact your system administrator.";
			case 0x510: return "The requested file operation failed because the storage policy blocks that type of file. For more information, contact your system administrator.";
			case 0x511: return "A privilege that the service requires to function properly does not exist in the service account configuration. You may use the Services Microsoft Management Console (MMC) snap-in (services.msc) and the Local Security Settings MMC snap-in (secpol.msc) to view the service configuration and the account configuration.";
			case 0x512: return "A thread involved in this operation appears to be unresponsive.";
			case 0x513: return "Indicates a particular Security ID may not be assigned as the label of an object.";
		}
		return "Description not found.";
	}
}