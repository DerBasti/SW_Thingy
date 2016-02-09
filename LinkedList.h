#pragma once

#ifndef __CUSTOM_LINKED_LIST__
#define __CUSTOM_LINKED_LIST__

#include <xtr1common>
#include "D:\Programmieren\Exceptions\TraceableException.h"

template<class _Ty, class T = typename std::enable_if<std::has_trivial_copy<_Ty>::value && std::has_trivial_assign<_Ty>::value>::type> class LinkedList {	
	public:
		struct Node {
			friend class LinkedList;
			private:
				_Ty value;
				Node* prev;
				Node* next;
			public:
				__inline _Ty getValue() const { return this->value; }
				__inline const _Ty& getValueCONST() const { return this->value; }
				__inline Node* getNextNode() const { return this->next; }
				__inline Node* getPreviousNode() const { return this->prev; }
		};
	private:
		Node* head;
		Node* last;
		size_t count;
	public:
		LinkedList();
		~LinkedList();

		__inline Node* getHeadNode() const { return this->head; }
		__inline const size_t getNodeCount() const { return this->count; }
		
		void add(const _Ty& data);
		Node* getNode(const size_t pos);
		_Ty& getValue(const size_t pos);

		Node* remove(const Node* node);
		Node* remove(const _Ty& data);
		Node* removeAt(const size_t pos);
		void clear();
};

//"reserve(...)" needs to be called before adding items to the list
template<class _Ty> class FixedArray {
	private:
		size_t maxSize;
		size_t usedSize;
		_Ty* container;
	public:
		FixedArray() {
			this->maxSize = 0x00;
			this->usedSize = 0x00;

			this->container = nullptr;
		}
		FixedArray(const size_t maxSize) {
			this->maxSize = this->usedSize = 0x00;
			this->reserve(maxSize);
		}
		~FixedArray() {
			this->clear();
		}
		__inline _Ty& operator[](const size_t pos) {
			return this->getValue(pos);
		}
		__inline const _Ty& operator[](const size_t pos) const {
			return (this->getValueCONST(pos));
		}

		void clear() {
			delete[] this->container;

			this->container = nullptr;
			this->usedSize = this->maxSize = 0x00;
		}

		_Ty* begin() { return this->container; }
		_Ty* end() { return &this->container[this->capacity()]; }

		void reserve(const size_t newMax) {
			if (newMax > 0 && this->maxSize == 0x00) {
				this->maxSize = newMax;
				this->usedSize = 0x00;

				this->container = new _Ty[this->maxSize];
			}
		}
		void addValue(const _Ty& val) {
			if (this->usedSize < this->maxSize) {
				this->container[this->usedSize] = val;
				this->usedSize++;
			}
		}
		const _Ty& getValueCONST(const size_t pos) const {
			if (pos >= this->usedSize)
				throw TraceableException("Position %i is outside of max %i", pos, this->usedSize);
			return this->container[pos];
		}
		_Ty& getValue(const size_t pos) const {
			if (pos >= this->usedSize)
				throw TraceableException("Position %i is outside of max %i", pos, this->usedSize);
			return this->container[pos];
		}
		const size_t capacity() const { return this->maxSize; }
		const size_t size() const { return this->usedSize; }
};

