// ------------------------------
// projects/allocator/Allocator.h
// Copyright (C) 2014
// Glenn P. Downing
// ------------------------------

#ifndef Allocator_h
#define Allocator_h

// --------
// includes
// --------

#include <cassert>   // assert
#include <cstddef>   // ptrdiff_t, size_t
#include <new>       // bad_alloc, new
#include <stdexcept> // invalid_argument
#include <cmath>
// ---------
// Allocator
// ---------

template <typename T, int N>
class Allocator {
    public:
        // --------
        // typedefs
        // --------

        typedef T                 value_type;

        typedef std::size_t       size_type;
        typedef std::ptrdiff_t    difference_type;

        typedef       value_type*       pointer;
        typedef const value_type* const_pointer;

        typedef       value_type&       reference;
        typedef const value_type& const_reference;

    public:
        // -----------
        // operator ==
        // -----------

        friend bool operator == (const Allocator&, const Allocator&) {
            return true;}                                              // this is correct

        // -----------
        // operator !=
        // -----------

        friend bool operator != (const Allocator& lhs, const Allocator& rhs) {
            return !(lhs == rhs);}

    private:
        // ----
        // data
        // ----

        char a[N];

        // -----
        // valid
        // -----

        /**
         * O(1) in space
         * O(n) in time
         * <your documentation>
         */
        bool valid () const {
    	    assert(N > 8);
    	    int sentinel = 0;
    	    int posVal = 0;

            for(int i = 0; i < N; i += posVal + 8){
                sentinel = view(i);
                posVal = fabs(sentinel);
                if(posVal == 0 || (i + posVal + 8) > N || view(i + posVal + 4) != sentinel)
                    return false;
                if(i + posVal + 7 == N - 1)
                    return true;
    	    }
            
            return false;}

        /**
         * O(1) in space
         * O(1) in time
         * <your documentation>
         */
        int& view (int i) {
            return *reinterpret_cast<int*>(&a[i]);}

    public:
        // ------------
        // constructors
        // ------------

        /**
         * O(1) in space
         * O(1) in time
         * throw a bad_alloc exception, if N is less than sizeof(T) + (2 * sizeof(int))
         */
        Allocator () {
            if(N < static_cast<int>(sizeof(T)) + 8){
                throw std::domain_error("Not enough space for allocater");
            }
            view(0) = N - 8;
            view(N - 4) = N - 8;
            assert(valid());}

        // Default copy, destructor, and copy assignment
        // Allocator  (const Allocator&);
        // ~Allocator ();
        // Allocator& operator = (const Allocator&);

        // --------
        // allocate
        // --------

        /**
         * O(1) in space
         * O(n) in time
         * after allocation there must be enough space left for a valid block
         * the smallest allowable block is sizeof(T) + (2 * sizeof(int))
         * choose the first block that fits
         * return 0, if allocation fails
         */
        pointer allocate (size_type n) {
    	    if(static_cast<int>(n) < 1)
                throw std::bad_alloc();
            else if(static_cast<int>(n) > N - 8){
                throw std::bad_alloc();
            }
    	    
    	    int posVal = 0;
    	    int aloBlocks = n * static_cast<int>(sizeof(T));
    	    assert(aloBlocks > 0);

    	    for(int i = 0; i < N; i += posVal + 8){
                int& front_sentinel = view(i);
                posVal = fabs(front_sentinel);
                int& back_sentinel = view(i + posVal + 4);
                if (front_sentinel == aloBlocks) {
                    front_sentinel = -front_sentinel;
                    back_sentinel = -back_sentinel;
                    assert(valid());
                    return reinterpret_cast<pointer>(a + i + 4);
                }else if(front_sentinel > 0 && front_sentinel >= static_cast<int>(sizeof(T)) + aloBlocks + 8){
                    front_sentinel = -aloBlocks;
                    view(i + aloBlocks + 4) = -aloBlocks;
                    view(i + aloBlocks + 8) = back_sentinel - aloBlocks - 8;
                    back_sentinel = back_sentinel - aloBlocks - 8;
                    assert(valid());
                    return reinterpret_cast<pointer>(a + i + 4);
                }else if (front_sentinel > 0 && front_sentinel >= aloBlocks) {
                    front_sentinel = -front_sentinel;
                    back_sentinel = -back_sentinel;
                    assert(valid());
                    return reinterpret_cast<pointer>(a + i + 4);}
    	    }
            assert(valid());
            throw std::bad_alloc();
            return 0;}

