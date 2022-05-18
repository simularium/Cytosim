// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// doubly linked list, STL style, with acces by iterators,
// some additions to manipulate the list: sorting, unsorting, etc.

#include "node_list.h"
#include "assert_macro.h"
#include "random.h"
#include <stdlib.h>


void NodeList::push_front(Node * n)
{
    //Cytosim::log("NodeList: push_front   %p in   %p\n", n, this);

    n->nPrev = nullptr;
    n->nNext = nFront;
    if ( nFront )
        nFront->nPrev = n;
    else
        nBack = n;
    nFront = n;
    ++nSize;
}


void NodeList::push_back(Node * n)
{
    //Cytosim::log("NodeList: push_back   %p in   %p\n", n, this);
    
    n->nPrev = nBack;
    n->nNext = nullptr;
    if ( nBack )
        nBack->nNext = n;
    else
        nFront = n;
    nBack = n;
    ++nSize;
}


/**
 Transfer objects in `list` to the end of `this`, until `list` is empty.
 */
void NodeList::merge(NodeList& list)
{
    Node * n = list.nFront;
    
    if ( n )
    {
        if ( nBack )
            nBack->nNext = n;
        else
            nFront = n;
        
        n->nPrev = nBack;
        nBack = list.nBack;
        nSize += list.nSize;
        
        list.nSize  = 0;
        list.nFront = nullptr;
        list.nBack  = nullptr;
    }
}


void NodeList::push_after(Node * p, Node * n)
{
    n->nPrev = p;
    n->nNext = p->nNext;
    if ( p->nNext )
        p->nNext->nPrev = n;
    else
        nBack = n;
    p->nNext = n;
    ++nSize;
}


void NodeList::push_before(Node * p, Node * n)
{
    n->nNext = p;
    n->nPrev = p->nPrev;
    if ( p->nPrev )
        p->nPrev->nNext = n;
    else
        nFront = n;
    p->nPrev = n;
    ++nSize;
}


void NodeList::pop_front()
{
    assert_true( nFront );
 
    Node * n = nFront->nNext;
    nFront = n;
    n->nNext = nullptr;  // unnecessary?

    if ( nFront )
        nFront->nPrev = nullptr;
    else
        nBack = nullptr;
    --nSize;
}


void NodeList::pop_back()
{
    assert_true( nBack );
    
    Node * n = nBack->nPrev;
    nBack = n;
    n->nPrev = nullptr;  // unnecessary?
    
    if ( nBack )
        nBack->nNext = nullptr;
    else
        nFront = nullptr;
    --nSize;
}


void NodeList::pop(Node * n)
{
    assert_true( nSize > 0 );
    Node * x = n->nNext;

    if ( n->nPrev )
        n->nPrev->nNext = x;
    else {
        assert_true( nFront == n );
        nFront = x;
    }
    
    if ( x )
        x->nPrev = n->nPrev;
    else {
        assert_true( nBack == n );
        nBack = n->nPrev;
    }
    
    n->nPrev = nullptr; // unnecessary?
    n->nNext = nullptr; // unnecessary?
    --nSize;
}


void NodeList::clear()
{
    Node * p, * n = nFront;
    while ( n )
    {
        n->nPrev = nullptr; // unnecessary?
        p = n->nNext;
        n->nNext = nullptr; // unnecessary?
        n = p;
    }
    nFront = nullptr;
    nBack  = nullptr;
    nSize  = 0;
}


void NodeList::erase()
{
    Node * n = nFront;
    Node * p;
    while ( n )
    {
        p = n->nNext;
        delete(n);
        n = p;
    }
    nFront = nullptr;
    nBack  = nullptr;
    nSize  = 0;
}


/**
This is a bubble sort?
comp(a,b) = -1 if (a<b) and 1 if (a>b) or 0
*/
void NodeList::sort(int (*comp)(const void*, const void*))
{
    Node * ii = front();
    
    if ( ii == nullptr )
        return;
    
    ii = ii->next();
    
    while ( ii )
    {
        Node * kk = ii->next();
        Node * jj = ii->prev();
        
        if ( comp(ii, jj) > 0 )
        {
            jj = jj->prev();
            
            while ( jj && comp(ii, jj) > 0 )
                jj = jj->prev();
            
            pop(ii);
            
            if ( jj )
                push_after(jj, ii);
            else
                push_front(ii);
        }
        ii = kk;
    }
}


