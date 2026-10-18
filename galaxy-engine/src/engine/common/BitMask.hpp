#pragma once

namespace Galaxy
{
    struct BitMask{
        int mask;

        BitMask(int initialValue = 1);
        
        void setBit(unsigned int position, bool value);
        static bool overlap(const BitMask& left, const BitMask& right);
    };
} // namespace Galaxy
