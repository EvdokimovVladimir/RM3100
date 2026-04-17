#include "RM3100_private.h"

bool RM3100Class::readRegistersSpi(uint8_t reg, uint8_t *data, size_t length)
{
    if (_spi == nullptr || _csPin == kNoPin)
    {
        return false;
    }

    _spi->beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    _spi->transfer(static_cast<uint8_t>(reg | kReadFlag));

    for (size_t index = 0; index < length; ++index)
    {
        data[index] = _spi->transfer(0x00u);
    }

    digitalWrite(_csPin, HIGH);
    _spi->endTransaction();
    return true;
}

bool RM3100Class::writeRegistersSpi(uint8_t reg, const uint8_t *data, size_t length)
{
    if (_spi == nullptr || _csPin == kNoPin)
    {
        return false;
    }

    _spi->beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    _spi->transfer(static_cast<uint8_t>(reg & kWriteMask));

    for (size_t index = 0; index < length; ++index)
    {
        _spi->transfer(data[index]);
    }

    digitalWrite(_csPin, HIGH);
    _spi->endTransaction();
    return true;
}