/**
This copies the data to a temporary space to use the standard library qsort()
comp(a,b) = -1 if (a<b) and 1 if (a>b) or 0
*/
void NodeList::quicksort(int (*comp)(const void*, const void*))
{
    const size_t cnt = nSize;
    Node ** tmp = new Node*[cnt];
    
    size_t i = 0;
    Node * n = nFront;
    
    while( n )
    {
        tmp[i++] = n;
        n = n->next();
    }
    
    qsort(tmp, cnt, sizeof(Node*), comp);
    
    n = tmp[0];
    nFront = n;
    n->nPrev = nullptr;
    for( i = 1; i < nSize; ++i )
    {
        n->nNext = tmp[i];
        tmp[i]->nPrev = n;
        n = tmp[i];
    }
    n->nNext = nullptr;
    nBack = n;
    
    delete[] tmp;
}


/**
 Rearrange [F--P][Q--L] into [Q--L][F--P]
 */
void NodeList::permute(Node * p)
{
    if ( p  &&  p->nNext )
    {
        nBack->nNext   = nFront;
        nFront->nPrev  = nBack;
        nFront         = p->nNext;
        nBack          = p;
        nBack->nNext   = nullptr;
        nFront->nPrev  = nullptr;
    }
    assert_false( bad() );
}


/**
 Rearrange [F--P][X--Y][Q--L] into [X--Y][F--P][Q--L]
 
 If Q is between nFront and P, this will destroy the list,
 but there is no way to check such condition here.
 */
void NodeList::shuffle_up(Node * p, Node * q)
{
    assert_true( p  &&  p->nNext );
    assert_true( q  &&  q->nPrev );
    
    if ( q != p->nNext )
    {
        nFront->nPrev   = q->nPrev;
        q->nPrev->nNext = nFront;
        nFront          = p->nNext;
        nFront->nPrev   = nullptr;
        p->nNext        = q;
        q->nPrev        = p;
    }
    assert_false( bad() );
}


/**
 Rearrange [F--P][X--Y][Q--L] into [F--P][Q--L][X--Y]
 
 If Q is between nFront and P, this will destroy the list,
 but there is no way to check such condition here.
 */
void NodeList::shuffle_down(Node * p, Node * q)
{
    assert_true( p  &&  p->nNext );
    assert_true( q  &&  q->nPrev );
    
    if ( q != p->nNext )
    {
        nBack->nNext    = p->nNext;
        p->nNext->nPrev = nBack;
        p->nNext        = q;
        nBack           = q->nPrev;
        nBack->nNext    = nullptr;
        q->nPrev        = p;
    }
    assert_false( bad() );
}


void NodeList::shuffle()
{
    if ( nSize < 2 )
        return;
    
    size_t pp, qq;
    if ( nSize > UINT32_MAX )
    {
        pp = RNG.pint64(nSize);
        qq = RNG.pint64(nSize);
    }
    else
    {
        pp = RNG.pint32(nSize);
        qq = RNG.pint32(nSize);
    }

    size_t n = 0;
    Node *p = nFront, *q;

    if ( pp+1 < qq )
    {
        for ( ; n < pp; ++n )
            p = p->nNext;
        for ( q = p; n < qq; ++n )
            q = q->nNext;
        
        shuffle_up(p, q);
    }
    else if ( qq+1 < pp )
    {
        for ( ; n < qq; ++n )
            p = p->nNext;
        for ( q = p; n < pp; ++n )
            q = q->nNext;
        
        shuffle_down(p, q);
    }
    else
    {
        for ( ; n < qq; ++n )
            p = p->nNext;

        permute(p);
    }
}


void NodeList::shuffle3()
{
    shuffle();
    shuffle();
    shuffle();
}


unsigned int NodeList::count() const
{
    unsigned int cnt = 0;
    Node * p = nFront;
    while ( p )
    {
        ++cnt;
        p = p->nNext;
    }
    return cnt;
}


bool NodeList::check(Node const* n) const
{
    Node * p = nFront;
    while ( p )
    {
        if ( p == n )
            return true;
        p = p->nNext;
    }
    return false;
}


int NodeList::bad() const
{
    size_t cnt = 0;
    Node * p = nFront, * q;
    
    if ( p  &&  p->nPrev != nullptr )
        return 71;
    while ( p )
    {
        q = p->nNext;
        if ( q == nullptr ) {
            if ( p != nBack )
                return 73;
        }
        else {
            if ( q->nPrev != p )
                return 74;
        }
        p = q;
        ++cnt;
    }
    
    if ( cnt != nSize )
        return 75;
    return 0;
}


