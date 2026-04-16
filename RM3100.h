#ifndef RM3100_H
#define RM3100_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#define RM3100_NO_PIN 0xFFu

#define RM3100_READ_FLAG 0x80u
#define RM3100_WRITE_MASK 0x7Fu

#define RM3100_I2C_BASE_ADDRESS 0x20u
#define RM3100_I2C_ADDRESS(sa1, sa0) (uint8_t)(RM3100_I2C_BASE_ADDRESS | (((sa1) & 0x01u) << 1) | ((sa0) & 0x01u))
#define RM3100_I2C_ADDRESS_00 RM3100_I2C_ADDRESS(0, 0)
#define RM3100_I2C_ADDRESS_01 RM3100_I2C_ADDRESS(0, 1)
#define RM3100_I2C_ADDRESS_10 RM3100_I2C_ADDRESS(1, 0)
#define RM3100_I2C_ADDRESS_11 RM3100_I2C_ADDRESS(1, 1)

#define RM3100_POLL 0x00u
#define RM3100_CMM 0x01u
#define RM3100_CCX1 0x04u
#define RM3100_CCX0 0x05u
#define RM3100_CCY1 0x06u
#define RM3100_CCY0 0x07u
#define RM3100_CCZ1 0x08u
#define RM3100_CCZ0 0x09u
#define RM3100_TMRC 0x0Bu
#define RM3100_MX2 0x24u
#define RM3100_MX1 0x25u
#define RM3100_MX0 0x26u
#define RM3100_MY2 0x27u
#define RM3100_MY1 0x28u
#define RM3100_MY0 0x29u
#define RM3100_MZ2 0x2Au
#define RM3100_MZ1 0x2Bu
#define RM3100_MZ0 0x2Cu
#define RM3100_BIST 0x33u
#define RM3100_STATUS 0x34u
#define RM3100_HSHAKE 0x35u
#define RM3100_REVID 0x36u

#define RM3100_AXIS_X 0x10u
#define RM3100_AXIS_Y 0x20u
#define RM3100_AXIS_Z 0x40u
#define RM3100_AXIS_ALL (RM3100_AXIS_X | RM3100_AXIS_Y | RM3100_AXIS_Z)

#define RM3100_POLL_PMX RM3100_AXIS_X
#define RM3100_POLL_PMY RM3100_AXIS_Y
#define RM3100_POLL_PMZ RM3100_AXIS_Z

#define RM3100_CMM_START 0x01u
#define RM3100_CMM_DRDY_WHEN_ANY 0x00u
#define RM3100_CMM_DRDY_WHEN_ALL 0x08u
#define RM3100_CMM_CMX RM3100_AXIS_X
#define RM3100_CMM_CMY RM3100_AXIS_Y
#define RM3100_CMM_CMZ RM3100_AXIS_Z

#define RM3100_TMRC_600HZ 0x92u
#define RM3100_TMRC_300HZ 0x93u
#define RM3100_TMRC_150HZ 0x94u
#define RM3100_TMRC_75HZ 0x95u
#define RM3100_TMRC_37HZ 0x96u
#define RM3100_TMRC_18HZ 0x97u
#define RM3100_TMRC_9HZ 0x98u
#define RM3100_TMRC_4HZ 0x99u
#define RM3100_TMRC_2HZ 0x9Au
#define RM3100_TMRC_1HZ 0x9Bu
#define RM3100_TMRC_0P6HZ 0x9Cu
#define RM3100_TMRC_0P3HZ 0x9Du
#define RM3100_TMRC_0P15HZ 0x9Eu
#define RM3100_TMRC_0P075HZ 0x9Fu

#define RM3100_STATUS_DRDY 0x80u

#define RM3100_BIST_STE 0x80u
#define RM3100_BIST_ZOK 0x40u
#define RM3100_BIST_YOK 0x20u
#define RM3100_BIST_XOK 0x10u
#define RM3100_BIST_BW1 0x08u
#define RM3100_BIST_BW0 0x04u
#define RM3100_BIST_BP1 0x02u
#define RM3100_BIST_BP0 0x01u

#define RM3100_HSHAKE_NACK2 0x40u
#define RM3100_HSHAKE_NACK1 0x20u
#define RM3100_HSHAKE_NACK0 0x10u
#define RM3100_HSHAKE_DRC1 0x02u
#define RM3100_HSHAKE_DRC0 0x01u
#define RM3100_HSHAKE_DEFAULT 0x1Bu

#define RM3100_REVID_UNKNOWN 0x00u
#define RM3100_DEFAULT_CYCLE_COUNT 200u
#define RM3100_MEASUREMENT_BYTES 9u

struct RM3100AxisCalibration
{
    int64_t gainNumerator;
    uint32_t gainDenominator;
    int32_t offsetPicoTesla;
    uint16_t referenceCycleCount;
    bool scaleWithCycleCount;
    bool valid;
};

struct RM3100Calibration
{
    RM3100AxisCalibration x;
    RM3100AxisCalibration y;
    RM3100AxisCalibration z;
};

