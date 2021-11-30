#include "Helper.h"

long Helper::ConvertToBCD(long num) {
    long res = 0;

    // Reverse digits in num;
    long reversed = 0;
    while(num) {
        reversed = reversed*10 + num%10;
        num/=10;
    }

    // Create a new number where each
    // nibble represents a digit of reversed
    short digit;
    while (reversed) {
        res <<= 4;
        res |= reversed % 10;
        reversed /= 10;
    }

    return res;
}

