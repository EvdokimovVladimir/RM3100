#include "RM3100_private.h"

uint32_t RM3100Class::pack24(const uint8_t *data)
{
    return (static_cast<uint32_t>(data[0]) << 16) |
           (static_cast<uint32_t>(data[1]) << 8) |
           static_cast<uint32_t>(data[2]);
}

int32_t RM3100Class::rawWordToSigned(uint32_t rawWord)
{
    int32_t value = static_cast<int32_t>(rawWord & 0x00FFFFFFUL);
    if ((value & 0x00800000L) != 0)
    {
        value |= static_cast<int32_t>(0xFF000000UL);
    }
    return value;
}

int32_t RM3100Class::clampInt64ToInt32(int64_t value)
{
    if (value > INT32_MAX)
    {
        return INT32_MAX;
    }
    if (value < INT32_MIN)
    {
        return INT32_MIN;
    }
    return static_cast<int32_t>(value);
}

int64_t RM3100Class::divideRounded(int64_t numerator, int64_t denominator)
{
    if (denominator == 0)
    {
        return 0;
    }

    const int64_t absDenominator = (denominator < 0) ? -denominator : denominator;
    int64_t bias = absDenominator / 2;

    if ((numerator < 0 && denominator > 0) || (numerator > 0 && denominator < 0))
    {
        bias = -bias;
    }

    return (numerator + bias) / denominator;
}

uint32_t RM3100Class::absInt32ToUint32(int32_t value)
{
    if (value >= 0)
    {
        return static_cast<uint32_t>(value);
    }

    if (value == INT32_MIN)
    {
        return 2147483648UL;
    }

    return static_cast<uint32_t>(-value);
}

uint32_t RM3100Class::integerSqrtU64(uint64_t value)
{
    uint64_t result = 0;
    uint64_t bit = 1ULL << 62;

    while (bit > value)
    {
        bit >>= 2;
    }

    while (bit != 0)
    {
        if (value >= result + bit)
        {
            value -= (result + bit);
            result = (result >> 1) + bit;
        }
        else
        {
            result >>= 1;
        }

        bit >>= 2;
    }

    return static_cast<uint32_t>(result);
}

uint32_t RM3100Class::magnitudeFromComponentsPicoTesla(int32_t bx, int32_t by, int32_t bz)
{
    const uint64_t x = static_cast<uint64_t>(absInt32ToUint32(bx));
    const uint64_t y = static_cast<uint64_t>(absInt32ToUint32(by));
    const uint64_t z = static_cast<uint64_t>(absInt32ToUint32(bz));

    const uint64_t sum = (x * x) + (y * y) + (z * z);
    return integerSqrtU64(sum);
}