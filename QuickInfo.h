#pragma once

#include "Trackable.hpp"
#include "LinkedList.h"
#include "LinkedList.cpp"

//SOURCE: http://www.codeproject.com/Articles/11132/Walking-the-callstack
//#include "StackWalker.h"

#include "VarsToString"

#include "..\ConfigReader\Config.h"
#include "..\CMyFile\MyFile.h"
#include "..\Exceptions\TraceableException.h"
#include <iostream>
#include <vector>
#include <functional>
#include <xutility>
#include <ShlObj.h>
#include <DbgHelp.h>
#include <Psapi.h>
#include <tlhelp32.h>


#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "DbgHelp.lib")

class Integer {
	private:
		int num;
	public:
		Integer() { this->num = 0; }
		Integer(int number) { this->num = number; }
		~Integer() {};
		Integer& operator=(const long num) {
			this->num = num;
			return (*this);
		}
		std::string toString() const {
			char buf[sizeof(int)* 8] = { 0x00 };
			::sprintf_s(buf, "%i", this->num);
			return std::string(buf);
		}
		static int fromString(const char* str) {
			if (!str)
				return 0;
			return atoi(str);
		}
};

namespace QuickInfo {

#ifndef WINDOW_SEARCH_TYPES
	#define WINDOW_SEARCH_TYPES
	#define WINDOW_SHARP_SEARCH 0x10000000
	#define WINDOW_FUZZY_SEARCH 0x20000000
	#define WINDOW_THREAD_SEARCH 0x40000000
	#define WINDOW_HANDLE_SEARCH 0x80000000
#endif

#if UNICODE
	typedef std::wstring tstring;
#else
	typedef std::string tstring;
#endif

	struct QUICKPROCESSINFOS {
		std::wstring path;
		std::wstring handleName;
		DWORD threadId;
		std::vector<std::wstring> dlls;
	};

	enum EOperatingSystems {
		OS_Windows_31 = 0,
		OS_Windows_95,
		OS_Windows_98,
		OS_Windows_ME,
		OS_Windows_NT,
		OS_Windows_2000,
		OS_Windows_XP,
		OS_Windows_Vista,
		OS_Windows_7
	};

	enum DirectXFunctionOffsets {
		D3D9_QueryInterface,
		D3D9_AddRef,
		D3D9_Release,

