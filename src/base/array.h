// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ARRAY_H
#define ARRAY_H

#include "assert_macro.h"
#include "random.h"
#include <iostream>


/**
 set to 1 if Array is expected to hold more than UINT32_MAX elements.
 This is 4 294 967 295, a huge number!
 */
#define HANDLE_HUGE_ARRAYS 0


/** 
 Array<typename VAL> stores objects of class VAL.
 
 This class is similar to std::array<VAL> and resembles std::vector<VAL>,
 but many functions of std::vector are missing, and some functions were
 added as needed:
 - remove_pack() will pack the array removing 'zero' values,
 - sort() will sort the array given a order function,
 - allocate() request memory with the conventions of C-arrays.
 - operator[](int) returns the object stored at a particular index.
 - shuffle() permutes the values to produce a random ordering.
 - data() returns a pointer to the underlying C-array.
 .
 
 VAL needs to have a public constructor without argument.
 New memory is allocated if necessary by allocate(), and the values
 from the old array are copied to the new memory space.
 
 Allocation when it is done exceeds what is requested, to ensure that allocation
 only occurs from time-to-time, even if objects are added one by one to the array.
 */

/// Dynamically allocated array of VAL
template <typename VAL>
class Array
{
public:

    /// typedef for the template argument
    typedef VAL value_type;

    /// iterator class type
    typedef value_type * iterator;
    
    /// const iterator class type
    typedef value_type const* const_iterator;

private:
    
    /// C-array holding the values
    VAL * val_;
    
    /// size of memory that was allocated for val_[]
    size_t alc_;
    
    /// number of objects currently present in the array
    size_t nbo_;
    
    /// size of the chunk used for memory allocation (a power of 2)
    size_t chk_;
    
#pragma mark -
private:
    
    /// the integer above s that is a multiple of chk_
    size_t chunked(size_t s)
    {
        return ( s + chk_ - 1 ) & ~( chk_ - 1 );
    }
    
    /// return smallest power of 2 that is greater or equal to `s`
    size_t next_power(size_t s)
    {
        if ( s & (s-1) )
        {
            do
                s &= s-1;
            while ( s & (s-1) );
            
            s <<= 1;
        }
        return s;
    }
    
    /// copy data
    inline void copy(VAL * dst, const VAL* src, const size_t cnt)
    {
        //std::clog << "Array::copy " << cnt << "   " << src << " ---> " << dst << "\n";
        for ( size_t n = 0; n < cnt; ++n )
            dst[n] = src[n];
    }
    
#pragma mark -
public:
        
    /// Default creator without allocation
    Array() : val_(nullptr), alc_(0), nbo_(0), chk_(16)
    {
    }

    /// set chunk size to `k` (do not allocate)
    Array(size_t k) : nbo_(0)
    {
        if ( !k )
        {
            fprintf(stderr, "Array::chunk must not be null");
            exit(1);
        }

        chk_ = next_power(k);
        alc_ = chk_;
        val_ = new VAL[chk_];
    }
    
    /// Copy constructor
    Array(const Array<VAL>& o)
    : val_(nullptr), alc_(0), nbo_(o.nbo_), chk_(o.chk_)
    {
        if ( o.alc_ )
        {
            //printf("Array %p copy constructor size %i\n", this, nbo_);
            allocate(o.alc_);
            copy(val_, o.val_, o.alc_);
        }
    }

    /// Destructor
    virtual ~Array()
    {
        deallocate();
    }
    
    /// Assignment operator
    Array& operator =(Array<VAL> const & o)
    {
        if ( o.nbo_ > alc_ )
        {
            //printf("Array %p allocated %i = from size %i\n", this, alc_, o.nbo_);
            deallocate();
            allocate(o.nbo_);
        }
        nbo_ = o.nbo_;
        copy(val_, o.val_, nbo_);
        return *this;
    }
    
#pragma mark -
    
    /// Number of objects
    size_t size() const
    {
        return nbo_;
    }

    /// true if this Array holds no value
    bool empty() const
    {
        return ( nbo_ == 0 );
    }
    
    /// Currently allocated size
    size_t capacity() const
    {
        return alc_;
    }
    
    /// Address of the underlying C-array
    VAL * data()
    {
        return val_;
    }
    
    /// Address of the underlying C-array
    VAL const * data() const
    {
        return val_;
    }
    
    /// pointer to first element
    iterator begin() const
    {
        return val_;
    }
    
    /// pointer to a position just past the last element
    iterator end() const
    {
        return val_+nbo_;
    }
    
    /// reference to Object at index ii (val_[ii])
    VAL & at(const size_t ii) const
    {
        assert_true( ii < nbo_ );
        return val_[ii];
    }
    
    /// reference to Object at index ii (val_[ii])
    VAL & operator[](const size_t ii) const
    {
        assert_true( ii < nbo_ );
        return val_[ii];
    }
    