template<class _Ty> class FixedArray2D {
	private:
		_Ty** container;
		DWORD cap[2];
	public:
		FixedArray2D() {
			this->cap[0] = this->cap[1] = 0x00;
			this->container = nullptr;
		}
		~FixedArray2D() {
			for (unsigned int i = 0; i<this->cap[0]; i++)
				delete[] this->container[i];
			delete[] this->container;

			this->container = nullptr;
			this->cap[0] = this->cap[1] = 0x00;
		}
		void reserve(const DWORD firstArraySize, const DWORD secondArraySize) {
			this->cap[0] = firstArraySize;
			this->cap[1] = secondArraySize;
			this->container = new _Ty*[this->capacityTopLevel()];
			for (unsigned int i = 0; i<this->capacityTopLevel(); i++) {
				this->container[i] = new _Ty[this->capacitySubLevel()];
			}
		}
		void addValue(const DWORD first, const DWORD second, const _Ty& value) {
			if (this->capacityTopLevel() < first || this->capacitySubLevel() < second)
				return;
			this->container[first][second] = _Ty(value);
		}

		_Ty& getValue(const DWORD first, const DWORD second) const {
			if (this->capacityTopLevel() < first || this->capacitySubLevel() < second)
				throw TraceableException("Given Levels[%i, %i] are invalid. Boundry: [%i, %i]", first, second, this->capacityTopLevel(), this->capacitySubLevel());
			return this->container[first][second];
		}

		void removeValue(const DWORD first, const DWORD second) {
			if (this->capacityTopLevel() < first || this->capacitySubLevel() < second)
				return;
			this->container[first][second] = _Ty(0);
		}
		DWORD capacity() const { return this->capacityTopLevel() * this->capacitySubLevel(); }
		__inline DWORD capacityTopLevel() const { return this->cap[0]; }
		__inline DWORD capacitySubLevel() const { return this->cap[1]; }
};

template<class _Ty> class LinkedHeaderList {
	public:
		class Node {
		private:
			_Ty value;
		public:
			Node* prev;
			Node* next;

			Node() {
				prev = next = nullptr;
				value = nullptr;
			}
			__inline const _Ty& getValue() const { return this->value; }
		};
	private:
		DWORD headCount;
		Node** nodeHeadList;
	public:
		explicit LinkedHeaderList(const DWORD newHeadCount) {
			this->headCount = newHeadCount;
			this->nodeHeadList = new Node*[this->headCount];
			for (unsigned int i = 0; i<this->headCount; i++) {
				this->nodeHeadList[i] = new Node();
				this->nodeHeadList[i]->prev = nullptr;
				this->nodeHeadList[i]->next = nullptr;
			}
		}
		~LinkedHeaderList() {
			for (unsigned int i = 0; i<this->headCount; i++) {
				Node* curNode = this->nodeHeadList[i];
				while (curNode) {
					//Get the next node before deleting it
					Node* nextNode = curNode->next;

					//Now we can delete the current node
					delete curNode;
					curNode = nullptr;

					//And mark it as next entry
					curNode = nextNode;
				}
			}
		}
		void addEntry(const DWORD headerId, const _Ty& value) {
			Node* curNode = this->nodeHeadList[headerId % this->headCount];
			while (curNode) {
				if (curNode->next == nullptr) {
					//Create a new linked node
					Node* newNode = new LinkedHeaderList::Node<_Ty>();
					newNode->prev = curNode;
					newNode->next = nullptr;

					//Assign the newest value to the last node
					curNode->next = newNode;
					curNode->value = value;
					return;
				}
				curNode = curNode->next;
			}
		}
		void removeEntry(const DWORD headerId, const _Ty& value) {
			DWORD curHeaderId = headerId % this->headCount;
			Node* curNode = this->nodeHeadList[curHeaderId];
			while (curNode) {
				if (curNode->getValue() == value) {
					//in case we have a previous node, 
					//re-wire it.
					if (curNode->prev != nullptr) {
						curNode->prev->next = curNode->next;
					}
					//Same as above, just with the next node
					if (curNode->next != nullptr) {
						curNode->next->prev = curNode->prev;
					}
					//In case we're deleting the head node
					if (curNode == this->getHeadEntry(curHeaderId)) {
						this->nodeHeadList[curHeaderId] = this->nodeHeadList[curHeaderId]->next;
					}
					delete curNode;
					curNode = nullptr;
				}
			}
		}
		__inline Node* getHeadEntry(const DWORD headerId) {
			return this->nodeHeadList[headerId % this->getHeadCount()];
		}
		__inline Node* getNextEntry(Node* curNode) {
			return curNode->next;
		}
		__inline const DWORD getHeaderCount() const { return this->headCount; }
};

