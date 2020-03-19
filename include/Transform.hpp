/*
Transform module
* Contains various functions for transforming pieces of compressed image data between different forms, and their helper functions

We will define two helper functions to convert between matrix indices and their corresponding zig-zag order:

* matIndicesToZZOrder: converts a matrix index, (i,j), to its corresponding order, k, in zig-zag ordering
* zzOrderToMatIndices: converts a zig-zag order, k, to it’s corresponding matrix index, (i,j)

We will also have two functions that will be used in the decoder

* bitStringToValue: convert a bit string representation to its corresponding value
* getValueCategory: get the category of a value
*/

#ifndef TRANSFORM_HPP // if TRANSFORM_HPP is defined, then the code inside #ifndef and #endif is taken for compilation
#define TRANSFORM_HPP

#include <string>
#include <utility>

#include "Types.hpp" // Importing the Types module which contains aliases and types

using namespace std;

namespace temp_peg {
    // Convert a zig-zag order index to its corresponding matrix indices
    // INPUT: zzIndex: the index in the zig-zag order
    // OUTPUT: returns the matrix indices corresponding to the zig-zag order
    const pair < const int, const int > zzOrderToMatIndices(const int zzindex);
    
    // Convert matrix indices to its corresponding zig-zag order index

    // INPUT: the parameters are the matrix indices
    // OUTPUT: returns the zig-zag index corresponding to the matrix indices
    const int matIndicesToZZOrder(const int row, const int column);

    // Convert a bit strig to it's corresponding value
    
    // INPUT: bitStr: the bit string
    // OUTPUT: returns the value corresponding to the bit string
    const Int16 bitStringtoValue(const string &bitStr);
    
    /// Get the category of a value
    
    // INPUT: value: the whose category has to be determined
    // OUTPUT: returns the category of the specified value
    const Int16 getValueCategory(const Int16 value);
}

#endif // TRANSFORM_HPP
