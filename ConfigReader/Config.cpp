#include "Config.h"

#pragma warning(disable:4996)

#ifndef _LINE_BREAK_VALUE
#define _LINE_BREAK_VALUE 10
#endif


ConfigA::ConfigA()
{
	this->handle = nullptr;
	this->configValues.empty();
	memset(this->path, 0x00, MAX_PATH);
}


ConfigA::~ConfigA()
{
	for(unsigned int i=0;i<this->configValues.size();i++) {
		delete this->configValues.at(i);
		this->configValues.at(i) = nullptr;
	}
	this->configValues.clear();
	fclose(this->handle);
}

bool ConfigA::init() {
	memset(this->path, 0x00, MAX_PATH);
	if (this->handle != nullptr) {
		for (unsigned int i = this->configValues.size() - 1; i >= 0; i--) {
			ValueKeyPairA* pair = this->configValues.at(i);

			delete pair;
			pair = nullptr;

			this->configValues.pop_back();
		}
		configValues.empty();
		fclose(this->handle);
		this->handle = nullptr;
	}
	return true;
}

bool ConfigA::init(const char *path) {
	if (this->handle != nullptr) {
		for (unsigned int i = this->configValues.size() - 1; i >= 0; i--) {
			ValueKeyPairA* pair = this->configValues.at(i);

			delete pair;

			this->configValues.pop_back();
		}
		configValues.empty();
		fclose(this->handle);
		this->handle = nullptr;
	}
	memset(this->path, 0x00, MAX_PATH);
	strncpy(this->path, path, MAX_PATH);
	this->handle = fopen(this->path, "r");
	if (this->handle) {
		char tmpBuf[MAX_PATH] = { 0x00 };
		while (feof(this->handle) != EOF) {
			fgets(tmpBuf, MAX_PATH, this->handle);
			std::string tmpStr = std::string(tmpBuf);

			//Comment or LineBreak
			if (tmpBuf[0] == '#' || tmpBuf[0] == 10) {
				memset(tmpBuf, 0x00, MAX_PATH);
				continue;
			}

			//additional precaution - letter+token = no real entry.
			//Unless we have a line break
			if (tmpStr.length() <= 2)
				break;

			int nameLen = min(tmpStr.find("="),32);
			int valLen = min(tmpStr.length() - nameLen - 1,MAX_PATH);
			_ASSERT(nameLen > 0 && valLen > 0);

			ValueKeyPairA *newPair = new ValueKeyPairA;
			memset(newPair->first, 0x00, 32);
			memset(newPair->second, 0x00, MAX_PATH);

			strncpy(newPair->first, tmpStr.c_str(), nameLen);
			strncpy(newPair->second, &tmpStr.c_str()[nameLen + 1], valLen);

			for (unsigned int i = 0; i < strlen(newPair->second); i++) {
				if (newPair->second[i] == _LINE_BREAK_VALUE) {
					newPair->second[i] = 0x00;
					break;
				}
			}

			this->configValues.push_back(newPair);

			tmpStr.empty();
			memset(tmpBuf, 0x00, MAX_PATH);
		}
		return true;
	}
	return false;
}

void ConfigA::dumpConfig() {
	if (strlen(this->path) == 0)
		return;
	if (this->handle == nullptr) {
		this->handle = fopen(this->path, "wb+");
	}
	if (this->handle != nullptr) {
		for (unsigned int i = 0; i < this->configValues.size(); i++) {
			std::string string = "";
			string = std::string(this->configValues.at(i)->first).append("=").append(this->configValues.at(i)->second);
			string += 0x0A;
			fputs(string.c_str(), this->handle);
		}
	}
	fclose(handle);
}

void ConfigA::dumpConfig(const char *path) {
	FILE *handle = fopen(path, "a+");
	if (!handle)
		return;
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		std::string string = "";
		string = std::string(this->configValues.at(i)->first).append("=").append(this->configValues.at(i)->second);
		string += 0x0A;
		fputs(string.c_str(), handle);
	}
	fclose(handle);
}


bool ConfigA::addValueBool(const char *valName, bool value) {
	ValueKeyPairA *newValueKeyPair = new ValueKeyPairA();
	if (newValueKeyPair == nullptr)
		return false;
	strncpy(newValueKeyPair->first, valName, 32);
	strncpy(newValueKeyPair->second, value ? "TRUE" : "FALSE", value ? 4 : 5);
	this->configValues.push_back(newValueKeyPair);
	return true;
}