template < class _KeyType, class _ValType, typename _KeyAlloc = std::allocator<_KeyType>, typename _ValAlloc = std::allocator<_ValType> >
class SortedList : private _KeyAlloc, _ValAlloc {
	private:
		_KeyAlloc _keyAllocator;
		_ValAlloc _valAllocator;

		template<class _Alloc, class _Ty1, class _Ty2> void constructValue(_Alloc& _alloc, _Ty1* _pDest, _Ty2&& _Src) {
			_alloc.construct(_pDest, std::forward<_Ty2>(_Src));
		}
	public:
	#ifndef typeDefinitions
	#define typeDefinitions(valName, allocName) \
		typedef allocName valName##_type; \
		typedef typename allocName##::reference valName##_reference; \
		typedef typename allocName##::const_reference valName##_const_reference; \
		\
		typedef typename allocName##::size_type valName##_size_type; \
		typedef typename allocName##::difference_type valName; \
		\
		typedef typename allocName##::pointer valName##_pointer; \
		typedef typename allocName##::const_pointer valName##_const_pointer;
	#endif

		typeDefinitions(key, _KeyAlloc);
		typeDefinitions(value, _ValAlloc);

		size_t _usedSize;
		size_t _capacity;

		template<class _Type> class _iteratorInfo {
		public:
			_Type _firstItem;
			_Type _lastItem;
			_Type _endOfContainer;
			_iteratorInfo() {
				this->_firstItem = _Type();
				this->_lastItem = _Type();
				this->_endOfContainer = _Type();
			}
		};

		_iteratorInfo<key_pointer> compareInfo;
		_iteratorInfo<value_pointer> valueInfo;

	#ifdef _CUSTOM_ITERATOR_
		typedef key_pointer key_iterator;
		typedef const value_pointer const_key_iterator;

		typedef value_pointer value_iterator;
		typedef const value_pointer const_value_iterator;

	#pragma region CompareValue Functions
		key_iterator beginKey() {
			return this->compareInfo._firstItem;
		}
		key_iterator endKey() {
			return (this->compareInfo._lastItem + 1);
		}
	#pragma endregion

	#pragma region ValueType Region
		value_iterator beginValue() {
			return this->valueInfo._firstItem;
		}
		value_iterator endValue() {
			return (this->valueInfo._lastItem + 1);
		}
	#else //ifdef _CUSTOM_ITERATOR_
	#ifdef _CUSTOM_SINGLE_ITERATOR
		template<class _ItTy, class _Ty> class iterator : public std::iterator<std::bidirectional_iterator_tag, _Ty> {
		protected:
			friend class SortedList;
			_ItTy* listPtr;
			DWORD offset;
			iterator(_ItTy* listToAssign, DWORD _offset = 0x00) {
				this->listPtr = listToAssign;
				this->offset = _offset;
			}
			iterator(const iterator* const rhs) {
				this->listPtr = rhs->listPtr;
				this->offset = rhs->offset;
			}
		public:
			iterator() {
				this->listPtr = nullptr;
				this->offset = 0x00;
			}
			iterator(const iterator& rhs) {
				this->listPtr = rhs.listPtr;
				this->offset = rhs.offset;
			}
			~iterator() {
				this->listPtr = nullptr;
				this->offset = 0x00;
			}

			iterator& operator++() {
				this->offset++;
				return (*this);
			}
			const iterator operator++(int) {
				iterator tmp(this);
				this->offset++;
				return tmp;
			}

			iterator& operator--() {
				this->offset--;
				return (*this);
			}
			const iterator operator--(int) {
				iterator tmp(this);
				this->offset--;
				return tmp;
			}

			_Ty& operator->() {
				try {
					_Ty& test = (**this);
					return test;
				}
				catch (CTraceableException& ex) {
					throw TraceableException(ex);
				}
			}

			_Ty& operator*() {
				if (this->offset >= this->listPtr->size())
					throw TraceableExceptionARGS("Iterator is out of range(%i of %i)", this->offset, this->listPtr->size());
				_Ty& value = this->listPtr->_firstItem[offset];
				return value;
			}

			bool operator==(const iterator& rhs) {
				if (this == (&rhs))
					return true;
				if (this->listPtr == rhs.listPtr) {
					if ((this->offset >= this->listPtr->size() &&
						rhs.offset >= rhs.listPtr->size()) || this->offset == rhs.offset)
						return true;
				}
				return false;
			}

			bool operator!=(const iterator& rhs) {
				return !(this->operator==(rhs));
			}
		};

