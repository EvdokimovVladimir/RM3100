#include "RM3100_private.h"

bool RM3100Class::readRegistersI2C(uint8_t reg, uint8_t *data, size_t length)
{
    if (_wire == nullptr)
    {
        return false;
    }

    configureI2CClock();

    size_t offset = 0;
    while (offset < length)
    {
        const size_t chunk = ((length - offset) > RM3100_I2C_READ_CHUNK_BYTES)
                                 ? RM3100_I2C_READ_CHUNK_BYTES
                                 : (length - offset);

        _wire->beginTransmission(_address);
        _wire->write(static_cast<uint8_t>(reg + offset));
        if (_wire->endTransmission(false) != 0)
        {
            return false;
        }

        const size_t requested = chunk;
        const size_t received = static_cast<size_t>(_wire->requestFrom(static_cast<int>(_address), static_cast<int>(requested)));
        if (received != requested)
        {
            return false;
        }

        for (size_t index = 0; index < chunk; ++index)
        {
            if (!_wire->available())
            {
                return false;
            }
            data[offset + index] = static_cast<uint8_t>(_wire->read());
        }

        offset += chunk;
    }

    return true;
}

bool RM3100Class::writeRegistersI2C(uint8_t reg, const uint8_t *data, size_t length)
{
    if (_wire == nullptr)
    {
        return false;
    }

    configureI2CClock();

    size_t offset = 0;
    while (offset < length)
    {
        const size_t chunk = ((length - offset) > RM3100_I2C_WRITE_CHUNK_BYTES)
                                 ? RM3100_I2C_WRITE_CHUNK_BYTES
                                 : (length - offset);

        _wire->beginTransmission(_address);
        _wire->write(static_cast<uint8_t>(reg + offset));

        for (size_t index = 0; index < chunk; ++index)
        {
            _wire->write(data[offset + index]);
        }

        if (_wire->endTransmission(true) != 0)
        {
            return false;
        }

        offset += chunk;
    }

    return true;
}