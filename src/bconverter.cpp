#include "bconverter.h"

namespace BCDConverter {

    // Reverse bits in a nibble
    unsigned char reverse(unsigned char b) {
        b = (b & 0b11001100) >> 2 | (b & 0b00110011) << 2;
        b = (b & 0b10101010) >> 1 | (b & 0b01010101) << 1;
        return b;
    }

    long ConvertToBCD_M(long num) {
        long res      = 0;
        long reversed = 0;

        uint8_t zeros_offset = 0; 

        // Count suffix zeros
        while(num && num%10==0) {
            zeros_offset++;
            num/=10;
        }

        // Reverse digits of num;
        while(num) {
            reversed = reversed*10 + num%10;
            num/=10;
        }

        // Decimal -> BCD
        while (reversed) {
            res <<= 4;
            res |= reverse(static_cast<unsigned char>(reversed % 10));
            reversed /= 10;
        }

        while(zeros_offset--) 
            res <<= 4;
        
        // Shift 10 MHz bits to the right
        return res | (res & 12582912) >> 2;
    }

}