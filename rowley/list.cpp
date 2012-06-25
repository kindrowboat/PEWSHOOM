// $list.cpp 3.0 milbo$
// Derived from code by Henry Rowley http://vasc.ri.cmu.edu/NNFaceDetector.

#ifndef list_cpp
#define list_cpp

#include "list.hpp"

template<class T>
ListNode<T>::ListNode ()
{
  prev=NULL;
  next=NULL;
}

template<class T>
ListNode<T>::~ListNode ()
{
  prev=NULL;
  next=NULL;
}

template<class T>
List<T>::List ()
{
  first=NULL;
  last=NULL;
  length=0;
}

template<class T>
List<T>::~List ()
{
  deleteNodes();
}

template<class T>
void List<T>::deleteNode (T *ptr)
{
  unchain(ptr);
  delete ptr;
}

template<class T>
void List<T>::deleteNodes ()
{
  T *ptr=first;
  T *next;
  while (ptr!=NULL)
  {
    next=ptr->next;
    deleteNode(ptr);
    ptr=next;
  }
}

template<class T>
void List<T>::addLast (T *ptr)
{
  ptr->prev=last;
  if (last!=NULL) last->next=ptr;
  ptr->next=NULL;
  last=ptr;
  if (first==NULL) first=ptr;
  length++;
}

template<class T>
void List<T>::unchain (T *ptr)
{
  if (ptr->prev!=NULL) ptr->prev->next=ptr->next; else
    first=ptr->next;
  if (ptr->next!=NULL) ptr->next->prev=ptr->prev; else
    last=ptr->prev;
  ptr->prev=ptr->next=NULL;
  length--;
}

#endif  // list_cpp
