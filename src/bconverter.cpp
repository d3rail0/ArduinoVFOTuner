#include "bconverter.h"

namespace bcv {

    // Reverse bits in a nibble
    unsigned char reverse(unsigned char b) {
        b = (b & 0b11001100) >> 2 | (b & 0b00110011) << 2;
        b = (b & 0b10101010) >> 1 | (b & 0b01010101) << 1;
        return b;
    }

    /*
        Converts 'num' to 22-bit BCD.
        
        First 20 LSB are BCD equivalent for last 5 digits in a number. 
        Every nibble has its bits reversed.
        
        4 MSB are also reversed and only 2 MSB are shifted to the right
        so that it fits the 22-bit output.
    */
    long mConvertToBCD(long num) {
        long res      = 0;
        long reversed = 0;

        uint8_t zerosOffset = 0; 

        // Count suffix zeros
        while(num && num%10==0) {
            zerosOffset++;
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

        while(zerosOffset--) 
            res <<= 4;
        
        // Shift 10 MHz bits to the right
        return res | (res & 0xC00000) >> 2;
    }

}