		/*** IDirect3DDevice9 methods ***/
		D3D9_TestCooperativeLevel,
		D3D9_GetAvailableTextureMem,
		D3D9_EvictManagedResources,
		D3D9_GetDirect3D,
		D3D9_GetDeviceCaps,
		D3D9_GetDisplayMode,
		D3D9_GetCreationParameters,
		D3D9_SetCursorProperties,
		D3D9_SetCursorPosition,
		D3D9_ShowCursor,
		D3D9_CreateAdditionalSwapChain,
		D3D9_GetSwapChain,
		D3D9_GetNumberOfSwapChains,
		D3D9_Reset,
		D3D9_Present,
		D3D9_GetBackBuffer,
		D3D9_GetRasterStatus,
		D3D9_SetDialogBoxMode,
		D3D9_SetGammaRamp,
		D3D9_GetGammaRamp,
		D3D9_CreateTexture,
		D3D9_CreateVolumeTexture,
		D3D9_CreateCubeTexture,
		D3D9_CreateVertexBuffer,
		D3D9_CreateIndexBuffer,
		D3D9_CreateRenderTarget,
		D3D9_CreateDepthStencilSurface,
		D3D9_UpdateSurface,
		D3D9_UpdateTexture,
		D3D9_GetRenderTargetData,
		D3D9_GetFrontBufferData,
		D3D9_StretchRect,
		D3D9_ColorFill,
		D3D9_CreateOffscreenPlainSurface,
		D3D9_SetRenderTarget,
		D3D9_GetRenderTarget,
		D3D9_SetDepthStencilSurface,
		D3D9_GetDepthStencilSurface,
		D3D9_BeginScene,
		D3D9_EndScene,
		D3D9_Clear,
		D3D9_SetTransform,
		D3D9_GetTransform,
		D3D9_MultiplyTransform,
		D3D9_SetViewport,
		D3D9_GetViewport,
		D3D9_SetMaterial,
		D3D9_GetMaterial,
		D3D9_SetLight,
		D3D9_GetLight,
		D3D9_LightEnable,
		D3D9_GetLightEnable,
		D3D9_SetClipPlane,
		D3D9_GetClipPlane,
		D3D9_SetRenderState,
		D3D9_GetRenderState,
		D3D9_CreateStateBlock,
		D3D9_BeginStateBlock,
		D3D9_EndStateBlock,
		D3D9_SetClipStatus,
		D3D9_GetClipStatus,
		D3D9_GetTexture,
		D3D9_SetTexture,
		D3D9_GetTextureStageState,
		D3D9_SetTextureStageState,
		D3D9_GetSamplerState,
		D3D9_SetSamplerState,
		D3D9_ValidateDevice,
		D3D9_SetPaletteEntries,
		D3D9_GetPaletteEntries,
		D3D9_SetCurrentTexturePalette,
		D3D9_GetCurrentTexturePalette,
		D3D9_SetScissorRect,
		D3D9_GetScissorRect,
		D3D9_SetSoftwareVertexProcessing,
		D3D9_GetSoftwareVertexProcessing,
		D3D9_SetNPatchMode,
		D3D9_GetNPatchMode,
		D3D9_DrawPrimitive,
		D3D9_DrawIndexedPrimitive,
		D3D9_DrawPrimitiveUP,
		D3D9_DrawIndexedPrimitiveUP,
		D3D9_ProcessVertices,
		D3D9_CreateVertexDeclaration,
		D3D9_SetVertexDeclaration,
		D3D9_GetVertexDeclaration,
		D3D9_SetFVF,
		D3D9_GetFVF,
		D3D9_CreateVertexShader,
		D3D9_SetVertexShader,
		D3D9_GetVertexShader,
		D3D9_SetVertexShaderConstantF,
		D3D9_GetVertexShaderConstantF,
		D3D9_SetVertexShaderConstantI,
		D3D9_GetVertexShaderConstantI,
		D3D9_SetVertexShaderConstantB,
		D3D9_GetVertexShaderConstantB,
		D3D9_SetStreamSource,
		D3D9_GetStreamSource,
		D3D9_SetStreamSourceFreq,
		D3D9_GetStreamSourceFreq,
		D3D9_SetIndices,
		D3D9_Indices,
		D3D9_CreatePixelShader,
		D3D9_SetPixelShader,
		D3D9_GetPixelShader,
		D3D9_SetPixelShaderConstantF,
		D3D9_GetPixelShaderConstantF,
		D3D9_SetPixelShaderConstantI,
		D3D9_GetPixelShaderConstantI,
		D3D9_SetPixelShaderConstantB,
		D3D9_GetPixelShaderConstantB,
		D3D9_DrawRectPatch,
		D3D9_DrawTriPatch,
		D3D9_DeletePatch,
		D3D9_CreateQuery,
		D3D9_ENUMEND
	};

	typedef int (__stdcall *procEnumProcessModulesExPtr)(
		HANDLE hProcess,
		HMODULE *lphModule,
		DWORD cb,
		LPDWORD lpcbNeeded,
		DWORD dwFilterFlag
    );
	extern procEnumProcessModulesExPtr procEnumProcessModulesEx;

	template<typename _funcType> void executeFunctions(const size_t num, ...);

	bool loadFont(const wchar_t* path);

	void convertImageNameToRealName(std::wstring*);
	BYTE getOperatingSystem();
	bool is64BitSystem();
	bool is64OperatingSystem();
	bool is64BitModule(HANDLE module);
	bool is64BitModule(HMODULE module);
	bool is64BitModule(DWORD threadId);
	bool is64BitModule(const wchar_t *name);

	bool isTarget64Bit(const wchar_t *windowName);
	bool isTarget64Bit(const wchar_t *windowName, const wchar_t *exeName);

	void isWindowExistent(DWORD threadId, HWND *result);
	void isWindowExistent(DWORD threadId, const wchar_t *exeName, HWND *result);
	void isWindowExistent(const wchar_t *windowName, HWND *exHwnd);
	void isWindowExistent(const wchar_t *windowName, const wchar_t *exeName, HWND *exHwnd);
	void isWindowExistent(HANDLE process, HWND *result);
	void isWindowExistent(HMODULE dll, HWND *result);