		typedef iterator<_iteratorInfo<key_pointer, key_size_type>, _KeyType> key_iterator;
		typedef iterator<_iteratorInfo<value_pointer, value_size_type>, _ValType> value_iterator;

		value_iterator beginValue() {
			return value_iterator(&this->valueInfo);
		}
		value_iterator endValue() {
			return value_iterator(&this->valueInfo, this->size());
		}

		key_iterator beginKey() {
			return key_iterator(&this->compareInfo);
		}
		key_iterator endKey() {
			return key_iterator(&this->compareInfo, this->size());
		}
		bool containsKey(const _KeyType compVal) {
			key_iterator it = this->cbegin();
			for (; it != this->cend(); it++) {
				if (it == compVal)
					return true;
			}
			return false;
		}

		bool containsValue(const _ValType dataVal) {
			value_iterator it = this->vbegin();
			for (; it != this->vend(); it++) {
				if (it == dataVal)
					return true;
			}
			return false;
		}

		bool removeByKey(const _KeyType compVal) {
			throw TraceableException("Method not implemented yet!");
		}
		bool removeByValue(const _ValType dataVal) {
			throw TraceableException("Method not implemented yet!");
		}

		bool remove(const _KeyType compVal, const _ValType dataVal) {
			throw TraceableException("Method not implemented yet!");
		}

		key_size_type findKeyPosition(const _KeyType key) {
			key_iterator it = this->cbegin();
			for (key_size_type i = 0; it != this->cend(); it++, i++) {
				if (*it == key)
					return i;
			}
			return -1;
		}
		value_size_type findValuePosition(const _ValType data) {
			value_iterator it = this->vbegin();
			for (value_size_type i = 0; it != this->vend(); it++, i++) {
				if (*it == data)
					return i;
			}
			return -1;
		}
		value_size_type find(const _KeyType comp, const _ValType data) {
			value_iterator v_it = this->vbegin();
			key_iterator c_it = this->cbegin();
			for (value_size_type i = 0; c_it != this->cend() && v_it != this->vend();; c_it++, v_it++, i++) {
				if (*v_it == data && *c_it == comp)
					return i;
			}
			return -1;
		}

	#else //_CUSTOM_SINGLE_ITERATOR_
		template<class _KeyTy, class _ValTy> class _iterator : public std::iterator<std::bidirectional_iterator_tag, std::pair<_KeyTy, _ValTy> > {
		private:
			friend SortedList<_KeyTy, _ValTy>;
			SortedList<_KeyTy, _ValTy> *pList;
			size_t offset;
			_iterator(SortedList* const newList, size_t newOffset = 0x00) {
				this->pList = newList;
				this->offset = newOffset;
			}
		public:
			_iterator() {
				this->pList = nullptr; this->offset = 0x00;
			}
			_iterator(const _iterator& rhs) {
				this->pList = rhs.pList;
				this->offset = rhs.offset;
			}
			~_iterator() {
				this->pList = nullptr; this->offset = 0x00;
			}
			_iterator& operator=(const _iterator& rhs) {
				this->pList = rhs.pList;
				this->offset = rhs.offset;
				return (*this);
			}
			_iterator& operator++() {
				this->offset++;
				return (*this);
			}
			const _iterator operator++(int) {
				_iterator tmp(*this);
				this->offset++;
				return tmp;
			}
			_iterator& operator--() {
				this->offset--;
				return (*this);
			}
			const _iterator operator--(int) {
				iterator tmp(*this);
				this->offset--;
				return tmp;
			}
			bool operator==(const _iterator& rhs) {
				if (this == (&rhs))
					return true;
				if (this->pList == rhs.pList) {
					if (this->offset >= this->pList->size() &&
						rhs.offset >= this->pList->size())
						return true;
				}
				return false;
			}

			bool operator!=(const _iterator& rhs) {
				return !(this->operator==(rhs));
			}

			std::pair<_KeyTy, _ValTy> operator->() {
				return (**this);
			}

			std::pair<_KeyTy, _ValTy> operator*() {
				if (this->pList != nullptr && this->offset < this->pList->size()) {
					return std::pair<_KeyTy, _ValTy>(this->pList->getKey(this->offset), this->pList->getValue(this->offset));
				}
				throw TraceableException("Couldn't create an empty std::pair!");
			}
		};

