// $list.hpp 3.0 milbo$
// Derived from code by Henry Rowley http://vasc.ri.cmu.edu/NNFaceDetector.

#ifndef list_hpp
#define list_hpp

template <class T>
class ListNode
{
  public:
  T *prev, *next;

  ListNode();
  ~ListNode();
};

template<class T>
class List
{
  public:
  T *first, *last;
  int length;

  List();
  ~List();
  void addLast(T *ptr);
  void deleteNode(T *ptr);
  void deleteNodes();
  void unchain(T *ptr);
  int  empty() { return (first==NULL); }
};

#endif // list_hpp