struct RM3100AxisMeasurement
{
    uint32_t rawWord;
    int32_t raw;
    uint32_t picoTeslaUnsigned;
    int32_t picoTesla;
};

struct MagField3
{
    RM3100AxisMeasurement x;
    RM3100AxisMeasurement y;
    RM3100AxisMeasurement z;
    uint8_t status;
    uint32_t timestampMs;
    bool success;
};

class RM3100Class
{
public:
    enum BusType
    {
        BUS_NONE = 0,
        BUS_SPI = 1,
        BUS_I2C = 2,
    };

    enum Axis
    {
        AXIS_INDEX_X = 0,
        AXIS_INDEX_Y = 1,
        AXIS_INDEX_Z = 2,
    };

    RM3100Class();

    bool beginSPI(
        uint8_t csPin,
        uint8_t drdyPin = RM3100_NO_PIN,
        SPIClass &spi = SPI,
        const SPISettings &spiSettings = SPISettings(1000000, MSBFIRST, SPI_MODE3));

    bool beginI2C(
        uint8_t address = RM3100_I2C_ADDRESS_00,
        uint8_t drdyPin = RM3100_NO_PIN,
        TwoWire &wire = Wire,
        uint32_t clockHz = 400000UL);

    void end();

    BusType busType() const;
    uint8_t address() const;
    uint8_t drdyPin() const;
    uint16_t cycleCount(Axis axis) const;

    uint8_t readRegister(uint8_t reg);
    bool readRegister(uint8_t reg, uint8_t &value);
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t reg, uint8_t *data, size_t length);
    bool writeRegisters(uint8_t reg, const uint8_t *data, size_t length);

    bool readCycleCounts(uint16_t &x, uint16_t &y, uint16_t &z);
    bool setCycleCounts(uint16_t x, uint16_t y, uint16_t z);
    bool setCycleCountAll(uint16_t cycleCount);

    bool singleMeasurement(uint8_t axisMask = RM3100_AXIS_ALL);
    bool startContinuous(uint8_t axisMask = RM3100_AXIS_ALL, bool drdyWhenAllAxes = true);
    bool setContinuousModeRaw(uint8_t cmmValue);
    bool stopContinuous();

    bool setTMRC(uint8_t value);
    uint8_t readTMRC();

    uint8_t readStatus();
    bool statusDataReady();
    bool pinDataReady() const;
    bool dataReady();
    bool waitForDataReady(uint32_t timeoutMs = 0UL);

    uint8_t readHandshake();
    bool writeHandshake(uint8_t value);

    uint8_t readBIST();
    bool writeBIST(uint8_t value);
    bool runBuiltInSelfTest(uint8_t axisMask = RM3100_AXIS_ALL, uint8_t bistConfig = (RM3100_BIST_STE | RM3100_BIST_BW0 | RM3100_BIST_BP0), uint32_t timeoutMs = 20UL, uint8_t *bistResult = 0);

    uint8_t readRevision();

    void clearCalibration();
    void clearCalibration(Axis axis);
    void setAxisCalibration(Axis axis, int64_t gainNumerator, uint32_t gainDenominator, int32_t offsetPicoTesla = 0, uint16_t referenceCycleCount = 0, bool scaleWithCycleCount = true, bool valid = true);
    void setCalibration(const RM3100Calibration &calibration);
    RM3100AxisCalibration axisCalibration(Axis axis) const;
    bool hasCalibration(Axis axis) const;
    int32_t applyCalibration(Axis axis, int32_t rawCounts) const;

    bool readMeasurementWords(uint32_t &mx, uint32_t &my, uint32_t &mz);
    bool read3(MagField3 &field, bool waitForReady = true, uint32_t timeoutMs = 0UL);
    MagField3 read3(bool waitForReady = true, uint32_t timeoutMs = 0UL);
    bool read3Once(MagField3 &field, uint8_t axisMask = RM3100_AXIS_ALL, uint32_t timeoutMs = 0UL);

    static uint8_t axisMask(bool x, bool y, bool z);
    static uint32_t pack24(const uint8_t *data);
    static int32_t rawWordToSigned(uint32_t rawWord);

private:
    static int32_t clampInt64ToInt32(int64_t value);
    static int64_t divideRounded(int64_t numerator, int64_t denominator);

    void configureDrdyPin();
    void updateCachedRegister(uint8_t reg, uint8_t value);
    void updateCachedRegisters(uint8_t reg, const uint8_t *data, size_t length);

    bool readRegistersSpi(uint8_t reg, uint8_t *data, size_t length);
    bool writeRegistersSpi(uint8_t reg, const uint8_t *data, size_t length);
    bool readRegistersI2C(uint8_t reg, uint8_t *data, size_t length);
    bool writeRegistersI2C(uint8_t reg, const uint8_t *data, size_t length);

    BusType _busType;
    SPIClass *_spi;
    TwoWire *_wire;
    uint8_t _csPin;
    uint8_t _drdyPin;
    uint8_t _address;
    uint32_t _i2cClockHz;
    SPISettings _spiSettings;
    uint16_t _cycleCounts[3];
    RM3100AxisCalibration _calibration[3];
};

extern RM3100Class RM3100;

#endif