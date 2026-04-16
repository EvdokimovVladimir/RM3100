#include "RM3100_private.h"

RM3100Class RM3100;

RM3100Class::RM3100Class()
    : _busType(BUS_NONE),
      _spi(0),
      _wire(0),
      _csPin(RM3100_NO_PIN),
      _drdyPin(RM3100_NO_PIN),
      _address(RM3100_I2C_ADDRESS_00),
      _i2cClockHz(400000UL),
      _spiSettings(1000000, MSBFIRST, SPI_MODE3)
{
    _cycleCounts[0] = RM3100_DEFAULT_CYCLE_COUNT;
    _cycleCounts[1] = RM3100_DEFAULT_CYCLE_COUNT;
    _cycleCounts[2] = RM3100_DEFAULT_CYCLE_COUNT;

    for (uint8_t i = 0; i < 3; ++i)
    {
        _calibration[i].gainNumerator = 0;
        _calibration[i].gainDenominator = 1;
        _calibration[i].offsetPicoTesla = 0;
        _calibration[i].referenceCycleCount = RM3100_DEFAULT_CYCLE_COUNT;
        _calibration[i].scaleWithCycleCount = true;
        _calibration[i].valid = false;
    }
}

bool RM3100Class::beginSPI(uint8_t csPin, uint8_t drdyPin, SPIClass &spi, const SPISettings &spiSettings)
{
    _busType = BUS_SPI;
    _spi = &spi;
    _wire = 0;
    _csPin = csPin;
    _drdyPin = drdyPin;
    _spiSettings = spiSettings;

    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    configureDrdyPin();

    _spi->begin();
    return true;
}

bool RM3100Class::beginI2C(uint8_t address, uint8_t drdyPin, TwoWire &wire, uint32_t clockHz)
{
    _busType = BUS_I2C;
    _spi = 0;
    _wire = &wire;
    _csPin = RM3100_NO_PIN;
    _drdyPin = drdyPin;
    _address = address;
    _i2cClockHz = clockHz;

    configureDrdyPin();

    _wire->begin();
    _wire->setClock(_i2cClockHz);
    return true;
}

void RM3100Class::end()
{
    if (_busType == BUS_SPI && _csPin != RM3100_NO_PIN)
    {
        digitalWrite(_csPin, HIGH);
    }

    _busType = BUS_NONE;
    _spi = 0;
    _wire = 0;
}

RM3100Class::BusType RM3100Class::busType() const
{
    return _busType;
}

uint8_t RM3100Class::address() const
{
    return _address;
}

uint8_t RM3100Class::drdyPin() const
{
    return _drdyPin;
}

uint16_t RM3100Class::cycleCount(Axis axis) const
{
    return _cycleCounts[static_cast<uint8_t>(axis)];
}

uint8_t RM3100Class::readRegister(uint8_t reg)
{
    uint8_t value = 0;
    readRegister(reg, value);
    return value;
}

bool RM3100Class::readRegister(uint8_t reg, uint8_t &value)
{
    return readRegisters(reg, &value, 1);
}

bool RM3100Class::writeRegister(uint8_t reg, uint8_t value)
{
    return writeRegisters(reg, &value, 1);
}

bool RM3100Class::readRegisters(uint8_t reg, uint8_t *data, size_t length)
{
    if (data == 0 || length == 0)
    {
        return false;
    }

    switch (_busType)
    {
    case BUS_SPI:
        return readRegistersSpi(reg, data, length);
    case BUS_I2C:
        return readRegistersI2C(reg, data, length);
    default:
        return false;
    }
}

bool RM3100Class::writeRegisters(uint8_t reg, const uint8_t *data, size_t length)
{
    if (data == 0 || length == 0)
    {
        return false;
    }

    bool ok = false;

    switch (_busType)
    {
    case BUS_SPI:
        ok = writeRegistersSpi(reg, data, length);
        break;
    case BUS_I2C:
        ok = writeRegistersI2C(reg, data, length);
        break;
    default:
        ok = false;
        break;
    }

    if (ok)
    {
        updateCachedRegisters(reg, data, length);
    }

    return ok;
}