bool ConfigA::addValueInt(const char *valName, int value) {
	ValueKeyPairA *newValueKeyPair = new ValueKeyPairA();
	if (newValueKeyPair == nullptr)
		return false;
	strncpy(newValueKeyPair->first, valName, 32);

	char array[64] = { 0x00 };
	_itoa(value, array, 10);
	strncpy(newValueKeyPair->second, array, 64);
	this->configValues.push_back(newValueKeyPair);
	return true;
}

bool ConfigA::addValueUInt(const char *valName, DWORD value) {
	ValueKeyPairA *newValueKeyPair = new ValueKeyPairA();
	if (newValueKeyPair == nullptr)
		return false;
	strncpy(newValueKeyPair->first, valName, 32);

	char array[64] = { 0x00 };
	(unsigned)_itoa(value, array, 10);
	strncpy(newValueKeyPair->second, array, 64);
	this->configValues.push_back(newValueKeyPair);
	return true;
}


bool ConfigA::addValueString(const char *valName, const char* value) {
	ValueKeyPairA *newValueKeyPair = new ValueKeyPairA();
	if (newValueKeyPair == nullptr)
		return false;
	strncpy(newValueKeyPair->first, valName, 32);
	strncpy(newValueKeyPair->second, value, MAX_PATH);
	this->configValues.push_back(newValueKeyPair);
	return true;
}



const char* ConfigA::getValueString(const char *valName) {
	if (this->configValues.size() == 0)
		return "";
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_stricmp(valName, this->configValues.at(i)->first) == 0) {
			return this->configValues.at(i)->second;
		}
	}
	return "";
}

bool ConfigA::getValueStringEx(const char *valName, DWORD maxLen, char **storage) {
	if (this->configValues.size() == 0)
		return false;
	
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_stricmp(valName, this->configValues.at(i)->first) == 0) {
			strncpy(*storage, this->configValues.at(i)->second, min(maxLen, strlen(this->configValues.at(i)->second)));
			return true;
		}
	}
	return false;
}

bool ConfigA::getValueBool(const char *valName) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_stricmp(valName, this->configValues.at(i)->first) == 0) {
			return _stricmp("TRUE",this->configValues.at(i)->second)==0?true:false;
		}
	}
	return false;
}

bool ConfigA::getValueBoolEx(const char *valName, bool* storage) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_stricmp(valName, this->configValues.at(i)->first) == 0) {
			*storage = _stricmp("TRUE", this->configValues.at(i)->second) == 0 ? true : false;
			return true;
		}
	}
	return false;
}


//INTEGER FUNCTIONS
int ConfigA::getValueInt(const char *valName) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_stricmp(valName, this->configValues.at(i)->first) == 0) {
			return atoi(this->configValues.at(i)->second);
		}
	}
	return (-1);
}

bool ConfigA::getValueIntEx(const char *valName, int* storage) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_stricmp(valName, this->configValues.at(i)->first) == 0) {
			*storage = atoi(this->configValues.at(i)->second);
			return true;
		}
	}
	return false;
}

//INTEGER FUNCTIONS
unsigned int ConfigA::getValueUInt(const char *valName) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_stricmp(valName, this->configValues.at(i)->first) == 0) {
			return (unsigned)atoi(this->configValues.at(i)->second);
		}
	}
	return (-1);
}

bool ConfigA::getValueUIntEx(const char *valName, unsigned int* storage) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_stricmp(valName, this->configValues.at(i)->first) == 0) {
			*storage = (unsigned)atoi(this->configValues.at(i)->second);
			return true;
		}
	}
	return false;
}