		typedef _iterator<_KeyType, _ValType> iterator;

		iterator begin() {
			return iterator(this);
		}
		iterator end() {
			return iterator(this, this->size());
		}

		bool containsKey(const _KeyType& key) {
			iterator it = this->begin();
			for (; it != this->end(); it++) {
				if ((*it).first == key)
					return true;
			}
			return false;
		}
		bool containsValue(const _ValType& value) {
			iterator it = this->begin();
			for (; it != this->end(); it++) {
				if ((*it).second == value)
					return true;
			}
			return false;
		}
		size_t find(const _KeyType& key, const _ValType& value) {
			iterator it = this->begin();
			for (size_t pos = 0x00; it != this->end(); it++, pos++) {
				if ((*it).first == key && (*it).second == value)
					return pos;
			}
			return static_cast<size_t>(-1);
		}
		key_size_type findKeyPosition(const _KeyType& key) {
			key_size_type position = 0x00;
			for (; position != this->size(); position++) {
				if (*(this->compareInfo._firstItem + position) == key)
					return position;
			}
			return static_cast<key_size_type>(-1);
		}

		value_size_type findValuePosition(const _ValType& value) {
			value_size_type position = 0x00;
			for (; position != this->size(); position++) {
				if (*(this->valueInfo._firstItem + position) == value)
					return position;
			}
			return static_cast<value_size_type>(-1);
		}
	#endif //_CUSTOM_SINGLE_ITERATOR_

	#endif //_CUSTOM_ITERATOR_
	#pragma endregion

		value_size_type capacity() const {
			return this->_capacity;
		}
		value_size_type size() const {
			return this->_usedSize;
		}

		bool empty() const {
			return (this->size() == 0);
		}

	#ifndef TypedClear
	#define TypedClear(structName, ptrName) this->structName##._firstItem = ptrName##();\
		this->structName##._lastItem = ptrName##(); \
		this->structName##._endOfContainer = ptrName##();
	#endif

		SortedList() {
			TypedClear(valueInfo, value_pointer);
			TypedClear(compareInfo, key_pointer);

			this->_usedSize = 0x00;
			this->_capacity = 0x00;
		}

		SortedList(const SortedList& rhs) {
			this->clear();

			key_pointer key_newContainer = this->_keyAllocator.allocate(rhs.capacity());
			value_pointer value_newContainer = this->_valAllocator.allocate(rhs.capacity());

	#define easierAlloc(structName, containerName) this->structName##._firstItem = containerName;\
		this->structName##._lastItem = this->structName##._firstItem + rhs.size(); \
		this->structName##._endOfContainer = this->structName##._firstItem + rhs.capacity();

			easierAlloc(compareInfo, key_newContainer);
			easierAlloc(valueInfo, value_newContainer);

			for (unsigned int i = 0; i < rhs.size(); i++) {
				this->_keyAllocator.construct(&this->compareInfo._firstItem[i], *(rhs.compareInfo._firstItem + i));
				this->_valAllocator.construct(&this->valueInfo._firstItem[i], *(rhs.valueInfo._firstItem + i));
			}
			this->_usedSize = rhs._usedSize;
			this->_capacity = rhs._capacity;
		}

