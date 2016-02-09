#pragma once
#include <vector>
#include "ConfigDefs.h"

typedef unsigned long       DWORD;


class ConfigA {
	private:
		FILE *handle;
		std::vector < ValueKeyPairA* > configValues;
		char path[MAX_PATH];		
	public:
		ConfigA();
		~ConfigA();

		bool init( /*EMPTY CONFIG*/); //EMPTY
		bool init(const char *path);

		void dumpConfig();
		void dumpConfig(const char *path);

		bool hasValue(const char *valName);

		DWORD getKeyPairBySearchTerm(const char* searchTermWithWildcard, std::vector<ValueKeyPairA*>* result);

		bool addValueBool(const char *valName, bool value);
		bool addValueInt(const char *valName, int value);
		bool addValueUInt(const char *valName, DWORD value);
		bool addValueString(const char *valName, const char *value);

		const char* getValueString(const char *valName);
		bool getValueStringEx(const char *valName, DWORD maxLen, char** storage);

		bool getValueBool(const char *valName);
		bool getValueBoolEx(const char *valName, bool *storage);

		int getValueInt(const char *valName);
		bool getValueIntEx(const char *valName, int* storage);

		unsigned int getValueUInt(const char *valName);
		bool getValueUIntEx(const char *valName, unsigned int* storage);

		void getAllValuesEx(std::string& res);
};

class ConfigW
{
	private:
		FILE *handle;
		std::vector < ValueKeyPairW* > configValues;
		wchar_t path[MAX_PATH];
	public:
		ConfigW();
		~ConfigW();

		bool init( /*EMPTY CONFIG*/); //EMPTY
		bool init(const wchar_t *path);

		void dumpConfig();
		void dumpConfig(const wchar_t *path);

		bool hasValue(const wchar_t *valName);

		DWORD getKeyPairBySearchTerm(const wchar_t* searchTermWithWildcard, std::vector<ValueKeyPairW*>* result);

		bool addValueBool(const wchar_t *valName, bool value);
		bool addValueInt(const wchar_t *valName, int value);
		bool addValueUInt(const wchar_t *valName, DWORD value);
		bool addValueString(const wchar_t *valName, const wchar_t *value);

		const wchar_t* getValueString(const wchar_t *valName);
		bool getValueStringEx(const wchar_t *valName, DWORD maxLen, wchar_t** storage);

		bool getValueBool(const wchar_t *valName);
		bool getValueBoolEx(const wchar_t *valName, bool *storage);

		int getValueInt(const wchar_t *valName);
		bool getValueIntEx(const wchar_t *valName, int* storage);

		unsigned int getValueUInt(const wchar_t *valName);
		bool getValueUIntEx(const wchar_t *valName, unsigned int* storage);

		void getAllValuesEx(std::wstring& res);
};

#define Config ConfigW
#define ValueKeyPair ValueKeyPairW