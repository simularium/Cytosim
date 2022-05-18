// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef NODE_H
#define NODE_H


/// Can be linked in a NodeList
/**
 This provides the necessary pointers to build doubly-linked lists:
 - nNext points to the next Node, and is null if *this is first in the list.
 - nPrev points to the previous Node, and is null if *this is last in the list.
 .
 A given Node can only be part of one NodeList. 
*/

class Node
{
    friend class NodeList;

protected:

    /// the next Node in the list
    Node      *nNext;
    
    /// the previous Node in the list
    Node      *nPrev;
        
public:

    /// constructor set as not linked
    Node() : nNext(nullptr), nPrev(nullptr) { }
    
    /// destructor
    virtual   ~Node() {};

    /// the next Node in the list, or zero if this is last
    Node*      next()      const { return nNext; }
    
    /// the previous Node in the list, or zero if this is first
    Node*      prev()      const { return nPrev; }

};


#endif