		SortedList& operator=(const SortedList<_KeyType, _ValType>& rhs) {
			this->clear();

			key_pointer key_newContainer = this->_keyAllocator.allocate(rhs.capacity());
			value_pointer value_newContainer = this->_valAllocator.allocate(rhs.capacity());

			easierAlloc(compareInfo, key_newContainer);
			easierAlloc(valueInfo, value_newContainer);

			for (unsigned int i = 0; i < rhs.size(); i++) {
				this->_keyAllocator.construct(&this->compareInfo._firstItem[i], *(rhs.compareInfo._firstItem + i));
				this->_valAllocator.construct(&this->valueInfo._firstItem[i], *(rhs.valueInfo._firstItem + i));
			}
			this->_usedSize = rhs.size();
			this->_capacity = rhs.capacity();
			return (*this);
		}

		~SortedList() {
			this->clear();
		}

		void reserve(const size_t newSize) {
			if (this->valueInfo._firstItem == value_pointer() &&
				this->compareInfo._firstItem == key_pointer()) {
				this->compareInfo._firstItem = this->_keyAllocator.allocate(newSize);
				this->valueInfo._firstItem = this->_valAllocator.allocate(newSize);

				this->compareInfo._lastItem = this->compareInfo._firstItem;
				this->valueInfo._lastItem = this->valueInfo._firstItem;

				this->compareInfo._endOfContainer = this->compareInfo._firstItem + newSize;
				this->valueInfo._endOfContainer = this->valueInfo._firstItem + newSize;

				this->_usedSize = 0x00;
				this->_capacity = newSize;
			}
		}

		void clear() {
			if (this->valueInfo._firstItem != value_pointer() &&
				this->compareInfo._firstItem != key_pointer() && this->capacity()>0) {

				std::_Destroy_range(this->valueInfo._firstItem, this->valueInfo._lastItem, this->_valAllocator);
				this->_valAllocator.deallocate(this->valueInfo._firstItem, this->capacity());

				std::_Destroy_range(this->compareInfo._firstItem, this->compareInfo._lastItem, this->_keyAllocator);
				this->_keyAllocator.deallocate(this->compareInfo._firstItem, this->capacity());

				this->_usedSize = 0x00;
				this->_capacity = 0x00;

				//EMPTY "pointer"
				TypedClear(valueInfo, value_pointer);
				TypedClear(compareInfo, key_pointer);
			}
		}
		void add(const _KeyType& newCompareValue, const _ValType& data) {
			_KeyType lastKey(newCompareValue);
			_ValType lastVal(data);

			//First item == nullptr or capacity was reached.
			if (this->valueInfo._firstItem == value_pointer() ||
				this->compareInfo._firstItem == key_pointer() ||
				this->size() == this->capacity()) {

				value_size_type newSize = this->capacity() + 1;

				key_pointer key_newContainer = this->_keyAllocator.allocate(newSize);
				value_pointer value_newContainer = this->_valAllocator.allocate(newSize);

				for (unsigned int i = 0; i < this->size(); i++) {
					_KeyType currentKey(*(this->compareInfo._firstItem + i));
					_ValType currentVal(*(this->valueInfo._firstItem + i));
					if (lastKey < currentKey) {
						_KeyType tmpKey(currentKey);
						currentKey = lastKey;
						lastKey = tmpKey;

						_ValType tmpVal(currentVal);
						currentVal = lastVal;
						lastVal = tmpVal;
					}
					this->constructValue(this->_keyAllocator, &key_newContainer[i], currentKey);
					this->constructValue(this->_valAllocator, &value_newContainer[i], currentVal);
				}
				this->constructValue(this->_keyAllocator, &key_newContainer[newSize - 1], lastKey);
				this->constructValue(this->_valAllocator, &value_newContainer[newSize - 1], lastVal);

				this->clear();

				this->valueInfo._firstItem = value_newContainer;
				this->valueInfo._lastItem = this->valueInfo._firstItem + (newSize - 1);
				this->valueInfo._endOfContainer = this->valueInfo._firstItem + newSize;

				this->compareInfo._firstItem = key_newContainer;
				this->compareInfo._lastItem = this->compareInfo._firstItem + (newSize - 1);
				this->compareInfo._endOfContainer = this->compareInfo._firstItem + newSize;

				this->_capacity = newSize;
				this->_usedSize = newSize;
			}
			else {
				bool realloc = false;
				if (newCompareValue < *this->compareInfo._lastItem) {
					for (unsigned int i = 0; i < this->size(); i++) {
						_KeyType currentKey(*(this->compareInfo._firstItem + i));
						_ValType currentVal(*(this->valueInfo._firstItem + i));
						if (lastKey < currentKey) {
							_KeyType tmpKey(currentKey);
							currentKey = lastKey;
							lastKey = tmpKey;

							_ValType tmpVal(currentVal);
							currentVal = lastVal;
							lastVal = tmpVal;
							realloc = true;
						}
						if (realloc) {
							this->constructValue(this->_keyAllocator, &this->compareInfo._firstItem[i], currentKey);
							this->constructValue(this->_valAllocator, &this->valueInfo._firstItem[i], currentVal);
						}
					}
				}
				this->constructValue(this->_keyAllocator, &this->compareInfo._firstItem[this->size()], lastKey);
				this->constructValue(this->_valAllocator, &this->valueInfo._firstItem[this->size()], lastVal);

				this->_usedSize++;
				this->compareInfo._lastItem++;
				this->valueInfo._lastItem++;
			}
		}
		bool contains(const _KeyType compVal, const _ValType dataVal) {
			if (containsKey(compVal) && containsValue(dataVal))
				return true;
			return false;
		}