	std::vector<HWND> getAllWindowsOfProcess(DWORD threadId);
	HWND getWindowByProcessID(DWORD processId);

	int stringCompare(const std::wstring& wString, const std::string& aString);
	void convertStringWA(std::wstring& storage, const std::string& data, bool forceWide=false);
	void convertStringAW(std::string& storage, const std::wstring& data, bool forceShrink = true);
	void replaceTermInString(std::string& textBlock, const char* termToSearchFor, const char* replacementForSearchTerm);
	void removeTermFromString(std::string& textBlock, const char* term);
	void removeTermFromString(std::string& textBlock, const char* termStart, const char* termEnd);
	bool substringExists(const char* buffer, const char* termToLookFor, bool caseSensitive = false);

	void intAsWString(const DWORD data, std::wstring& result);
	void intAsWString(const unsigned int data, std::wstring& result);
	void wstringAsFloat(const std::wstring& data, float& result);
	void wstringAsInt(const std::wstring& data, DWORD& result, bool ignoreComma);
	void wstringAsCurrency(const std::wstring& data, DWORD& result);
	void floatAsInt(const float& toConvert, DWORD& result);
	void intAsFloat(const DWORD& toConvert, float& result);
	float fRand(const float max = 100.0f, bool allowNegative=false);
	template<class _Ty> _Ty random(const _Ty max) {
		if(max == _Ty(0))
			return _Ty(0);
		return rand() % max;
	}

	void getClipboardData(std::wstring& dataStorage);
	void setClipboardData(std::wstring& dataToAssign);
	
	void getPath(std::string *cArray, bool removeHandle = true);

	void getPath(std::wstring *wArray, bool removeHandle = true);
	void getPath(DWORD handle, std::wstring *wArray, bool removeHandle = true);
	void getPath(HWND handle, std::wstring *wArray, bool removeHandle = true);
	void getPath(HMODULE handle, std::wstring *wArray, bool removeHandle = true);
	void getPath(HANDLE handle, std::wstring *wArray, bool removeHandle = true);

	void getHandleName(std::wstring *wArray, bool removePath = true);
	void getHandleName(DWORD handle, std::wstring *wArray, bool removePath = true);
	void getHandleName(HWND handle, std::wstring *wArray, bool removePath = true);
	void getHandleName(HMODULE handle, std::wstring *wArray, bool removePath = true);
	void getHandleName(HANDLE handle, std::wstring *wArray, bool removePath = true);

	time_t getFileCreationTime(const char* filePath);
	time_t getFileCreationTime(const wchar_t* filePath);
	time_t getFileLastChangeTime(const char* filePath);
	time_t getFileLastChangeTime(const wchar_t *filePath);

	DWORD getThreadIdByName(const wchar_t *executableName);
	DWORD getThreadIdByHWND(HWND window);
	DWORD getParentThreadId();
	DWORD getParentThreadId(const wchar_t *exeName);

	bool isDllLoaded(HMODULE dll);
	bool isDllLoaded(std::wstring pathToDll);
	bool isDllLoaded(const wchar_t *pathToDll);

	bool ejectDLL(DWORD threadId, std::wstring dllName);
	bool ejectDLL(HANDLE process, std::wstring dllName);

	LPCTSTR folderDialog(LPCTSTR initPath, tstring& result);

	DWORD getAllRunningProcesses(std::wstring *result);
	DWORD getAllRunningProcesses(QUICKPROCESSINFOS *pResult, DWORD maxEntries = 1);

	void getAllDllsOfProcess(DWORD threadId, std::wstring *result, bool removePath = true);
	void getAllDllsOfProcess(HANDLE process, std::wstring *result, bool removePath = true);
	
	void loadAllDllsOfProcess(HANDLE process);
	void getCallStack(std::string& log);

	void getDllExports(std::wstring pathToDll, std::wstring* result);
	void getDllExports(HMODULE hDll, std::wstring* result);

