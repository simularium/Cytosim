// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Created by Francois Nedelec on 04/04/2012.


#include <list>
#include <cmath>
#include <iostream>

#include "random.h"


real sim_time = 0;

/// the link of a doubly-linked list
class Linkable
{
public:
    
    Linkable * prev;
    Linkable * next;
    
public:
    
    Linkable() : prev(0), next(0) {}
    
    void push_front(Linkable *& head, Linkable *& tail)
    {
        prev = 0;
        next = head;
        if ( head )
            head->prev = this;
        else
            tail = this;
        head = this;
    }

    void push_back(Linkable *& head, Linkable *& tail)
    {
        prev = tail;
        next = 0;
        if ( tail )
            tail->next = this;
        else
            head = this;
        tail = this;
    }
    
    void pop(Linkable *& head, Linkable *& tail)
    {
        if ( prev )
            prev->next = next;
        else
            head = next;
        
        if ( next )
            next->prev = prev;
        else
            tail = prev;
        prev = 0;
        next = 0;
    }
};


/// a stochastic event
template < typename ENGINE >
class Reaction : public Linkable
{
public:
    
    typedef void (ENGINE::*MembFuncPointer)(Reaction<ENGINE>*);

private:
    
    real            mRand;
    real            mTime;
    MembFuncPointer mFunc;
    
public:
    
    Reaction(real rate, MembFuncPointer mfp)
    {
        mRand = RNG.exponential();
        mTime = mRand / rate;
        mFunc = mfp;
    }
    
    /// use this if the rate is constant
    void step(real interval)
    {
        mTime -= interval;
    }
    
    /// use this if the rate changes with time
    void step(real interval, real rate)
    {
        mRand -= rate * interval;
        if ( mRand < 0 )
            mTime = mRand / rate;
    }
    
    void act(ENGINE & obj)
    {
        // call engine's member function:
        (obj.*mFunc)(this);
        std::cout << " at time " << sim_time + mTime << std::endl;
    }
    
    // increment Gillespie time
    void renew(real rate)
    {
        if ( mRand < 0 )
        {
            mRand += RNG.exponential();
            mTime = mRand / rate;
        }
        else
        {
            mTime += RNG.exponential() / rate;
        }
    }
    
    real time()
    {
        return mTime;
    }
    
    void print(std::ostream& os)
    {
        os << mTime << "  " << mFunc << std::endl;
    }    
};


/// a stochastic engine
class Engine
{
public:
    
    typedef Reaction<Engine> Event;
    typedef Linkable * iterator;
    
protected:
    
    /// this are the ends of the event list:
    Linkable * head, * tail;
    
    
    void add(real r, Event::MembFuncPointer mfp)
    {
        (new Event(r, mfp)) -> push_front(head, tail);
    }
    
    void remove(Event * e)
    {
        e->pop(head, tail);
        delete(e);
    }
    
    
    real rate1, rate2, rate3;

public:
    
    Engine(real r1, real r2, real r3) : head(0), tail(0)
    {
        rate1 = r1;
        rate2 = r2;
        rate3 = r3;
        initialize();
    }
    
    void step(real interval)
    {
        real    earliest_time = 0;
        Event * earliest_event = 0;
        
        for ( iterator i = head; i; i = i->next )
        {
            Event * e = static_cast<Event*>(i);
            e->step(interval);
            real t = e->time();
            if ( t < earliest_time )
            {
                earliest_time = t;
                earliest_event = e;
            }
        }
        
        while ( earliest_time < 0 )
        {
            earliest_event->act(*this);
            
            earliest_time = 0;
            for ( iterator i = head; i; i = i->next )
            {
                Event * e = static_cast<Event*>(i);
                real t = e->time();
                if ( t < earliest_time )
                {
                    earliest_time = t;
                    earliest_event = e;
                }
            }
        }
    }

    //-------------------------------------------------    
    
    void event1(Event* e)
    {
        std::cout << "event 1";
        remove(e);
        add(rate2, &Engine::event2);
        add(rate3, &Engine::event3);
    }
    
    void event2(Event* e)
    {
        std::cout << "event 2";
        e->renew(rate2);
    }
    
    void event3(Event* e)
    {
        std::cout << "event 3";
        e->renew(rate3);
    }
    
    void initialize()
    {
        add(rate1, &Engine::event1);
    }
    
};


int main(int argc, char * argv[])
{
    RNG.seed();
    Engine engine(1,1,1);

    real time_step = 1;
    for ( int i = 0; i < 10; ++i )
    {
        sim_time += time_step;
        engine.step(time_step);
        std::cout << "------" << std::endl;
    }
    return 0;
}

