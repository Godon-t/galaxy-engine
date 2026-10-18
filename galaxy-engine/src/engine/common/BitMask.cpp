#include "BitMask.hpp"

namespace Galaxy
{
    BitMask::BitMask(int initialValue): mask(initialValue) {}

    void BitMask::setBit(unsigned int position, bool value){
        if(value)
            mask = mask | 1 << position;
        else
            mask = mask & ~(1 << position);
    }
    
    bool BitMask::overlap(const BitMask& left, const BitMask& right){
        return (left.mask & right.mask) != 0;
    }
} // namespace Galaxy