	const char* getErrorName(DWORD reason);
	const char* getErrorDescription(DWORD reason);

#undef max
	template<class _Ty> const _Ty& max(const _Ty& a, const _Ty& b);
#undef min
	template<class _Ty> const _Ty& min(const _Ty& a, const _Ty& b);
	template<class _Ty> __inline _Ty round(double x) {
		return static_cast<_Ty>(::floor(x + 0.5));
	}

	unsigned int* removeHandleFromPath(std::wstring, unsigned int *res);
	unsigned int* removePathFromHandle(std::wstring, unsigned int *res);

	DWORD numlen(DWORD number);
	DWORD fnumlen(double number);

	void getFilesFromDirectoryW(std::wstring& filePath, std::vector<std::wstring>& result, bool parseSubDirectories);
	void getFilesFromDirectoryW(std::wstring& filePath, std::wstring& extensionFilter, std::vector<std::wstring>& result, bool parseSubDirectories);
	void getFilesFromDirectoryA(std::string& filePath, std::vector<std::string>& result, bool parseSubDirectories=true);
	void getFilesFromDirectoryA(std::string& filePath, std::string& extensionFilter, std::vector<std::string>& result, bool parseSubDirectories=true);

	//http://stackoverflow.com/questions/4064134/arraysize-c-macro-how-does-it-work
	template< class Type, ptrdiff_t n> static ptrdiff_t arraySize(Type(&)[n]) { return n; }

	//FOR DIRECTX
	unsigned long* createVTable(HMODULE d9dll, DWORD *vTable, std::wstring *log = nullptr, DWORD vTableSize = D3D9_ENUMEND, DWORD timeoutInMs = 20000, bool logOffsets = false);
	DWORD getDirectXOffset(unsigned long *pVTable, DWORD functionId);

#ifndef AddWindowStyle
#define AddWindowStyle(hwnd, WS_NEWSTYLE) SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | (WS_NEWSTYLE))
#endif

#ifndef RemoveWindowStyle
#define RemoveWindowStyle(hwnd, WS_STYLE) SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~(WS_STYLE))
#endif

#ifndef AddWindowExStyle
#define AddWindowExStyle(hwnd, WS_EX_NEWSTYLE) SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | (WS_EX_NEWSTYLE))
#endif

#ifndef RemoveWindowExStyle
#define RemoveWindowExStyle(hwnd, WS_EX_STYLE) SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~(WS_EX_STYLE))
#endif

}

#ifdef __DEBUG__
CMyFile logFile;
#endif