        // ---------
        // construct
        // ---------

        /**
         * O(1) in space
         * O(1) in time
         * <your documentation>
         */
        void construct (pointer p, const_reference v) {
            new (p) T(v);                               // this is correct and exempt
            assert(valid());}                           // from the prohibition of new

        // ----------
        // deallocate
        // ----------

        /**
         * O(1) in space
         * O(1) in time
         * after deallocation adjacent free blocks must be coalesced
         * <your documentation>
         */
        void deallocate (pointer p, size_type s = 0) {
            if(p == 0 || p == NULL){
                return;
            }else if(static_cast<int>(reinterpret_cast<char*>(p) - a) < 0){
                throw std::domain_error("Not a valid pointer");
            }else if(static_cast<int>(reinterpret_cast<char*>(p) - a) > N - 4){
                throw std::domain_error("Not a valid pointer");
            }

            if(N - 8 < static_cast<int>(s * static_cast<int>(sizeof(T)))){
                throw std::domain_error("Not a valid amount to deallocate");
            }else if(static_cast<int>(s) < 0){
                throw std::domain_error("Not a valid amount to deallocate");
            }

            int& front_sentinel = view(reinterpret_cast<char*>(p) - a - 4);
            int posVal = fabs(front_sentinel);
            int& back_sentinel = view(reinterpret_cast<char*>(p) - a + posVal);

            if(front_sentinel != back_sentinel){
                throw std::domain_error("Not a valid pointer");
            }else if(front_sentinel > 0){
                throw std::domain_error("Not a valid pointer");
            }

            char* front = (char*)&front_sentinel;
            char* back = (char*)&back_sentinel;
            
            if(front - 4 > a){
                int& prev_sentinel = view(front - a - 4);
                if(prev_sentinel > 0){
                    view(front - a - 8 - prev_sentinel) = posVal + prev_sentinel + 8;
                    back_sentinel = posVal + prev_sentinel + 8;
                    front_sentinel = 0;
                    prev_sentinel = 0;
                }else{
                    front_sentinel = posVal;
                    back_sentinel = posVal;
                }
            }else if(back + 4 < a + N){
                int& next_sentinel = view(back - a + 4);
                if(next_sentinel > 0){
                    view(back - a + 8 + next_sentinel) = posVal + next_sentinel + 8;
                    front_sentinel = posVal + next_sentinel + 8;
                    back_sentinel = 0;
                    next_sentinel = 0;
                }else{
                    front_sentinel = posVal;
                    back_sentinel = posVal;
                }
            }else if(back + 4 < a + N && front - 4 > a){
                int& next_sentinel = view(back - a + 4);
                int& prev_sentinel = view(front - a - 4);
                if(next_sentinel > 0 && prev_sentinel > 0){
                    view(front - a - 8 - prev_sentinel) = posVal + prev_sentinel + 8;
                    view(back - a + 8 + next_sentinel) = posVal + next_sentinel + 8;
                    next_sentinel = 0;
                    prev_sentinel = 0;
                    back_sentinel = 0;
                    front_sentinel = 0;
                }else if(prev_sentinel > 0){
                    view(front - a - 8 - prev_sentinel) = posVal + prev_sentinel + 8;
                    back_sentinel = posVal + prev_sentinel + 8;
                    front_sentinel = 0;
                    prev_sentinel = 0;
                }else if(next_sentinel > 0){
                    view(back - a + 8 + next_sentinel) = posVal + next_sentinel + 8;
                    front_sentinel = posVal + next_sentinel + 8;
                    back_sentinel = 0;
                    next_sentinel = 0;
                }else{
                    front_sentinel = posVal;
                    back_sentinel = posVal;
                }
            }else{
                front_sentinel = posVal;
                back_sentinel = posVal;
            }
            assert(valid());}

        // -------
        // destroy
        // -------

        /**
         * O(1) in space
         * O(1) in time
         * throw an invalid_argument exception, if pointer is invalid
         * <your documentation>
         */
        void destroy (pointer p) {
            p->~T();
            assert(valid());}

        /**
         * O(1) in space
         * O(1) in time
         * <your documentation>
         */
        const int& view (int i) const {
            return *reinterpret_cast<const int*>(&a[i]);}};

#endif // Allocator_h