		bool remove(const _KeyType& key, const _ValType& value) {
			iterator it = this->begin();
			for (size_t pos = 0x00; it != this->end(); it++, pos++) {
				if ((*it).first == key && (*it).second == value) {
					this->_keyAllocator.destroy((this->compareInfo._firstItem + pos));
					this->_valAllocator.destroy((this->valueInfo._firstItem + pos));

					for (; pos < this->size() - 1; pos++) {
						this->_keyAllocator.construct((this->compareInfo._firstItem + pos), *(this->compareInfo._firstItem + pos + 1));
						this->_valAllocator.construct((this->valueInfo._firstItem + pos), *(this->valueInfo._firstItem + pos + 1));
					}
					this->_usedSize--;
					this->compareInfo._lastItem--;
					this->valueInfo._lastItem--;
					return true;
				}
			}
			return false;
		}

		bool removeByKey(const _KeyType& key) {
			iterator it = this->begin();
			for (size_t pos = 0x00; it != this->end(); it++, pos++) {
				if ((*it).first == key) {
					this->_keyAllocator.destroy((this->compareInfo._firstItem + pos));
					this->_valAllocator.destroy((this->valueInfo._firstItem + pos));

					for (; pos < this->size() - 1; pos++) {
						this->_keyAllocator.construct((this->compareInfo._firstItem + pos), *(this->compareInfo._firstItem + pos + 1));
						this->_valAllocator.construct((this->valueInfo._firstItem + pos), *(this->valueInfo._firstItem + pos + 1));
					}
					this->_usedSize--;

					this->compareInfo._lastItem--;
					this->valueInfo._lastItem--;
					return true;
				}
			}
			return false;
		}
		bool removeByValue(const _ValType& value) {
			iterator it = this->begin();
			for (size_t pos = 0x00; it != this->end(); it++, pos++) {
				if ((*it).second == value) {
					this->_keyAllocator.destroy((this->compareInfo._firstItem + pos));
					this->_valAllocator.destroy((this->valueInfo._firstItem + pos));

					for (; pos < this->size() - 1; pos++) {
						this->_keyAllocator.construct((this->compareInfo._firstItem + pos), *(this->compareInfo._firstItem + pos + 1));
						this->_valAllocator.construct((this->valueInfo._firstItem + pos), *(this->valueInfo._firstItem + pos + 1));
					}
					this->_usedSize--;

					this->compareInfo._lastItem--;
					this->valueInfo._lastItem--;
					return true;
				}
			}
			return false;
		}
		bool removeAt(size_t pos) {
			if (this->_usedSize <= pos)
				return false;
			this->_keyAllocator.destroy((this->compareInfo._firstItem + pos));
			this->_valAllocator.destroy((this->valueInfo._firstItem + pos));
			for (; pos < this->size() - 1; pos++) {
				this->_keyAllocator.construct((this->compareInfo._firstItem + pos), *(this->compareInfo._firstItem + pos + 1));
				this->_valAllocator.construct((this->valueInfo._firstItem + pos), *(this->valueInfo._firstItem + pos + 1));
			}
			this->_usedSize--;

			this->compareInfo._lastItem--;
			this->valueInfo._lastItem--;
			return true;
		}