#if _SORTEDLIST_WITHOUT_ALLOCATOR_
template<class _keyarable, class _ValueType> class SortedList {
public:
	static const DWORD _NewBuyMinimum = 3;
	static const DWORD _NewMarkPercentage = 90;

	struct ValueKeyPair {
		_keyarable compareVal;
		_ValueType val;
		ValueKeyPair(const _keyarable& comp, const _ValueType& value) : val(value), compareVal(comp) {
		}
	};

	struct ContainerType {
		ValueKeyPair **_Values;
		size_t _UsedSize;
		size_t _MaxSize;

		ContainerType(size_t _valSize) {
			this->_MaxSize = _valSize;
			this->_UsedSize = 0x00;
			this->_Values = new SortedList<_keyarable, _ValueType>::ValueKeyPair*[_valSize];
		}
		ContainerType() {
			this->_MaxSize = this->_UsedSize = 0x00;
			this->_Values = nullptr;
		}
		void add(const class SortedList<_keyarable, _ValueType>::iterator& it) {
			if (this->_UsedSize < this->_MaxSize) {
				this->_Values[this->_UsedSize] = (*it);
				this->_UsedSize++;
			}
		}
	};

	class iterator {
		private:
			ContainerType _containerPtr;
			size_t _containerIdx;
			friend class SortedList<_keyarable, _ValueType>;
			iterator(ContainerType &_newContainerPtr, size_t containerIdx) : _containerPtr(_newContainerPtr) {
				if (this->_containerPtr._Values == nullptr) {
					this->_containerIdx = 0x00;
				}
				else {
					this->_containerIdx = (containerIdx >= _newContainerPtr._UsedSize ? _newContainerPtr._UsedSize: containerIdx);
				}
			}
		public:
			iterator() {
				this->_containerIdx = 0x00;
			}
			iterator(const iterator& rhs) {
				this->_containerPtr = rhs._containerPtr;
				this->_containerIdx = rhs._containerIdx;
			}

			iterator& operator=(const iterator& rhs) {
				this->_containerPtr = rhs._containerPtr;
				return (*this);
			}

			iterator& operator++() {
				this->_containerIdx++;
				return (*this);
			}
			iterator operator++(int) {
				iterator tmp(*this);
				this->operator++();
				return tmp;
			}

			iterator& operator--() {
				this->_containerIdx--;
				return (*this);
			}
			iterator operator--(int) {
				iterator tmp(*this);
				this->operator--();
				return tmp;
			}

			bool operator==(const iterator& rhs) {
				if (this->_containerIdx == rhs._containerIdx && this->_containerPtr._Values == rhs._containerPtr._Values)
					return true;
				return false;
			}
			bool operator!=(const iterator& rhs) {
				return !(this->operator==(rhs));
			}

			ValueKeyPair* operator*() {
				if (this->_containerIdx >= this->_containerPtr._UsedSize)
					return nullptr;
				return this->_containerPtr._Values[this->_containerIdx];
			}

			ValueKeyPair* operator->() {
				if (this->_containerIdx >= this->_containerPtr._UsedSize)
					return nullptr;
				return this->_containerPtr._Values[this->_containerIdx];
			}
	};	
	__inline iterator begin() {
		return iterator(this->_container, 0x00);
	}
	__inline iterator end() {
		return iterator(this->_container, MAXDWORD);
	}

private:

	ContainerType _container;

	struct _NewBuyInformationStruct {
		size_t _Mark;
		size_t _NewTotalSize;
	} _NewBuyInformation;


	//Used algorithm: Divide and conquer (more divide, less conquer)
	WORD divideList(const _keyarable &toInsert, WORD lowerLimit, WORD upperLimit) {
		WORD pivot = (upperLimit + lowerLimit) / 2;
		SortedList<_keyarable, _ValueType>::ValueKeyPair* smallestSample = this->_container._Values[lowerLimit];
		SortedList<_keyarable, _ValueType>::ValueKeyPair* pivotEntry = this->_container._Values[pivot];
		SortedList<_keyarable, _ValueType>::ValueKeyPair* biggestSample = this->_container._Values[upperLimit];

		if (toInsert <= smallestSample->compareVal) {
#ifdef __DEBUG__
			::logFile.putStringWithVar(L"Insert(%i) <= smallestSample(%i) -- inserting at %i of %i\n", toInsert->compareVal, smallestSample->compareVal, lowerLimit, this->list.size());
#endif
			return lowerLimit;
		}
		else if (toInsert >= biggestSample->compareVal) {
#ifdef __DEBUG__
			::logFile.putStringWithVar(L"Insert(%i) >= biggestSample(%i) -- inserting at %i of %i\n", toInsert->compareVal, biggestSample->compareVal, (upperLimit + 1), this->list.size());
#endif
			return (upperLimit + 1);
		}
		if (((upperLimit - lowerLimit) / 2) == 0) {
#ifdef __DEBUG__
			::logFile.putStringWithVar(L"avg(upperLimit,lowerLimit)(%i) <= 1 -- inserting at %i of %i\n", ((upperLimit - lowerLimit) / 2), lowerLimit, this->list.size() );
#endif
			return (toInsert == smallestSample->compareVal ? lowerLimit : lowerLimit + 1);
		}

		//If the sample is in the smaller bracket
		if (toInsert >= smallestSample->compareVal && toInsert <= pivotEntry->compareVal) {
#ifdef __DEBUG__
			::logFile.putStringWithVar(L"toInsert(%i) >= smallestSample(%i) && toInsert(%i) <= pivotEntry(%i)\n", toInsert->compareVal, smallestSample->compareVal, toInsert->compareVal, pivotEntry->compareVal);
#endif
			return divideList(toInsert, lowerLimit, pivot);
		}
		if (toInsert >= pivotEntry->compareVal && toInsert <= biggestSample->compareVal) {
#ifdef __DEBUG__
			::logFile.putStringWithVar(L"toInsert(%i) >= pivotEntry(%i) && toInsert(%i) <= biggestSample(%i)\n", toInsert->compareVal, pivotEntry->compareVal, toInsert->compareVal, biggestSample->compareVal);
#endif
			return divideList(toInsert, pivot, upperLimit);
		}
#ifdef __DEBUG__
		::logFile.putStringWithVar(L"No criteria fits value %i", toInsert->compareVal);
#endif

		//Neither of the criteria above is fitting (just in case?)
		return this->_container._UsedSize;
	}

	__inline size_t _calculateMark() {
		return static_cast<size_t>(floor(this->_NewBuyInformation._NewTotalSize * SortedList<_keyarable, _ValueType>::_NewMarkPercentage / 100.0f));
	}

	void _buyAdditionalSpace() {
		if (!_checkBuyMark())
			return;
		ValueKeyPair **_newContainer = new ValueKeyPair*[this->_NewBuyInformation._NewTotalSize];
		if (!_newContainer)
			throw TraceableException("Additional Space container was not allocatable!");

		SortedList< _keyarable, _ValueType>::iterator it;
		unsigned int i = 0;
		for (it = this->begin(); it != this->end(); it++, i++) {
			_newContainer[i] = *it;
		}

		ValueKeyPair **_tmpPtr = this->_container._Values;
		this->_container._Values = _newContainer;

		for (it = this->begin(); it != this->end(); it++) {
			_tmpPtr[i] = nullptr;
		}
		delete[] _tmpPtr;
		_tmpPtr = nullptr;

		this->_NewBuyInformation._NewTotalSize += min(SortedList::_NewBuyMinimum, static_cast<size_t>(ceil(this->_NewBuyInformation._NewTotalSize * 0.1f + this->_NewBuyInformation._NewTotalSize)) );
		this->_NewBuyInformation._Mark = _calculateMark();
		this->_container._MaxSize = this->_NewBuyInformation._NewTotalSize;
	}
	bool _checkBuyMark() {
		if ((this->_container._UsedSize + 1) <= this->_NewBuyInformation._Mark)
			return false;
		return true;
	}

	bool insert(size_t position, ValueKeyPair* val) {
		this->_buyAdditionalSpace();
		for (int i = this->_container._UsedSize; i > position && i>0; i--) {
			this->_container._Values[i] = this->_container._Values[i-1];
		}
		this->_container._Values[position] = val;
		this->_container._UsedSize++;
		return true;
	}
public:
	SortedList() {
		this->_NewBuyInformation._NewTotalSize = SortedList::_NewBuyMinimum;
		this->_NewBuyInformation._Mark = 0x00;
		this->_buyAdditionalSpace();
	}
	~SortedList() {
		this->clear();
	}

	__inline size_t size() const {
		return this->_container._UsedSize;
	}

	__inline bool isEmpty() const {
		return (this->size() == 0);
	}

	void clear() {
		SortedList<_keyarable, _ValueType>::iterator it;
		for (it = this->begin(); it != this->end(); it++) {
			delete (*it);
		}
		delete[] this->_container._Values;
		this->_container = nullptr;
	}

	bool operator==(const SortedList<_keyarable, _ValueType>& otherList) {
		if (this->size() != otherList.size())
			return false;
		SortedList<_keyarable, _ValueType>::iterator itThis;
		SortedList<_keyarable, _ValueType>::iterator itOther;
		for (itThis = this->begin(), itOther = otherList.begin(); itThis != this->end() && itOther != otherList.end(); itThis++, itOther++) {
			if (*itThis == *itOther)
				return true;
		}
		return true;
	}

	SortedList<_keyarable, _ValueType>& operator=(const SortedList<_keyarable, _ValueType>& listToCpy) {
		if ((*this) == listToCpy)
			return (*this);
		this->clear();
		for (it = this->begin(); it != this->end(); it++) {
			this->add(it);
		}
		return (*this);
	}

	//Adds a given Value-Key-Pair into the storage.
	//The "CompVal" needs to have a working comparison operator (<, <=, ==, >, >=, !=)
	//The actual entry can have any pointer form; It should have a working assignment operator (=), though.
	//Inserting a given pair is done via the Divide-And-Conquer approach - total complexity: O(n * log(n))
	bool add(const _keyarable& comp, const _ValueType& entry) {
		SortedList<_keyarable, _ValueType>::ValueKeyPair *newPair = new SortedList<_keyarable, _ValueType>::ValueKeyPair(comp, const_cast<_ValueType&>(entry));
		if (this->list.empty()) {
			this->list.push_back(newPair);
			return true;
		}
		WORD position = divideList(newPair->compareVal, 0, this->list.size() - 1);
#ifdef __DEBUG__
		::logFile.putString("\n\n");
#endif
		if (position == this->list.size()) {
			this->list.push_back(newPair);
		}
		else {
			std::vector<SortedList<_keyarable, _ValueType>::ValueKeyPair*> tmpQueue;
			for (unsigned int i = 0; i < position; i++) {
				tmpQueue.push_back(this->list.at(i));
			}
			tmpQueue.push_back(newPair);
			for (unsigned int j = position; j < this->list.size(); j++) {
				tmpQueue.push_back(this->list.at(j));
			}
			this->list = tmpQueue;
			return true;
		}
		return false;
	}
	//Removes a given entry based on its previously added value (of the Value-Key-Pair)
	//Complexity: O(n)
	bool _CRT_DEPRECATE_TEXT("It only removes the first found _ValueType-value. For a safer route, take remove(_keyVal, _ValueType, bool);") remove(const _ValueType& entryToDelete, bool deleteAllDuplicates) {
		bool hasDeletedAnything = false;
		for (unsigned int i = 0; i < this->list.size(); i++) {
			if (entryToDelete == this->list.at(i)->val) {

				SortedList<_keyarable, _ValueType>::ValueKeyPair* pair = *(this->list.erase(this->list.begin() + i));
				delete pair; pair = nullptr;
				if (!deleteAllDuplicates)
					return true;

				hasDeletedAnything = true;
			}
		}
		return hasDeletedAnything;
	}

	bool remove(const _keyarable& comp, const _ValueType& val, bool deleteAllDuplicates) {
		bool hasDeletedAnything = false;
		for (unsigned int i = 0; i < this->list.size(); i++) {
			if (this->list.at(i)->val == val && this->list.at(i)->compareVal == comp) {
				SortedList<_keyarable, _ValueType>::ValueKeyPair *pair = *(this->list.erase(this->list.begin() + i));
				delete pair; pair = nullptr;
				if (!deleteAllDuplicates)
					return true;
				hasDeletedAnything = true;
			}
		}
		return hasDeletedAnything;
	}

	bool contains(const _ValueType& entryToFind) const {
		for (unsigned int i = 0; i < this->list.size(); i++) {
			if (entryToFind == this->list.at(i)->val) {
				return true;
			}
		}
		return false;
	}

	__inline bool contains(const _keyarable& compValue, const _ValueType& value) {
		return (this->findPair(compValue, value) != MAXWORD);
	}


#ifndef ppFindPosition
#define ppFindPosition(totalBooleanExpression) \
	if (totalBooleanExpression) {\
		return position; \
	} \
	else if (static_cast<size_t>(position + 1) < this->list.size()) {\
		pair = this->list.at(position + 1);\
		if (totalBooleanExpression)\
			return (position + 1);\
	} else if (static_cast<size_t>(position - 1) >= 0x00) { \
		pair = this->list.at(position - 1); \
		if (totalBooleanExpression)\
			return (position - 1); \
	}
#endif

		size_t findKey(const _keyarable& compValue) {
			if (this->list.empty())
				return MAXWORD;
			WORD position = divideList(compValue, 0, this->list.size() - 1);
			ValueKeyPair* pair = this->list.at(position);
			ppFindPosition(pair->compareVal == compValue);
			return MAXWORD;
		}

		size_t findValue(const _ValueType& value) {
			for (unsigned int i = 0; i < this->size(); i++) {
				if (value == this->list.at(i)->val)
					return i;
			}
			return MAXWORD;
		}

		size_t findPair(const _keyarable& compValue, const _ValueType& value) {
			if (this->list.empty())
				return MAXWORD;
			ValueKeyPair tmpPair(compValue, value);
			WORD position = divideList(compValue, 0, this->list.size() - 1);
			ValueKeyPair* pair = this->list.at( (position >= this->list.size() ? position - 1 : position) );
			ppFindPosition(pair->compareVal == compValue && pair->val == value);
			return MAXWORD;
		}

		__inline size_t size() const {
			return this->list.size();
		}

		__inline bool isEmpty() const {
			return this->list.empty();
		}

		void clear() {
			while (!this->list.empty()) {
				ValueKeyPair *pair = *(this->list.erase(this->list.begin()));
				delete pair; pair = nullptr;
			}
			this->list.clear();
		}

		//Throws CTraceableException in case the wanted position is out of range
		_keyarable& getKeyByPosition(size_t& position) const {
			if (position >= this->list.size())
				throw TraceableExceptionARGS("Given position(%i) is bigger than the SortedListSize(%i)", position, this->list.size());
			return this->list.at(position)->compareVal;
		}

		//Throws CTraceableException in case the wanted value could not be found
		_ValueType& getValueByKey(const _keyarable& comp) const {
			WORD position = this->findKey(comp);
			if (position < this->list.size()) {
				return this->list.at(position);
			}
			throw TraceableExceptionARGS("Given position(%i) is bigger than the SortedListSize(%i)", position, this->list.size());
		}

		//Throws CTraceableException in case the wanted position is out of range
		_ValueType& getValueByPosition(const size_t& position) const {
			if (position >= this->list.size())
				throw TraceableExceptionARGS("Given position(%i) is bigger than the SortedListSize(%i)", position, this->list.size());
			return this->list.at(position)->val;
		}

		//Throws CTraceableException in case the wanted position is out of range
		ValueKeyPair*& at(const size_t& position) {
			if (position >= this->list.size())
				throw TraceableExceptionARGS("Given position(%i) is bigger than the SortedListSize(%i)", position, this->list.size());
			return this->list.at(position);
		}
	*/
};

