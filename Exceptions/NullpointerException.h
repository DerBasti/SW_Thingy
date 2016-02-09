#pragma once

#ifndef __NULLPOINTER_EXCEPTION__
#define __NULLPOINTER_EXCEPTION__

#include "TraceableException.h"

class NullpointerException : public TraceableException {
	public:
		NullpointerException() {
			this->errorMsg = std::string("Nullpointer-Exception\n");
		}
		NullpointerException(const char* msg, ...) {
			va_list args;
			va_start(args, msg);
			this->errorMsg = QuickInfo::convertVarsToString(msg, args);
			va_end(args);

			this->errorMsg = std::string("Nullpointer-Exception: ") + this->errorMsg;
		}
};

#endif //__NULLPOINTER_EXCEPTION__
