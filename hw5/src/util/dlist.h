/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   T              _data;
   DListNode<T>*  _prev;
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList() {
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
      _isSorted =false;
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () {     //++ it
         _node =_node->_next;
         return *(this); 
      }
      iterator operator ++ (int) { 
         iterator tmp =*this;
         _node =_node->_next;
         return tmp; 
      }
      iterator& operator -- () { 
         _node =_node->_prev;
         return *(this); 
      }
      iterator operator -- (int) { 
         iterator tmp =*this;
         _node =_node->_prev;
         return tmp; 
      }
      iterator& operator = (const iterator& i) { 
         _node = i._node;
         return *(this); 
      }
      bool operator != (const iterator& i) const { return (_node != i._node); }
      bool operator == (const iterator& i) const { return (_node == i._node); }

   private:
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const { return iterator(_head); }
   iterator end() const { 
      // DList::iterator it = DList::begin();
      // --it;
      return --iterator(_head); 
   }
   bool empty() const { return (begin() == end()); }
   size_t size() const {  
      size_t con=0;
      for(DList::iterator it = DList::begin();it != end();++it) ++con;
         return con;
   }
   void push_back(const T& x) { 
      DListNode<T>* new_node =new DListNode<T>(x, getTail(), end()._node);
      getTail()->_next = end()._node->_prev = new_node;
      _head = new_node->_next->_next;
      _isSorted =false;    
   }
   void pop_front() { 
      if(empty()) return ;
      DListNode<T>* del_node = _head;
      DListNode<T>* dummy_node = end()._node;
      _head = _head -> _next;
      _head -> _prev = dummy_node;
      dummy_node->_next =_head;
      delete del_node;
      del_node = NULL;
   }
   void pop_back() { 
      if(empty()) return ;
      DListNode<T>* del_node = getTail();
      DListNode<T>* tail_node = del_node->_prev;
      DListNode<T>* dummy_node = end()._node;
      dummy_node -> _prev = tail_node;
      tail_node->_next =dummy_node;
      delete del_node;
      del_node = NULL;
   }

   // return false if nothing to erase
   bool erase(iterator pos) {  
      DList::iterator it = DList::begin();
      for(;it != end() ;++it){
         if(it == pos ){
            if(it == begin()) _head = _head->_next;
            DListNode<T>* del_node =it._node;
            (it._node)->_prev->_next = (it._node)->_next;
            (it._node)->_next->_prev = (it._node)->_prev;
            delete del_node;
            del_node = NULL;
            return true; 
         }
      }
      return false;
   }
   bool erase(const T& x) { 
      DList::iterator it = DList::begin();
      for(;it != DList::end() ;++it){
         if(*it == x ){
            if(it == begin()) _head = _head->_next;
            DListNode<T>* del_node =it._node;
            (it._node)->_prev->_next = (it._node)->_next;
            (it._node)->_next->_prev = (it._node)->_prev;
            delete del_node;
            del_node = NULL;
            return true; 
         }
      }
      return false;
   }

   void clear() {   // delete all nodes except for the dummy node
      _head = end()._node;
      _head->_prev = _head->_next = _head;
   }
   void sort() const { 
      if(_isSorted) return;
      for(DList::iterator it = DList::begin();it !=  DList::end();it++){
         for(DList::iterator it2=it; it2 !=  DList::end();it2++)
            if(*it >= *it2) swap(*it,*it2);
      }
      _isSorted =true;
   }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted
   // [OPTIONAL TODO] helper functions; called by public member functions
   void swap(T& _data1, T& _data2) const {
      T tmp  = _data1;
      _data1 = _data2;
      _data2 =tmp;
   }
   DListNode<T>* getTail(){
      DList::iterator it = DList::end();
      it--;
      return it._node;
   }
};

#endif // DLIST_H