template<class _keyarable, class _ValueType> bool operator==(SortedList<_keyarable, _ValueType>& lhs, SortedList<_keyarable, _ValueType>& rhs) {
	if (reinterpret_cast<void*>(&lhs) == reinterpret_cast<void*>(&rhs))
		return true;
	if (lhs.size() != rhs.size())
		return false;
	for (unsigned int i = 0; i < lhs.size(); i++) {
		std::auto_ptr<SortedList<_keyarable, _ValueType>::ValueKeyPair> lhsPair(new ValueKeyPair(lhs.getKeyByPosition(i), lhs.getValueByPosition(i)));
		std::auto_ptr<SortedList<_keyarable, _ValueType>::ValueKeyPair> rhsPair(new ValueKeyPair(rhs.getKeyByPosition(i), rhs.getValueByPosition(i)));

		if (lhsPair->compareVal != rhsPair->compareVal ||
			lhsPair->val != rhsPair->val) {
			return false;
		}
	}
	return true;
}
#else //SortedList with Allocator

#endif //SortedList with Allocator

/*
template<class _T, class _U>
struct allocator_switch : _U { };

template<class _T>
struct allocator_switch<_T, _T> : _T { };
*/

class Timer {
	private:
		clock_t currentTime;
		SortedList<clock_t, std::function<void()>> list;
		bool isRunning;
		
		DWORD loop() {
			while(this->isRunning) {
				this->currentTime = clock();
				for(unsigned int i=0;i<this->list.size();) {
					clock_t oTime = this->list.getKey(i);
					if(oTime <= this->currentTime)
						this->list.getValue(i)();
					else //other timers are "higher"
						break;
				}
				Sleep(1);
			}
		}

		static DWORD WINAPI threadStart(void *pParam) {
			Timer* thisPtr = reinterpret_cast<Timer*>(pParam);
			return thisPtr->loop();
		}
	public:
		Timer() {
			this->currentTime = clock();
		}
		~Timer() {
			this->isRunning = false;
		}

		void start() {
			this->isRunning = true;
			::CreateThread(nullptr, 0, threadStart, this, 0, nullptr);
		}
		__inline void add(clock_t timeToPassInMilliseconds, std::function<void()> functionToApply) {
			this->currentTime = clock();
			this->list.add( (timeToPassInMilliseconds + this->currentTime), functionToApply );
		}
};