/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>
#include <vector>
#include <iomanip>

using namespace std;

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
   // TODO: design your own class!!
   friend class BSTree<T>;
   friend class BSTree<T>::iterator;

    BSTreeNode(const T& d,BSTreeNode<T>* p =0, BSTreeNode<T>* c1 = 0, BSTreeNode<T>* c2 =0)
      :_data(d),_parents(p), _left(c1), _right(c2)  { }

    BSTreeNode<T>* getLarger(){
      BSTreeNode<T>* tracer = this;
      if(_right != 0){
        tracer = tracer ->_right;
        while(tracer -> _left != 0){
          tracer = tracer-> _left;
        }
      }
      else { // if the node left child, then parents is successor
        while(tracer->_parents !=0 && tracer != tracer->_parents->_left){
          tracer = tracer->_parents;
          if (tracer->_parents == 0){
            return this->_right;
          }
        }
        // if(tracer->_parents==0)return tracer;
        tracer = tracer->_parents;
      }
      return tracer; 
    }
    BSTreeNode<T>* getSmaller(){
      BSTreeNode* tracer = this;
      if(_left != 0){
        tracer = tracer ->_left;
        while(tracer-> _right != 0){
          tracer = tracer->_right;
        }
      }
      else { // if the node right child, then parents is Precessor
        while(tracer != tracer->_parents->_right){
          tracer = tracer->_parents;
          if(tracer->_parents == 0) return 0;
        }
        tracer = tracer->_parents;
      }
      return tracer; 
    }
    
    T _data;
    BSTreeNode<T>* _parents;
    BSTreeNode<T>* _left;
    BSTreeNode<T>* _right;
};

template <class T>
class BSTree
{
   // TODO: design your own class!!
public:
  BSTree() :_size(0){
    _root = new BSTreeNode<T>(T("*dummy"));
    _tail = _root;
  }  
  ~BSTree(){  clear(); }

    class iterator {
      friend class BSTree;
   	public:
      iterator(BSTreeNode<T>* n =0):_node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator(){} //should not delete _node

      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () {     //++ it
         _node =_node->getLarger();
         return *(this); 
      }
      iterator operator ++ (int) { 
         iterator tmp =*this;
         _node =_node->getLarger();
         return tmp; 
      }
      iterator& operator -- () { 
         _node =_node->getSmaller();
         return *(this); 
      }
      iterator operator -- (int) { 
         iterator tmp =*this;
         _node =_node->getSmaller();
         return tmp; 
      }
      iterator& operator = (const iterator& i) { 
         _node = i._node;
         return *(this); 
      }
      bool operator != (const iterator& i) const { return (_node != i._node); }
      bool operator == (const iterator& i) const { return (_node == i._node); }
   	private:
      BSTreeNode<T>* _node;
   	};


    iterator begin() const { 
      BSTreeNode<T>* tracer = _root;
      while(tracer->_left != 0){
        tracer = tracer->_left;
      }
      return iterator(tracer); 
    }
    iterator end()const{
      return iterator(_tail); 
    }
    bool empty() const{
      if(_root->_left == 0 && _root->_right == 0) return true;
      return false;
    }
    size_t size()const{ return _size; }