bool ConfigA::hasValue(const char *valName) {
	if (this->configValues.size() == 0)
		return false;
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		ValueKeyPairA* pair = this->configValues.at(i);
		if (_stricmp(pair->first, valName) == 0)
			return true;
	}
	return false;
}
/**
*		PROTOTYPE ONLY, NOT SAFE TO USE!
*/
DWORD ConfigA::getKeyPairBySearchTerm(const char *wSearchTerm, std::vector<ValueKeyPairA*>* resultVector) {
	std::string searchTerm = std::string(wSearchTerm);

	int wildCardPos = 0; int lastPos = 0;

	struct posKeyPairA {
		unsigned int pos;
		std::string key;
	};

	std::vector<posKeyPairA*> subStrings;
	//STRING_*_LOL_*=
	//STR*LOL=
	//LOL*=
	//*GL

	while ((wildCardPos = searchTerm.find("*", lastPos)) != -1 && searchTerm.length() > 0) {
		posKeyPairA* newPair = new posKeyPairA;
		if ((wildCardPos - lastPos) > 0 ) { //Just in case an Asterix was set as first symbol
			newPair->key = searchTerm.substr(lastPos, wildCardPos);
			newPair->pos = wildCardPos+1;
			subStrings.push_back(newPair);
		} else {
			delete newPair;
			newPair = nullptr;
		}
		searchTerm = searchTerm.substr(lastPos, wildCardPos + 1);
		lastPos = wildCardPos + 1;
	}
	if(searchTerm.length()>0) {
		posKeyPairA* newPair = new posKeyPairA;
		newPair->key = searchTerm;
		newPair->pos = lastPos;
		subStrings.push_back(newPair);
	}
	DWORD result = 0; int pos = 0; posKeyPairA *curPair;
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		ValueKeyPairA *pair = this->configValues.at(i);
		std::string str = std::string(pair->first);
		for (unsigned int j = 0; j < subStrings.size(); j++) {
			curPair = subStrings.at(j);
			pos = str.find(curPair->key, curPair->pos+pos);
			if (pos == -1)
				break;
			pos += curPair->key.length() + 1;
		}
		if (pos != -1) {
			result++;
			if (resultVector != nullptr) {
				(*resultVector).push_back(pair);
			}
		}
	}
	for(unsigned int i=0;i<subStrings.size();i++) {
		delete subStrings.at(i);
	}
	subStrings.clear();
	return result;
}

void ConfigA::getAllValuesEx(std::string& result) {
	result = std::string("\n=========== CONFIG BEGIN ===========\n");
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		ValueKeyPairA *pair = this->configValues.at(i);
		result += std::string(pair->first).append(std::string("=").append(std::string(pair->second)));
		result += 0x0A;
	}
	result += std::string("\n=========== CONFIG END ===========\n");
}



ConfigW::ConfigW()
{
	this->handle = nullptr;
	this->configValues.empty();
	memset(this->path, 0x00, sizeof(wchar_t)*MAX_PATH);
}


ConfigW::~ConfigW()
{
}

bool ConfigW::init() {
	memset(this->path, 0x00, sizeof(wchar_t)*MAX_PATH);
	if (this->handle != nullptr) {
		for (unsigned int i = this->configValues.size() - 1; i >= 0; i--) {
			ValueKeyPair* pair = this->configValues.at(i);

			delete pair;

			this->configValues.pop_back();
		}
		configValues.empty();
		fclose(this->handle);
		this->handle = nullptr;
	}
	return true;
}

bool ConfigW::init(const wchar_t *path) {
	if (this->handle != nullptr) {
		for (unsigned int i = this->configValues.size() - 1; i >= 0; i--) {
			ValueKeyPair* pair = this->configValues.at(i);

			delete pair;

			this->configValues.pop_back();
		}
		configValues.empty();
		fclose(this->handle);
		this->handle = nullptr;
	}
	memset(this->path, 0x00, sizeof(wchar_t)*MAX_PATH);
	wcsncpy(this->path, path, MAX_PATH);
	this->handle = _wfopen(this->path, L"r");
	if (this->handle) {
		wchar_t tmpBuf[MAX_PATH] = { 0x00 };
		while (feof(this->handle) != EOF) {
			fgetws(tmpBuf, MAX_PATH, this->handle);
			std::wstring tmpStr = std::wstring(tmpBuf);

			//Comment or LineBreak
			if (tmpBuf[0] == '#' || tmpBuf[0] == 10) {
				memset(tmpBuf, 0x00, MAX_PATH * sizeof(wchar_t));
				continue;
			}

			//additional precaution - letter+token = no real entry.
			//Unless we have a line break
			if (tmpStr.length() <= 2)
				break;

			int nameLen = min(tmpStr.find(L"="),32);
			int valLen = min(tmpStr.length() - nameLen - 1,MAX_PATH);
			_ASSERT(nameLen > 0 && valLen > 0);

			ValueKeyPair *newPair = new ValueKeyPair;
			memset(newPair->first, 0x00, 32 * sizeof(wchar_t));
			memset(newPair->second, 0x00, MAX_PATH * sizeof(wchar_t));

			wcsncpy(newPair->first, tmpStr.c_str(), nameLen);
			wcsncpy(newPair->second, &tmpStr.c_str()[nameLen + 1], valLen);

			for (unsigned int i = 0; i < wcslen(newPair->second); i++) {
				if (newPair->second[i] == _LINE_BREAK_VALUE) {
					newPair->second[i] = 0x00;
					break;
				}
			}

			this->configValues.push_back(newPair);

			tmpStr.empty();
			memset(tmpBuf, 0x00, MAX_PATH * sizeof(wchar_t));
		}
		return true;
	}
	return false;
}