bool RM3100Class::readCycleCounts(uint16_t &x, uint16_t &y, uint16_t &z)
{
    uint8_t buffer[6];
    if (!readRegisters(RM3100_CCX1, buffer, sizeof(buffer)))
    {
        return false;
    }

    x = static_cast<uint16_t>((static_cast<uint16_t>(buffer[0]) << 8) | buffer[1]);
    y = static_cast<uint16_t>((static_cast<uint16_t>(buffer[2]) << 8) | buffer[3]);
    z = static_cast<uint16_t>((static_cast<uint16_t>(buffer[4]) << 8) | buffer[5]);

    _cycleCounts[0] = x;
    _cycleCounts[1] = y;
    _cycleCounts[2] = z;
    return true;
}

bool RM3100Class::setCycleCounts(uint16_t x, uint16_t y, uint16_t z)
{
    uint8_t buffer[6];
    buffer[0] = static_cast<uint8_t>(x >> 8);
    buffer[1] = static_cast<uint8_t>(x & 0xFFu);
    buffer[2] = static_cast<uint8_t>(y >> 8);
    buffer[3] = static_cast<uint8_t>(y & 0xFFu);
    buffer[4] = static_cast<uint8_t>(z >> 8);
    buffer[5] = static_cast<uint8_t>(z & 0xFFu);
    return writeRegisters(RM3100_CCX1, buffer, sizeof(buffer));
}

bool RM3100Class::setCycleCountAll(uint16_t cycleCount)
{
    return setCycleCounts(cycleCount, cycleCount, cycleCount);
}

bool RM3100Class::singleMeasurement(uint8_t axisMask)
{
    uint8_t value = static_cast<uint8_t>(axisMask & RM3100_AXIS_ALL);
    return writeRegister(RM3100_POLL, value);
}

bool RM3100Class::startContinuous(uint8_t axisMask, bool drdyWhenAllAxes)
{
    uint8_t cmmValue = static_cast<uint8_t>((axisMask & RM3100_AXIS_ALL) | RM3100_CMM_START);
    if (drdyWhenAllAxes)
    {
        cmmValue = static_cast<uint8_t>(cmmValue | RM3100_CMM_DRDY_WHEN_ALL);
    }
    return writeRegister(RM3100_CMM, cmmValue);
}

bool RM3100Class::setContinuousModeRaw(uint8_t cmmValue)
{
    return writeRegister(RM3100_CMM, cmmValue);
}

bool RM3100Class::stopContinuous()
{
    return writeRegister(RM3100_CMM, 0x00u);
}

bool RM3100Class::setTMRC(uint8_t value)
{
    return writeRegister(RM3100_TMRC, value);
}

uint8_t RM3100Class::readTMRC()
{
    return readRegister(RM3100_TMRC);
}

uint8_t RM3100Class::readStatus()
{
    return readRegister(RM3100_STATUS);
}

bool RM3100Class::statusDataReady()
{
    return (readStatus() & RM3100_STATUS_DRDY) != 0u;
}

bool RM3100Class::pinDataReady() const
{
    if (_drdyPin == RM3100_NO_PIN)
    {
        return false;
    }

    return digitalRead(_drdyPin) == HIGH;
}

bool RM3100Class::dataReady()
{
    if (_drdyPin != RM3100_NO_PIN)
    {
        return pinDataReady();
    }

    return statusDataReady();
}

bool RM3100Class::waitForDataReady(uint32_t timeoutMs)
{
    const uint32_t start = millis();

    while (!dataReady())
    {
        if (timeoutMs != 0UL && (millis() - start) >= timeoutMs)
        {
            return false;
        }
        delay(1);
    }

    return true;
}

uint8_t RM3100Class::readHandshake()
{
    return readRegister(RM3100_HSHAKE);
}

bool RM3100Class::writeHandshake(uint8_t value)
{
    return writeRegister(RM3100_HSHAKE, value);
}

uint8_t RM3100Class::readBIST()
{
    return readRegister(RM3100_BIST);
}

