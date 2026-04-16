#ifndef RM3100_PRIVATE_H
#define RM3100_PRIVATE_H

#include "RM3100.h"

#define RM3100_I2C_READ_CHUNK_BYTES 28u
#define RM3100_I2C_WRITE_CHUNK_BYTES 24u

inline RM3100AxisMeasurement rm3100MakeAxisMeasurement(uint32_t rawWord, int32_t rawCounts, int32_t picoTesla)
{
    RM3100AxisMeasurement axis;
    axis.rawWord = rawWord;
    axis.raw = rawCounts;
    axis.picoTesla = picoTesla;
    return axis;
}

#endif