void ConfigW::dumpConfig() {
	if (wcslen(this->path) == 0)
		return;
	if (this->handle == nullptr) {
		this->handle = _wfopen(this->path, L"wb+");
	}
	if (this->handle != nullptr) {
		for (unsigned int i = 0; i < this->configValues.size(); i++) {
			std::wstring string = L"";
			string = std::wstring(this->configValues.at(i)->first).append(L"=").append(this->configValues.at(i)->second);
			string += 0x0A;
			fputws(string.c_str(), this->handle);
		}
	}
	fclose(handle);
}

void ConfigW::dumpConfig(const wchar_t *path) {
	FILE *handle = _wfopen(path, L"a+");
	if (!handle)
		return;
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		std::wstring string = L"";
		string = std::wstring(this->configValues.at(i)->first).append(L"=").append(this->configValues.at(i)->second);
		string += 0x0A;
		fputws(string.c_str(), handle);
	}
	fclose(handle);
}


bool ConfigW::addValueBool(const wchar_t *valName, bool value) {
	ValueKeyPair *newValueKeyPair = new ValueKeyPair();
	if (newValueKeyPair == nullptr)
		return false;
	wcsncpy(newValueKeyPair->first, valName, 32);
	wcsncpy(newValueKeyPair->second, value ? L"TRUE" : L"FALSE", value ? 4 : 5);
	this->configValues.push_back(newValueKeyPair);
	return true;
}

bool ConfigW::addValueInt(const wchar_t *valName, int value) {
	ValueKeyPair *newValueKeyPair = new ValueKeyPair();
	if (newValueKeyPair == nullptr)
		return false;
	wcsncpy(newValueKeyPair->first, valName, 32);

	wchar_t array[64] = { 0x00 };
	_itow(value, array, 10);
	wcsncpy(newValueKeyPair->second, array, 64);
	this->configValues.push_back(newValueKeyPair);
	return true;
}

bool ConfigW::addValueUInt(const wchar_t *valName, DWORD value) {
	ValueKeyPair *newValueKeyPair = new ValueKeyPair();
	if (newValueKeyPair == nullptr)
		return false;
	wcsncpy(newValueKeyPair->first, valName, 32);

	wchar_t array[64] = { 0x00 };
	(unsigned)_itow(value, array, 10);
	wcsncpy(newValueKeyPair->second, array, 64);
	this->configValues.push_back(newValueKeyPair);
	return true;
}


bool ConfigW::addValueString(const wchar_t *valName, const wchar_t* value) {
	ValueKeyPair *newValueKeyPair = new ValueKeyPair();
	if (newValueKeyPair == nullptr)
		return false;
	wcsncpy(newValueKeyPair->first, valName, 32);
	wcsncpy(newValueKeyPair->second, value, MAX_PATH);
	this->configValues.push_back(newValueKeyPair);
	return true;
}

const wchar_t* ConfigW::getValueString(const wchar_t *valName) {
	if (this->configValues.size() == 0)
		return L"";
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_wcsicmp(valName, this->configValues.at(i)->first) == 0) {
			return this->configValues.at(i)->second;
		}
	}
	return L"";
}

bool ConfigW::getValueStringEx(const wchar_t *valName, DWORD maxLen, wchar_t **storage) {
	if (this->configValues.size() == 0)
		return false;
	
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_wcsicmp(valName, this->configValues.at(i)->first) == 0) {
			wcsncpy(*storage, this->configValues.at(i)->second, min(maxLen, wcslen(this->configValues.at(i)->second)));
			return true;
		}
	}
	return false;
}

bool ConfigW::getValueBool(const wchar_t *valName) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_wcsicmp(valName, this->configValues.at(i)->first) == 0) {
			return _wcsicmp(L"TRUE",this->configValues.at(i)->second)==0?true:false;
		}
	}
	return false;
}

bool ConfigW::getValueBoolEx(const wchar_t *valName, bool* storage) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_wcsicmp(valName, this->configValues.at(i)->first) == 0) {
			*storage = _wcsicmp(L"TRUE", this->configValues.at(i)->second) == 0 ? true : false;
			return true;
		}
	}
	return false;
}