bool RM3100Class::writeBIST(uint8_t value)
{
    return writeRegister(RM3100_BIST, value);
}

bool RM3100Class::runBuiltInSelfTest(uint8_t axisMask, uint8_t bistConfig, uint32_t timeoutMs, uint8_t *bistResult)
{
    uint8_t config = static_cast<uint8_t>(bistConfig | RM3100_BIST_STE);

    if (!writeBIST(config))
    {
        return false;
    }

    if (!singleMeasurement(axisMask))
    {
        return false;
    }

    if (!waitForDataReady(timeoutMs))
    {
        return false;
    }

    const uint8_t result = readBIST();
    if (bistResult != 0)
    {
        *bistResult = result;
    }

    writeBIST(static_cast<uint8_t>(config & ~RM3100_BIST_STE));

    return (result & axisMask) == (axisMask & RM3100_AXIS_ALL);
}

uint8_t RM3100Class::readRevision()
{
    return readRegister(RM3100_REVID);
}

void RM3100Class::clearCalibration()
{
    clearCalibration(AXIS_INDEX_X);
    clearCalibration(AXIS_INDEX_Y);
    clearCalibration(AXIS_INDEX_Z);
}

void RM3100Class::clearCalibration(Axis axis)
{
    const uint8_t index = static_cast<uint8_t>(axis);
    _calibration[index].gainNumerator = 0;
    _calibration[index].gainDenominator = 1;
    _calibration[index].offsetPicoTesla = 0;
    _calibration[index].referenceCycleCount = _cycleCounts[index];
    _calibration[index].scaleWithCycleCount = true;
    _calibration[index].valid = false;
}

void RM3100Class::setAxisCalibration(Axis axis, int64_t gainNumerator, uint32_t gainDenominator, int32_t offsetPicoTesla, uint16_t referenceCycleCount, bool scaleWithCycleCount, bool valid)
{
    const uint8_t index = static_cast<uint8_t>(axis);
    _calibration[index].gainNumerator = gainNumerator;
    _calibration[index].gainDenominator = (gainDenominator == 0u) ? 1u : gainDenominator;
    _calibration[index].offsetPicoTesla = offsetPicoTesla;
    _calibration[index].referenceCycleCount = (referenceCycleCount == 0u) ? _cycleCounts[index] : referenceCycleCount;
    _calibration[index].scaleWithCycleCount = scaleWithCycleCount;
    _calibration[index].valid = valid;
}

void RM3100Class::setCalibration(const RM3100Calibration &calibration)
{
    _calibration[0] = calibration.x;
    _calibration[1] = calibration.y;
    _calibration[2] = calibration.z;
}

RM3100AxisCalibration RM3100Class::axisCalibration(Axis axis) const
{
    return _calibration[static_cast<uint8_t>(axis)];
}

bool RM3100Class::hasCalibration(Axis axis) const
{
    return _calibration[static_cast<uint8_t>(axis)].valid;
}

int32_t RM3100Class::applyCalibration(Axis axis, int32_t rawCounts) const
{
    const uint8_t index = static_cast<uint8_t>(axis);
    const RM3100AxisCalibration &calibration = _calibration[index];

    if (!calibration.valid || calibration.gainDenominator == 0u)
    {
        return 0;
    }

    int64_t numerator = calibration.gainNumerator;
    int64_t denominator = calibration.gainDenominator;

    if (calibration.scaleWithCycleCount)
    {
        const uint16_t referenceCycleCount = (calibration.referenceCycleCount == 0u) ? _cycleCounts[index] : calibration.referenceCycleCount;
        const uint16_t currentCycleCount = (_cycleCounts[index] == 0u) ? 1u : _cycleCounts[index];
        numerator *= referenceCycleCount;
        denominator *= currentCycleCount;
    }

    int64_t picoTesla = divideRounded(static_cast<int64_t>(rawCounts) * numerator, denominator);
    picoTesla += calibration.offsetPicoTesla;

    return clampInt64ToInt32(picoTesla);
}