		key_const_reference getKey(size_t position) const {
			if (position >= this->_usedSize) {
				throw TraceableException("Position is out of range(%i of %i) ", position, this->_usedSize);
			}
			return (*(this->compareInfo._firstItem + position));
		}

		value_reference getValue(size_t position) const {
			if (position >= this->_usedSize) {
				throw TraceableException("Position is out of range(%i of %i) ", position, this->_usedSize);
			}
			return (*(this->valueInfo._firstItem + position));
		}
		value_reference getValueByKey(key_reference key) const {
			for (unsigned int i = 0; i<this->size(); i++) {
				if (*(this->compareInfo._firstItem + i) == key)
					return this->getValue(i);
			}
			throw TraceableException("Wanted key could not be found!");
		}
};

template < class _KeyType, class _ValType, class _KeyAlloc = std::allocator<_KeyType>, class _ValAlloc = std::allocator<_ValType> >
class UniqueSortedList {
	private:
		SortedList<_KeyType, _ValType, _KeyAlloc, _ValAlloc> container;
	public:
		UniqueSortedList() {
			this->container.clear();
		}
		~UniqueSortedList() {
		}

		size_t size() const { return this->container.size(); }
		size_t capacity() const { return this->container.capacity(); }

		bool add(const _KeyType& newCompareValue, const _ValType& data) {
			if (this->container.findKeyPosition(newCompareValue) != static_cast<size_t>(-1))
				return false; //NOT A UNIQUE KEY VALUE
			this->container.add(newCompareValue, data);
			return true;
		}
		bool contains(const _KeyType key, const _ValType val) {
			return this->container.contains(key, val);
		}
		size_t find(const _KeyType key, const _ValType val) {
			return this->container.find(key, val);
		}
		size_t findKey(const _KeyType key) {
			return this->container.findKeyPosition(key);
		}
		size_t findValue(const _ValType value) {
			return this->container.findValuePosition(value);
		}
		bool removeByKey(const _KeyType& key) {
			return this->container.removeByKey(key);
		}
		bool removeByValue(const _ValType& data) {
			return this->container.removeByValue(data);
		}
		bool removeAt(size_t pos) {
			return this->container.removeAt(pos);
		}
		const _KeyType getKey(size_t pos) {
			return this->container.getKey(pos);
		}
		_ValType& getValueAtPosition(size_t pos) {
			return this->container.getValue(pos);
		}
		_ValType& getValue(const _KeyType& key) {
			return this->container.getValueByKey(key);
		}
		void clear() {
			return this->container.clear();
		}
		bool operator==(const UniqueSortedList<_KeyType, _ValType>& rhs) {
			if ((&rhs) == this)
				return true;
			if (rhs.size() != this->size() || rhs.capacity() != this->capacity())
				return false;
			if (std::memcmp(this, &rhs, sizeof(UniqueSortedList<_KeyType, _ValType>)) == 0)
				return true;
			return false;
		}
		bool operator!=(const UniqueSortedList<_KeyType, _ValType>& rhs) {
			return !(this->operator==(rhs));
		}
};


#endif //__CUSTOM_LINKED_LIST__