    void insert(const T& x){
      _size++;
      if(empty()){
        _root = new BSTreeNode<T>(x,0,0,_tail); 
        _tail->_parents = _root;
      }
      else{
        bool searching = false;
        BSTreeNode<T>* comp_node = _root;
        while(!searching){
          if(x <= (comp_node->_data) ){ //push left
            if(comp_node->_left != 0) comp_node =comp_node->_left;
            else{  //find termainal
              BSTreeNode<T>* new_node = new BSTreeNode<T>(x,comp_node,0,0);
              comp_node->_left = new_node;
              searching = true;
            }
          }
          else{                //push right
            if(comp_node->_right == _tail){    // change _tail
                BSTreeNode<T>* new_node = new BSTreeNode<T>(x, comp_node, 0 , _tail);
                comp_node->_right =new_node;
                _tail->_parents = new_node;
                searching = true;
            }
            if(comp_node->_right != 0)  comp_node = comp_node->_right;
            else{  //find termainal
              BSTreeNode<T>* new_node = new BSTreeNode<T>(x,comp_node,0,0);
              comp_node->_right = new_node;
              searching = true;
            }
          }
        }
      }
    }
    void pop_front(){
      if(empty()) return;
      BSTreeNode<T>* del_node = begin()._node;
      if(del_node == _root){
        _root =_root->_right;
        _root->_parents = NULL;
        _size--;
        return;
      }
      BSTreeNode<T>* grand_p = del_node->_parents;
      if(del_node->_right != 0){  
        grand_p->_left = del_node->_right;
        del_node->_right->_parents = grand_p;
      }
      else{  //no child node
        del_node->_parents->_left =0;
      }
      _size--;
    }
    void pop_back(){
      if(empty()) return;
      BSTreeNode<T>* del_node = _tail->_parents;
      if(del_node == _root){
        if(_root->_left != NULL)_root =_root->_left;
        else {
          _root =_tail;
          _root->_parents =NULL;
          _size--;
          return;
        }
        _root->_parents = NULL;
        placeTail(_root);
        _size--;
        return;
      }
      BSTreeNode<T>* grand_p = _tail->_parents->_parents;
      if(del_node->_left != 0){  
        grand_p->_right = del_node->_left;
        del_node->_left->_parents = grand_p;
        placeTail(grand_p->_right);
      }
      else{  //no child node
        grand_p->_right = _tail;
        _tail->_parents = grand_p;
      }
      _size--;
    }
    bool erase(iterator pos){
      BSTreeNode<T>* del_node = find(*pos);
      if(empty() || del_node == 0)  
        return false;
      --_size;
      if(del_node->_right == 0 && del_node->_left != 0){
        findParents(del_node, true);
        return true;
      }
      else if(del_node->_left == 0 && del_node->_right != _tail && del_node->_right != 0){
        findParents(del_node, false);
        return true;
      } 
      BSTreeNode<T>* suc = getSuccessor(del_node);
      while(suc != 0){  //sawp data
        del_node->_data = suc->_data;
        del_node = suc;
        suc = getSuccessor(suc);
      }   
      return true;
    }
    bool erase(const T& x){
      BSTreeNode<T>* del_node = find(x);
      if(empty() || del_node == 0)  {
        return false;
      }
      --_size;
      if(del_node->_right == 0 && del_node->_left != 0){
        findParents(del_node, true);
        return true;
      }
      else if(del_node->_left == 0 && del_node->_right != _tail && del_node->_right != 0){
        findParents(del_node, false);
        return true;
      } 
      BSTreeNode<T>* suc = getSuccessor(del_node);
      while(suc != 0){  //sawp data
        del_node->_data = suc->_data;
        del_node = suc;
        suc = getSuccessor(del_node);
      }
      return true;
    }
    void clear(){
      _size =0;
      _root = _tail;
      _tail->_parents = NULL;
    }
    void sort() const{}  //dummy func
    void print() const{  //print tree
        printInternal(cout, _root, 0);
    }
    void printInternal(ostream& os, BSTreeNode<T>* i, size_t indent) const{
      // http://stackoverflow.com/questions/1550329/how-can-i-indent-cout-output
      cout << string(indent, ' ');
      if(i)
      {
         os << i->_data << "\n";
      }
      else
      {
         os << "[0]\n";
         return; // no more elements to print
      }
      printInternal(os, i->_left, indent+2);
      printInternal(os, i->_right, indent+2);
      cout.flush();
   }
    private:
      void placeTail(BSTreeNode<T>* subroot){
        while(subroot->_right != 0){
          subroot = subroot->_right;
        }
        subroot->_right = _tail;
        _tail->_parents = subroot;
      }
      void findParents(BSTreeNode<T>* del_node, bool is_lchild){
        bool is_lparents;
        BSTreeNode<T>* grand_node = del_node->_parents;

        if(grand_node->_left == del_node) is_lparents = true;
        else is_lparents = false;

        if(is_lparents){
          if(is_lchild){
            grand_node->_left = del_node->_left;
            del_node->_left->_parents = grand_node;
          }else{
            grand_node->_left = del_node->_right;
            del_node->_right ->_parents = grand_node;
          }
        }
        else{
          if(is_lchild){
            grand_node->_right = del_node->_left;
            del_node->_left->_parents = grand_node;
          }else{
            grand_node->_right = del_node->_right;
            del_node->_right->_parents = grand_node;
          }
        }
      }
      BSTreeNode<T>* find(const T& x){
        BSTreeNode<T>* comp_node = _root;
        while (comp_node != 0 ){    
          if( x == comp_node->_data){
            return comp_node;
          }        
          if(x <= (comp_node->_data) ){            //search left
            comp_node = comp_node->_left;         
          }
          else {   //search right
            comp_node = comp_node->_right;
          }  
        }
        return comp_node;   
      }
     
      BSTreeNode<T>* getSuccessor(BSTreeNode<T>* tracer){
        //BSTreeNode<T>* tmp = tracer;
        if(tracer == _tail){  //case 8
          tracer->_parents->_right = NULL;
          _tail = tracer->_parents;
          return 0;
        }
        if(tracer->_right != 0 ){   //case 3 4 5 6 7
          tracer = tracer ->_right;
          while(tracer -> _left != 0){ //case  3
            tracer = tracer-> _left;
          } 
          if(tracer == _tail){    //case 4 5 6 7
            if(tracer->_parents->_left != 0){  // case 4 5, cases that del node has l child & _tail
              tracer =tracer->_parents->_left;  
              if(tracer->_parents->_parents == 0){  //case 5
                placeTail(tracer);
                _root = tracer;
                _root ->_parents = NULL;
                return 0;
              }
            }
            else{                 // case 6 7, cases that del node has no child but _tail
               _tail = tracer;
              if(tracer ->_parents->_parents == 0){ //case 7
                _root = _tail;
                _root->_parents = _root->_right = NULL;
                return 0;
              }
              return tracer;
            }
          }
        }
        // else if(tracer->_left != 0){ // case 2
        //   return tracer->_left;
        // }
        else {                      // case 1
          if(tracer->_parents->_left == tracer) tracer->_parents->_left =0;
          else tracer->_parents->_right =0;
          tracer = 0;
        }
        return tracer;
      }
      
      size_t _size;
      BSTreeNode<T>* _root;
      BSTreeNode<T>* _tail;
};

#endif // BST_H