    /// return element at index 0
    VAL const& front() const
    {
        assert_true( 0 < nbo_ );
        return val_[0];
    }
    
    /// return last element
    VAL const& back() const
    {
        assert_true( 0 < nbo_ );
        return val_[nbo_-1];
    }

    
#pragma mark -
    /// Allocate to hold `s` objects: valid indices are 0 <= indx < max
    void reallocate(const size_t alc_new)
    {
        VAL * val_new = new VAL[alc_new];
        if ( val_ )
        {
            if ( alc_ < alc_new )
                copy(val_new, val_, alc_);
            else
                copy(val_new, val_, alc_new);
            delete[] val_;
        }
        alc_ = alc_new;
        val_ = val_new;
    }
    
    /// Allocate to hold at least `s` objects: valid indices are 0 <= indx < max
    size_t allocate(const size_t s)
    {
        if ( s > alc_ )
        {
            reallocate(chunked(s));
            assert_true( alc_ >= s );
            return s;
        }
        return 0;
    }
    
    /// Allocate and set new values to `zero`
    size_t allocate_zero(const size_t arg, VAL const& zero)
    {
        size_t res = allocate(arg);
        if ( res )
        {
            //set the newly allocated memory to zero
            for ( size_t ii = nbo_; ii < alc_; ++ii )
                val_[ii] = zero;
        }
        return res;
    }
    
    /// Reduce size to min(arg, actual_size), keeping elements starting from index 0
    void truncate(const size_t arg)
    {
        nbo_ = std::min(arg, nbo_);
    }
    
    /// Set size to `arg`, allocating if necessary
    void resize(const size_t arg)
    {
        if ( arg < nbo_ )
            nbo_ = arg;
        else if ( arg > nbo_ )
        {
            allocate(arg);
            nbo_ = arg;
        }
    }
    
    /// Release allocated memory
    void deallocate()
    {
        //printf("Array %p deallocate %i\n", this, allocated);
        delete[] val_;
        val_ = nullptr;
        alc_ = 0;
        nbo_ = 0;
    }
    
    /// Set the number of objects to zero
    inline void clear()
    {
        nbo_ = 0;
    }
    
    /// Delete all values as if they were pointers to Object
    void destroy()
    {
        assert_true( val_ || nbo_==0 );
        for ( size_t ii=0; ii < nbo_; ++ii )
        {
            //std::clog << " delete " << val_[ii] << std::endl;
            delete(val_[ii]);
            val_[ii] = nullptr;
        }
        nbo_ = 0;
    }
    
    /// Set all values to `zero`
    void zero(VAL const& zero)
    {
        assert_true( val_ || alc_==0 );
        for ( size_t ii=0; ii < alc_; ++ii )
            val_[ii] = zero;
    }
    
    
#pragma mark -
    
    /// Increment the size of the array, and return new value at end of it
    VAL & new_val()
    {
        if ( nbo_ >= alc_ )
            reallocate(chunked(nbo_+1));
        VAL& res = val_[nbo_++];
        return res;
    }
    
    /// Add `np` at the end of this Array
    void push_back(VAL const& v)
    {
        if ( nbo_ >= alc_ )
            reallocate(chunked(nbo_+1));
        val_[nbo_++] = v;
    }
    
    /// remove last element
    void pop_back()
    {
        --nbo_;
    }

    /// Add the elements of `array` at the end of this Array
    void append(const Array<VAL> array)
    {
        allocate(nbo_+array.nbo_);
        for ( size_t ii = 0; ii < array.nbo_; ++ii )
            val_[ii+nbo_] = array.val_[ii];
        nbo_ += array.nbo_;
    }
    
    /// Add the elements of `array` at the end of this Array
    void append_except(const Array<VAL> array, VAL const& v)
    {
        allocate(nbo_+array.nbo_);
        for ( size_t ii = 0; ii < array.nbo_; ++ii )
            if ( array.val_[ii] != v )
                val_[nbo_++] = array.val_[ii];
    }

    /// Return index of `obj`, or ~0 if not found in the list (linear search)
    size_t index(const VAL obj) const
    {
        assert_true( val_ || nbo_==0 );
        for ( size_t ii = 0; ii < nbo_; ++ii )
            if ( val_[ii] == obj )
                return ii;
        return ~0;
    }
    
    /// Replace `old_value` by `new_value`, or return false if `old_value` is not found
    bool replace(VAL const& old_value, VAL const& new_value)
    {
        assert_true( val_ || nbo_==0 );
        for ( size_t ii=0; ii < nbo_; ++ii )
        {
            if ( val_[ii] == old_value )
            {
                val_[ii] = new_value;
                return true;
            }
        }
        return false;
    }
    
#pragma mark -
    
