#pragma once

#include <exception>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include "..\QuickInfos\VarsToString"
#include "CallstackGetter.h"

#ifdef max
#undef max
#endif

class TraceableException : public std::exception {
	protected:
		std::string errorMsg; 
		std::string stackTrace;
		TraceableException() {
			this->stackTrace = CallStack::getAsAscii();
			//Remove this constructor call from the CallStack
			for (unsigned int i = 0; i < 2; i++) {
				this->stackTrace = this->stackTrace.substr(this->stackTrace.find_first_of(0x0A) + 1);
			}
			this->stackTrace = std::string("StackTrace:\n") + this->stackTrace;
		}
	public:
		TraceableException(const char *msg, ... ) : TraceableException() {
			va_list arg; 
			va_start(arg, msg);
			std::string result = QuickInfo::convertVarsToString(msg, arg);
			va_end(arg);
			this->errorMsg = "Exception: ";
			this->errorMsg = this->errorMsg.append(result).append("\n\n");
		}

		~TraceableException() {
			//nothing to do here.
		}

		virtual const char* what() const throw() {
			return (this->errorMsg + this->stackTrace).c_str();
		}

		TraceableException& operator=(const TraceableException& rhs) {
			this->errorMsg = rhs.errorMsg;
			this->stackTrace = rhs.stackTrace;
			return (*this);
		}
};

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif