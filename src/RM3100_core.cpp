#include "RM3100_private.h"

RM3100Class RM3100;

RM3100Class::RM3100Class()
    : _busType(BusType::None),
      _spi(nullptr),
      _wire(nullptr),
      _csPin(kNoPin),
      _drdyPin(kNoPin),
      _address(kI2CAddress00),
      _i2cClockHz(400000UL),
      _spiSettings(1000000, MSBFIRST, SPI_MODE3)
{
    _cycleCounts[0] = kDefaultCycleCount;
    _cycleCounts[1] = kDefaultCycleCount;
    _cycleCounts[2] = kDefaultCycleCount;

    for (uint8_t index = 0; index < 3; ++index)
    {
        _calibration[index].gainNumerator = 0;
        _calibration[index].gainDenominator = 1;
        _calibration[index].offsetPicoTesla = 0;
        _calibration[index].referenceCycleCount = kDefaultCycleCount;
        _calibration[index].scaleWithCycleCount = true;
        _calibration[index].valid = false;
    }
}

bool RM3100Class::beginSPI(uint8_t csPin, uint8_t drdyPin, SPIClass &spi, const SPISettings &spiSettings)
{
    _busType = BusType::Spi;
    _spi = &spi;
    _wire = nullptr;
    _csPin = csPin;
    _drdyPin = drdyPin;
    _spiSettings = spiSettings;

    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    configureDrdyPin();

    return true;
}

bool RM3100Class::beginI2C(uint8_t address, uint8_t drdyPin, TwoWire &wire, uint32_t clockHz)
{
    _busType = BusType::I2C;
    _spi = nullptr;
    _wire = &wire;
    _csPin = kNoPin;
    _drdyPin = drdyPin;
    _address = address;
    _i2cClockHz = clockHz;

    configureDrdyPin();

    return true;
}

void RM3100Class::end()
{
    if (_busType == BusType::Spi && _csPin != kNoPin)
    {
        digitalWrite(_csPin, HIGH);
    }

    _busType = BusType::None;
    _spi = nullptr;
    _wire = nullptr;
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

uint8_t RM3100Class::axisToIndex(Axis axis)
{
    return static_cast<uint8_t>(axis);
}

uint16_t RM3100Class::cycleCount(Axis axis) const
{
    return _cycleCounts[axisToIndex(axis)];
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
    if (data == nullptr || length == 0u)
    {
        return false;
    }

    switch (_busType)
    {
    case BusType::Spi:
        return readRegistersSpi(reg, data, length);
    case BusType::I2C:
        return readRegistersI2C(reg, data, length);
    default:
        return false;
    }
}

bool RM3100Class::writeRegisters(uint8_t reg, const uint8_t *data, size_t length)
{
    if (data == nullptr || length == 0u)
    {
        return false;
    }

    bool ok = false;

    switch (_busType)
    {
    case BusType::Spi:
        ok = writeRegistersSpi(reg, data, length);
        break;
    case BusType::I2C:
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
    if (!readRegisters(kRegisterCcx1, buffer, sizeof(buffer)))
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
    return writeRegisters(kRegisterCcx1, buffer, sizeof(buffer));
}

bool RM3100Class::setCycleCountAll(uint16_t cycleCount)
{
    return setCycleCounts(cycleCount, cycleCount, cycleCount);
}

bool RM3100Class::singleMeasurement(uint8_t axisMask)
{
    const uint8_t value = static_cast<uint8_t>(axisMask & kAxisAll);
    return writeRegister(kRegisterPoll, value);
}

bool RM3100Class::startContinuous(uint8_t axisMask, bool drdyWhenAllAxes)
{
    uint8_t cmmValue = static_cast<uint8_t>((axisMask & kAxisAll) | kCmmStart);
    if (drdyWhenAllAxes)
    {
        cmmValue = static_cast<uint8_t>(cmmValue | kCmmDrdyWhenAll);
    }
    return writeRegister(kRegisterCmm, cmmValue);
}

bool RM3100Class::setContinuousModeRaw(uint8_t cmmValue)
{
    return writeRegister(kRegisterCmm, cmmValue);
}

bool RM3100Class::stopContinuous()
{
    return writeRegister(kRegisterCmm, 0x00u);
}

bool RM3100Class::setTMRC(uint8_t value)
{
    return writeRegister(kRegisterTmrc, value);
}

uint8_t RM3100Class::readTMRC()
{
    return readRegister(kRegisterTmrc);
}

uint8_t RM3100Class::readStatus()
{
    return readRegister(kRegisterStatus);
}

bool RM3100Class::statusDataReady()
{
    return (readStatus() & kStatusDataReady) != 0u;
}

bool RM3100Class::pinDataReady() const
{
    if (_drdyPin == kNoPin)
    {
        return false;
    }

    return digitalRead(_drdyPin) == HIGH;
}

bool RM3100Class::dataReady()
{
    if (_drdyPin != kNoPin)
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
    return readRegister(kRegisterHandshake);
}

bool RM3100Class::writeHandshake(uint8_t value)
{
    return writeRegister(kRegisterHandshake, value);
}

uint8_t RM3100Class::readBIST()
{
    return readRegister(kRegisterBist);
}

bool RM3100Class::writeBIST(uint8_t value)
{
    return writeRegister(kRegisterBist, value);
}

bool RM3100Class::runBuiltInSelfTest(uint8_t axisMask, uint8_t bistConfig, uint32_t timeoutMs, uint8_t *bistResult)
{
    const uint8_t expectedAxisMask = static_cast<uint8_t>(axisMask & kAxisAll);
    const uint8_t config = static_cast<uint8_t>(bistConfig | kBistSte);

    if (!writeBIST(config))
    {
        return false;
    }

    if (!singleMeasurement(expectedAxisMask))
    {
        return false;
    }

    if (!waitForDataReady(timeoutMs))
    {
        return false;
    }

    const uint8_t result = readBIST();
    if (bistResult != nullptr)
    {
        *bistResult = result;
    }

    writeBIST(static_cast<uint8_t>(config & ~kBistSte));

    return (result & expectedAxisMask) == expectedAxisMask;
}

uint8_t RM3100Class::readRevision()
{
    return readRegister(kRegisterRevision);
}

void RM3100Class::clearCalibration()
{
    clearCalibration(Axis::X);
    clearCalibration(Axis::Y);
    clearCalibration(Axis::Z);
}

void RM3100Class::clearCalibration(Axis axis)
{
    const uint8_t index = axisToIndex(axis);
    _calibration[index].gainNumerator = 0;
    _calibration[index].gainDenominator = 1;
    _calibration[index].offsetPicoTesla = 0;
    _calibration[index].referenceCycleCount = _cycleCounts[index];
    _calibration[index].scaleWithCycleCount = true;
    _calibration[index].valid = false;
}

void RM3100Class::setAxisCalibration(Axis axis, int64_t gainNumerator, uint32_t gainDenominator, int32_t offsetPicoTesla, uint16_t referenceCycleCount, bool scaleWithCycleCount, bool valid)
{
    const uint8_t index = axisToIndex(axis);
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
    return _calibration[axisToIndex(axis)];
}

bool RM3100Class::hasCalibration(Axis axis) const
{
    return _calibration[axisToIndex(axis)].valid;
}

int32_t RM3100Class::applyCalibration(Axis axis, int32_t rawCounts) const
{
    const uint8_t index = axisToIndex(axis);
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
    uint8_t buffer[kMeasurementBytes];
    if (!readRegisters(kRegisterMx2, buffer, sizeof(buffer)))
    {
        return false;
    }

    mx = pack24(&buffer[0]);
    my = pack24(&buffer[3]);
    mz = pack24(&buffer[6]);
    return true;
}

bool RM3100Class::read3(RM3100MagField3 &field, bool waitForReady, uint32_t timeoutMs)
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

    field.x = rm3100MakeAxisMeasurement(mx, rawX, applyCalibration(Axis::X, rawX));
    field.y = rm3100MakeAxisMeasurement(my, rawY, applyCalibration(Axis::Y, rawY));
    field.z = rm3100MakeAxisMeasurement(mz, rawZ, applyCalibration(Axis::Z, rawZ));
    field.magnitudePicoTesla = magnitudeFromComponentsPicoTesla(field.x.picoTesla, field.y.picoTesla, field.z.picoTesla);
    field.status = readStatus();
    field.timestampMs = millis();
    field.success = true;

    return true;
}

RM3100MagField3 RM3100Class::read3(bool waitForReady, uint32_t timeoutMs)
{
    RM3100MagField3 field;
    read3(field, waitForReady, timeoutMs);
    return field;
}

bool RM3100Class::read3Once(RM3100MagField3 &field, uint8_t axisMask, uint32_t timeoutMs)
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
        mask = static_cast<uint8_t>(mask | kAxisX);
    }
    if (y)
    {
        mask = static_cast<uint8_t>(mask | kAxisY);
    }
    if (z)
    {
        mask = static_cast<uint8_t>(mask | kAxisZ);
    }
    return mask;
}

void RM3100Class::configureDrdyPin()
{
    if (_drdyPin != kNoPin)
    {
        pinMode(_drdyPin, INPUT);
    }
}

void RM3100Class::configureI2CClock() const
{
    if (_wire != nullptr && _i2cClockHz != 0UL)
    {
        _wire->setClock(_i2cClockHz);
    }
}

void RM3100Class::updateCachedRegister(uint8_t reg, uint8_t value)
{
    if (reg < kRegisterCcx1 || reg > kRegisterCcz0)
    {
        return;
    }

    const uint8_t index = static_cast<uint8_t>((reg - kRegisterCcx1) / 2u);
    const bool isMsb = ((reg - kRegisterCcx1) % 2u) == 0u;
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
    for (size_t index = 0; index < length; ++index)
    {
        updateCachedRegister(static_cast<uint8_t>(reg + index), data[index]);
    }
}