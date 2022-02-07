#ifndef BCONVERTER_H
#define BCONVERTER_H

#include <stdint.h>

namespace BCDConverter {
    unsigned char reverse(unsigned char b);
    long ConvertToBCD_M(long num);
}

#endif