    /// set `np` at any position equal to `zero`, or at the end of the array
    void push_pack(const VAL np, VAL const& zero)
    {
        assert_true( val_ || nbo_==0 );
        for ( size_t ii = 0; ii < nbo_; ++ii )
        {
            if ( val_[ii] == zero )
            {
                val_[ii] = np;
                return;
            }
        }
        push_back(np);
    }

    
    /// Returns the number of occurence of 'val' in the array
    size_t count(VAL const& v) const
    {
        if ( !val_ || nbo_==0 )
            return 0;
        size_t res = 0;
        for ( size_t ii = 0; ii < nbo_; ++ii )
            if ( val_[ii] == v ) ++res;
        return res;
    }

    /// Number of values which are different from `val` in the array
    size_t count_except(VAL const& v) const
    {
        if ( val_ == 0 || nbo_==0 )
            return 0;
        size_t res = 0;
        for ( size_t ii = 0; ii < nbo_; ++ii )
            if ( val_[ii] != v ) ++res;
        return res;
    }

    
    /**
     Remove all entries which are equal to `zero`, and pack array by shuffling values around.
     The order of the elements is not preserved, and copy operations are minimized
     */
    template <typename T>
    static T * remove_pack(T * s, T * e, T const& zero)
    {
        if ( e <= s )
            return s;
        --e;
        while ( s < e )
        {
            // find the next `zero` going upward:
            while ( *s != zero )
            {
                ++s;
                if ( e <= s )
                    return e + ( *e != zero );
            }
            // going downward, skip `zero` values:
            while ( *e == zero )
            {
                --e;
                if ( e <= s )
                    return e;
            }
            // flip the two values:
            *s = *e;
            *e = zero; // maybe not necessary
            ++s;
            --e;
        }
        return e + ( *e != zero );
    }
    
    
    

    /// Remove all entries which are equal to `zero`, and pack array
    void remove_pack(VAL const& zero)
    {
        assert_true( val_ || nbo_==0 );
        nbo_ = remove_pack(val_, val_+nbo_, zero) - val_;
    }
    
    
    /// Sort array using `std::qsort()` and the provided comparison function
    void sort(int (*comp)(const void *, const void *))
    {
        assert_true(val_);
        qsort(val_, nbo_, sizeof(VAL), comp);
    }
    
    /// Return one of the value in the array, chosen randomly
    VAL& pick_one()
    {
        assert_true(nbo_>0);
#if HANDLE_HUGE_ARRAYS
        return val_[RNG.pint64(nbo_)];
#else
        return val_[RNG.pint32(nbo_)];
#endif
    }

    /// Move the last Object on top, push all other values down by one slot
    void turn()
    {
        if ( nbo_ > 1 )
        {
            assert_true(val_);
        
            VAL * tmp = val_[0];
            for ( size_t ii = 0; ii < nbo_-1; ++ii )
                val_[ii] = val_[ii+1];
            val_[nbo_-1] = tmp;
        }
    }
    
    /// exchange the values of `a` and `b`
    static void swap(VAL* a, VAL* b)
    {
        VAL tmp = *a;
        *a = *b;
        *b = tmp;
    }
    
    /// Swap two random values in the array
    void permute()
    {
        assert_true(val_);
#if HANDLE_HUGE_ARRAYS
        size_t ii = RNG.pint64(nbo_);
        size_t jj = RNG.pint64(nbo_);
#else
        size_t ii = RNG.pint32(nbo_);
        size_t jj = RNG.pint32(nbo_);
#endif
        if ( ii != jj )
            swap(val_+ii, val_+jj);
    }
    
    
    /// Randomly permutes all objects in the array
    /**
     Fisher-Yates shuffle
     This produces uniform shuffling in linear time.
     see Knuth's The Art of Programming, Vol 2 chp. 3.4.2 
     */
    void shuffle32()
    {
        assert_true( nbo_ <= UINT32_MAX );
        assert_true( val_ || nbo_==0 );
        uint32_t jj = nbo_, kk;
        while ( jj > 1 )
        {
            kk = RNG.pint32(jj);  // 32 bits in [0, j-1]
            --jj;
            swap(val_+jj, val_+kk);
        }
    }
    
    /// Randomly permutes all objects in the array
    /**
     Fisher-Yates shuffle
     This produces uniform shuffling in linear time.
     see Knuth's The Art of Programming, Vol 2 chp. 3.4.2
     */
    void shuffle64()
    {
        assert_true( val_ || nbo_==0 );
        uint64_t jj = nbo_, kk;
        while ( jj > 1 )
        {
            kk = RNG.pint64(jj);  // 64 bits in [0, j-1]
            --jj;
            swap(val_+jj, val_+kk);
        }
    }

    void shuffle()
    {
#if HANDLE_HUGE_ARRAYS
        assert_true( nbo_ < UINT32_MAX );
        shuffle32();
#else
        if ( nbo_ > UINT32_MAX )
            shuffle64();
        else
            shuffle32();
#endif
    }
};


#endif
