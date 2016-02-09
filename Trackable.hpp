#pragma once

#ifndef __TRACKABLE_DATA__
#define __TRACKABLE_DATA__

#include <xtr1common>

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

template<class _Ty, class = typename std::enable_if< std::is_integral<_Ty>::value>::type > class Trackable {
	private:
		_Ty* data;
		DWORD _size;
	public:
		Trackable() {
			this->data = nullptr;
			this->_size = 0x00;
		}

		~Trackable() {
			delete [] this->data;
			this->data = nullptr;
			this->_size = 0x00;
		}
		
		Trackable(const Trackable<_Ty>& rhs) {
			this->data = nullptr;
			(*this) = rhs;
		}

		Trackable(const _Ty* newData, DWORD newLen) {
			this->data = new _Ty[newLen + 1];
			this->_size = newLen;
			
			for(unsigned int i=0;i<newLen;i++)
				this->data[i] = newData[i];
			this->data[this->_size] = 0x00;
		}
		const Trackable<_Ty>& operator=(const Trackable<_Ty>& rhs) {
			if(this->data) {
				delete [] this->data;
				this->data = nullptr;
			}
			this->_size = rhs._size;
			this->data = new _Ty[this->_size + 1];
			for(unsigned int i=0;i<this->_size;i++)
				this->data[i] = rhs.data[i];
			this->data[this->_size] = 0x00;
			return (*this);
		}
		void init(const _Ty* newData, DWORD newLen) {
			if (this->data) {
				delete[] this->data;
				this->data = nullptr;
			}
			if (!newData) {
				return;
			}
			this->_size = newLen;
			this->data = new _Ty[this->_size + 1];
			
			memcpy(this->data, newData, this->_size * sizeof(_Ty));
			this->data[this->_size] = 0x00;
		}
		//Allow implicit casting on this one.
		__inline operator _Ty*() const { return this->data; }
		__inline const DWORD size() const { return this->_size; }
		__inline const _Ty* getData() const { return this->data; }
};

#endif //__TRACKABLE_DATA__