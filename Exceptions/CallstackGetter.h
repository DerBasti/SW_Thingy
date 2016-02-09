
#ifndef __CALLSTACK_GETTER__
#define __CALLSTACK_GETTER__
#include <type_traits>

#include <ShlObj.h>
//SYM_*
#include <DbgHelp.h>

#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

//VA_ARGS
#include <stdlib.h>
#include <stdarg.h>
//ToolSnapshots
#include <tlhelp32.h>

#pragma comment(lib, "DbgHelp.lib")

#include "D:\Programmieren\QuickInfos\VarsToString"

class CallStack {
	private:
		CallStack() {}
		~CallStack() {}
		static void initAllDLLs(HANDLE process) {
			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(process));
			MODULEENTRY32 entry;
			entry.dwSize = sizeof(MODULEENTRY32);
			if (Module32First(snapshot, &entry) == TRUE) {
				do {
					SymLoadModuleExW(process, 0, entry.szExePath, entry.szModule, (DWORD64)entry.modBaseAddr, entry.modBaseSize, nullptr, 0);
				} while (Module32Next(snapshot, &entry) == TRUE);
			}
		}
		template<class _Ty> static void _getCallstack(std::basic_string<_Ty>& log) {
			log.clear();
			HMODULE dbgHelp = LoadLibraryA("%windir%\\system32\\dbghelp.dll");
			HANDLE currentProcess = GetCurrentProcess();
			std::string symInit = "SRV*%windir%\\system32\\symsrv.dll*http://msdl.microsoft.com/download/symbols";
			if (!SymInitialize(currentProcess, nullptr, false)) {
				return;
			}
			initAllDLLs(currentProcess);
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
			char buf[4096] = { 0x00 };
			for (unsigned int frameNum = 0;; ++frameNum) {
				if (!StackWalk64(imageType, currentProcess, currentThread, &stackFrame, &context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
					break;

				//Loop?
				if (stackFrame.AddrPC.Offset == stackFrame.AddrReturn.Offset)
					break;

				if (stackFrame.AddrPC.Offset != 0) {
					std::string symbolName = "";
					if (SymGetSymFromAddr64(currentProcess, stackFrame.AddrPC.Offset, &offsetFromSymbol64, symbol)) {
						UnDecorateSymbolName(symbol->Name, buf, 1024, UNDNAME_COMPLETE);
						symbolName = std::string(buf);
					}
					if (SymGetLineFromAddr64(currentProcess, stackFrame.AddrPC.Offset, &offsetFromLine, &line)) {
						std::string fileName = std::string(line.FileName);
						bool fullPath = false;
						if (!fullPath) {
							fileName = fileName.substr(fileName.find_last_of("\\") + 1);
						}
						sprintf_s(buf, "%s[%s(%i)]\n", log.c_str(), symbolName.c_str(), line.LineNumber);
						std::string tmpResult = std::string(buf);
						log = std::basic_string<_Ty>(tmpResult.begin(), tmpResult.end());
					}
					else {
						log += '\n';
					}
				}
			}
			free(symbol);
			FreeLibrary(dbgHelp);

			//Strip common frame + this one
			for (unsigned int i = 0; i < 6; i++) {
				log = log.substr(0, log.find_last_of(0x0a));
			}
			log = log.substr(log.find_first_of(0x0A) + 1);
		}
	public:
		template<class _Ty, class = std::enable_if<std::false_type>::type> static void get(std::basic_string<_Ty>& log){}
		static void get(std::string& log) {
			return _getCallstack<char>(log);
		}
		
		static void get(std::wstring& log) {
			return _getCallstack<wchar_t>(log);
		}

		static std::string getAsAscii() {
			std::string res;
			_getCallstack<char>(res);
			return res;
		}

		static std::wstring getAsUnicode() {
			std::wstring res;
			_getCallstack<wchar_t>(res);
			return res;
		}
};

#endif //__CALLSTACK_GETTER__