//INTEGER FUNCTIONS
int ConfigW::getValueInt(const wchar_t *valName) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_wcsicmp(valName, this->configValues.at(i)->first) == 0) {
			return _wtoi(this->configValues.at(i)->second);
		}
	}
	return (-1);
}

bool ConfigW::getValueIntEx(const wchar_t *valName, int* storage) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_wcsicmp(valName, this->configValues.at(i)->first) == 0) {
			*storage = _wtoi(this->configValues.at(i)->second);
			return true;
		}
	}
	return false;
}

//INTEGER FUNCTIONS
unsigned int ConfigW::getValueUInt(const wchar_t *valName) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_wcsicmp(valName, this->configValues.at(i)->first) == 0) {
			return (unsigned)_wtoi(this->configValues.at(i)->second);
		}
	}
	return (-1);
}

bool ConfigW::getValueUIntEx(const wchar_t *valName, unsigned int* storage) {
	if (this->configValues.size() == 0)
		return false;

	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		if (_wcsicmp(valName, this->configValues.at(i)->first) == 0) {
			*storage = (unsigned)_wtoi(this->configValues.at(i)->second);
			return true;
		}
	}
	return false;
}

bool ConfigW::hasValue(const wchar_t *valName) {
	if (this->configValues.size() == 0)
		return false;
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		ValueKeyPair* pair = this->configValues.at(i);
		if (_wcsicmp(pair->first, valName) == 0)
			return true;
	}
	return false;
}
/**
*		PROTOTYPE ONLY, NOT SAFE TO USE!
*/
DWORD ConfigW::getKeyPairBySearchTerm(const wchar_t *wSearchTerm, std::vector<ValueKeyPair*>* resultVector) {
	std::wstring searchTerm = std::wstring(wSearchTerm);

	int wildCardPos = 0; int lastPos = 0;

	struct posKeyPair {
		unsigned int pos;
		std::wstring key;
	};

	std::vector<posKeyPair*> subStrings;
	//STRING_*_LOL_*=
	//STR*LOL=
	//LOL*=
	//*GL

	while ((wildCardPos = searchTerm.find(L"*", lastPos)) != -1 && searchTerm.length() > 0) {
		posKeyPair* newPair = new posKeyPair;
		if ((wildCardPos - lastPos) > 0 ) { //Just in case an Asterix was set as first symbol
			newPair->key = searchTerm.substr(lastPos, wildCardPos);
			newPair->pos = wildCardPos+1;
			subStrings.push_back(newPair);
		} else {
			delete newPair;
			newPair = nullptr;
		}
		searchTerm = searchTerm.substr(lastPos, wildCardPos + 1);
		lastPos = wildCardPos + 1;
	}
	if(searchTerm.length()>0) {
		posKeyPair* newPair = new posKeyPair;
		newPair->key = searchTerm;
		newPair->pos = lastPos;
		subStrings.push_back(newPair);
	}
	DWORD result = 0; int pos = 0; posKeyPair *curPair;
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		ValueKeyPair *pair = this->configValues.at(i);
		std::wstring str = std::wstring(pair->first);
		for (unsigned int j = 0; j < subStrings.size(); j++) {
			curPair = subStrings.at(j);
			pos = str.find(curPair->key, curPair->pos+pos);
			if (pos == -1)
				break;
			pos += curPair->key.length() + 1;
		}
		if (pos != -1) {
			result++;
			if (resultVector != nullptr) {
				(*resultVector).push_back(pair);
			}
		}
	}
	for(unsigned int i=0;i<subStrings.size();i++) {
		delete subStrings.at(i);
	}
	subStrings.clear();
	return result;
}

void ConfigW::getAllValuesEx(std::wstring& result) {
	result = std::wstring(L"\n=========== CONFIG BEGIN ===========\n");
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		ValueKeyPairW *pair = this->configValues.at(i);
		result += std::wstring(pair->first).append(std::wstring(L"=").append(std::wstring(pair->second)));
		result += 0x0A;
	}
	result += std::wstring(L"\n=========== CONFIG END ===========\n");
}

#pragma warning(default:4996)

/*

#include <iostream>
#include <io.h>
#include <fcntl.h>

void ConfigW::printAllValues() {
	_setmode(_fileno(stdout), _O_U16TEXT);
	for (unsigned int i = 0; i < this->configValues.size(); i++) {
		std::wstring tmpStr = this->configValues.at(i)->first + std::wstring(L"=") + this->configValues.at(i)->second;
		std::wcout << tmpStr.c_str() << std::endl;
	}
}
*/