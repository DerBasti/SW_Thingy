#include "LinkedList.h"

template<class _Ty, class T> LinkedList<_Ty, T>::LinkedList() {
	this->count = 0x00;
	this->head = this->last = nullptr;
}

template<class _Ty, class T> LinkedList<_Ty, T>::~LinkedList() {
	this->clear();
}

template<class _Ty, class T> void LinkedList<_Ty, T>::add(const _Ty& data) {
	Node* newNode = new Node();
	if(this->count == 0x00) { //empty list
		newNode->prev = nullptr;
		newNode->next = nullptr;
		this->head = this->last = newNode;
	} else {
		newNode->prev = this->last;
		this->last->next = newNode;

		//current last doesn't get lost as the new "last" will have a remaining
		//reference to the current last.

		this->last = newNode;
	}
	this->last->value = _Ty(data);
	this->count++;
}
		
template<class _Ty, class T> typename LinkedList<_Ty, T>::Node* LinkedList<_Ty, T>::getNode(const size_t pos) {
	size_t offset = 0x00;
	LinkedList<_Ty, T>::Node* tmpNode = this->head;
	while(tmpNode && offset != pos) {
		tmpNode = tmpNode->next;
		offset++;
	}
	return tmpNode;
}

template<class _Ty, class T>  _Ty& LinkedList<_Ty, T>::getValue(const size_t pos) {
	LinkedList<_Ty, T>::Node* tmpNode = this->getNode(pos);
	if(!tmpNode)
		throw ::TraceableException("Invalid node requested!");
	return tmpNode->getValue();
}

template<class _Ty, class T> typename LinkedList<_Ty, T>::Node* LinkedList<_Ty, T>::remove(const typename LinkedList<_Ty, T>::Node* tmpNode) {
	if(tmpNode == nullptr || this->count == 0x00)
		return nullptr;

	Node* toReturn = nullptr;
	if(tmpNode == this->head) {
		if(this->count == 0x01) {
			this->head = this->last = nullptr;
		} else {
			this->head = this->head->next;
			this->head->prev = nullptr;

			//next logical successor of the earlier "head"
			toReturn = this->head;
		}
	} else if(tmpNode == this->last) {
		//toReturn stays "nullptr" as the next node
		//after the ex-last one is invalid
		this->last = this->last->prev;
		this->last->next = nullptr;
	} else { 
		//Here we can safely assume the node-count is bigger
		//than 2; we'd end up with either the headNode or 
		//the lastNode
		tmpNode->prev->next = tmpNode->next;
		tmpNode->next->prev = tmpNode->prev;

		toReturn = tmpNode->next;
	}
	this->count--;
	delete tmpNode;
	tmpNode = nullptr;

	return toReturn;
}

template<class _Ty, class T> typename LinkedList<_Ty, T>::Node* LinkedList<_Ty, T>::remove(const _Ty& data) {
	LinkedList<_Ty, T>::Node* tmpNode = this->head;
	size_t idx = 0x00;
	while(tmpNode) {
		if(tmpNode->value == data) {
			return this->remove(tmpNode);
		}
		idx++;
		tmpNode = tmpNode->next;
	}
	return nullptr;
}

template<class _Ty, class T> typename LinkedList<_Ty, T>::Node* LinkedList<_Ty, T>::removeAt(const size_t pos) {
	if(this->count == 0x00 || pos >= this->count)
		return nullptr;

	LinkedList<_Ty, T>::Node* tmpNode = this->head;
	if(pos == 0x00) {
		return this->remove(this->head);
	} 
	if(pos == this->count - 1)
		return this->remove(this->last);
	
	size_t offset = 0x00;
	//Iterate up to the wanted node
	while(tmpNode && offset != pos) {
		tmpNode = tmpNode->next;
		offset++;
	}
	return this->remove(tmpNode);
}

template<class _Ty, class T> void LinkedList<_Ty, T>::clear() {
	size_t cnt = this->getNodeCount();
	for(unsigned int i=0;i<cnt;i++)
		this->removeAt(0x00);
}