bool RM3100Class::readMeasurementWords(uint32_t &mx, uint32_t &my, uint32_t &mz)
{
    uint8_t buffer[RM3100_MEASUREMENT_BYTES];
    if (!readRegisters(RM3100_MX2, buffer, sizeof(buffer)))
    {
        return false;
    }

    mx = pack24(&buffer[0]);
    my = pack24(&buffer[3]);
    mz = pack24(&buffer[6]);
    return true;
}

bool RM3100Class::read3(MagField3 &field, bool waitForReady, uint32_t timeoutMs)
{
    field.status = 0;
    field.magnitudePicoTesla = 0;
    field.timestampMs = millis();
    field.success = false;
    field.x = rm3100MakeAxisMeasurement(0, 0, 0);
    field.y = rm3100MakeAxisMeasurement(0, 0, 0);
    field.z = rm3100MakeAxisMeasurement(0, 0, 0);

    if (waitForReady && !waitForDataReady(timeoutMs))
    {
        field.status = readStatus();
        return false;
    }

    uint32_t mx = 0;
    uint32_t my = 0;
    uint32_t mz = 0;

    if (!readMeasurementWords(mx, my, mz))
    {
        field.status = readStatus();
        return false;
    }

    const int32_t rawX = rawWordToSigned(mx);
    const int32_t rawY = rawWordToSigned(my);
    const int32_t rawZ = rawWordToSigned(mz);

    field.x = rm3100MakeAxisMeasurement(mx, rawX, applyCalibration(AXIS_INDEX_X, rawX));
    field.y = rm3100MakeAxisMeasurement(my, rawY, applyCalibration(AXIS_INDEX_Y, rawY));
    field.z = rm3100MakeAxisMeasurement(mz, rawZ, applyCalibration(AXIS_INDEX_Z, rawZ));
    field.magnitudePicoTesla = magnitudeFromComponentsPicoTesla(field.x.picoTesla, field.y.picoTesla, field.z.picoTesla);
    field.status = readStatus();
    field.timestampMs = millis();
    field.success = true;

    return true;
}

MagField3 RM3100Class::read3(bool waitForReady, uint32_t timeoutMs)
{
    MagField3 field;
    read3(field, waitForReady, timeoutMs);
    return field;
}

bool RM3100Class::read3Once(MagField3 &field, uint8_t axisMask, uint32_t timeoutMs)
{
    if (!singleMeasurement(axisMask))
    {
        return false;
    }

    return read3(field, true, timeoutMs);
}

uint8_t RM3100Class::axisMask(bool x, bool y, bool z)
{
    uint8_t mask = 0;
    if (x)
    {
        mask = static_cast<uint8_t>(mask | RM3100_AXIS_X);
    }
    if (y)
    {
        mask = static_cast<uint8_t>(mask | RM3100_AXIS_Y);
    }
    if (z)
    {
        mask = static_cast<uint8_t>(mask | RM3100_AXIS_Z);
    }
    return mask;
}

void RM3100Class::configureDrdyPin()
{
    if (_drdyPin != RM3100_NO_PIN)
    {
        pinMode(_drdyPin, INPUT);
    }
}

void RM3100Class::updateCachedRegister(uint8_t reg, uint8_t value)
{
    if (reg < RM3100_CCX1 || reg > RM3100_CCZ0)
    {
        return;
    }

    const uint8_t index = static_cast<uint8_t>((reg - RM3100_CCX1) / 2u);
    const bool isMsb = ((reg - RM3100_CCX1) % 2u) == 0u;
    uint8_t msb = static_cast<uint8_t>(_cycleCounts[index] >> 8);
    uint8_t lsb = static_cast<uint8_t>(_cycleCounts[index] & 0xFFu);

    if (isMsb)
    {
        msb = value;
    }
    else
    {
        lsb = value;
    }

    _cycleCounts[index] = static_cast<uint16_t>((static_cast<uint16_t>(msb) << 8) | lsb);
}

void RM3100Class::updateCachedRegisters(uint8_t reg, const uint8_t *data, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        updateCachedRegister(static_cast<uint8_t>(reg + i), data[i]);
    }
}