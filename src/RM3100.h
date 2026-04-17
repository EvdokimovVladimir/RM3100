#ifndef RM3100_H
#define RM3100_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

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
    int32_t picoTesla;
};

struct RM3100MagField3
{
    RM3100AxisMeasurement x;
    RM3100AxisMeasurement y;
    RM3100AxisMeasurement z;
    uint8_t status;
    uint32_t magnitudePicoTesla;
    uint32_t timestampMs;
    bool success;
};

class RM3100Class
{
public:
    enum class BusType : uint8_t
    {
        None = 0,
        Spi = 1,
        I2C = 2,
    };

    enum class Axis : uint8_t
    {
        X = 0,
        Y = 1,
        Z = 2,
    };

    static constexpr uint8_t kNoPin = 0xFFu;

    static constexpr uint8_t kReadFlag = 0x80u;
    static constexpr uint8_t kWriteMask = 0x7Fu;

    static constexpr uint8_t kI2CBaseAddress = 0x20u;
    static constexpr uint8_t kI2CAddress00 = static_cast<uint8_t>(kI2CBaseAddress | 0x00u);
    static constexpr uint8_t kI2CAddress01 = static_cast<uint8_t>(kI2CBaseAddress | 0x01u);
    static constexpr uint8_t kI2CAddress10 = static_cast<uint8_t>(kI2CBaseAddress | 0x02u);
    static constexpr uint8_t kI2CAddress11 = static_cast<uint8_t>(kI2CBaseAddress | 0x03u);
    static constexpr uint8_t i2cAddress(uint8_t sa1, uint8_t sa0)
    {
        return static_cast<uint8_t>(kI2CBaseAddress | (((sa1) & 0x01u) << 1) | ((sa0) & 0x01u));
    }

    static constexpr uint8_t kRegisterPoll = 0x00u;
    static constexpr uint8_t kRegisterCmm = 0x01u;
    static constexpr uint8_t kRegisterCcx1 = 0x04u;
    static constexpr uint8_t kRegisterCcx0 = 0x05u;
    static constexpr uint8_t kRegisterCcy1 = 0x06u;
    static constexpr uint8_t kRegisterCcy0 = 0x07u;
    static constexpr uint8_t kRegisterCcz1 = 0x08u;
    static constexpr uint8_t kRegisterCcz0 = 0x09u;
    static constexpr uint8_t kRegisterTmrc = 0x0Bu;
    static constexpr uint8_t kRegisterMx2 = 0x24u;
    static constexpr uint8_t kRegisterMx1 = 0x25u;
    static constexpr uint8_t kRegisterMx0 = 0x26u;
    static constexpr uint8_t kRegisterMy2 = 0x27u;
    static constexpr uint8_t kRegisterMy1 = 0x28u;
    static constexpr uint8_t kRegisterMy0 = 0x29u;
    static constexpr uint8_t kRegisterMz2 = 0x2Au;
    static constexpr uint8_t kRegisterMz1 = 0x2Bu;
    static constexpr uint8_t kRegisterMz0 = 0x2Cu;
    static constexpr uint8_t kRegisterBist = 0x33u;
    static constexpr uint8_t kRegisterStatus = 0x34u;
    static constexpr uint8_t kRegisterHandshake = 0x35u;
    static constexpr uint8_t kRegisterRevision = 0x36u;

    static constexpr uint8_t kAxisX = 0x10u;
    static constexpr uint8_t kAxisY = 0x20u;
    static constexpr uint8_t kAxisZ = 0x40u;
    static constexpr uint8_t kAxisAll = static_cast<uint8_t>(kAxisX | kAxisY | kAxisZ);

    static constexpr uint8_t kPollMeasureX = kAxisX;
    static constexpr uint8_t kPollMeasureY = kAxisY;
    static constexpr uint8_t kPollMeasureZ = kAxisZ;

    static constexpr uint8_t kCmmStart = 0x01u;
    static constexpr uint8_t kCmmDrdyWhenAny = 0x00u;
    static constexpr uint8_t kCmmDrdyWhenAll = 0x08u;
    static constexpr uint8_t kCmmEnableX = kAxisX;
    static constexpr uint8_t kCmmEnableY = kAxisY;
    static constexpr uint8_t kCmmEnableZ = kAxisZ;

    static constexpr uint8_t kTmrc600Hz = 0x92u;
    static constexpr uint8_t kTmrc300Hz = 0x93u;
    static constexpr uint8_t kTmrc150Hz = 0x94u;
    static constexpr uint8_t kTmrc75Hz = 0x95u;
    static constexpr uint8_t kTmrc37Hz = 0x96u;
    static constexpr uint8_t kTmrc18Hz = 0x97u;
    static constexpr uint8_t kTmrc9Hz = 0x98u;
    static constexpr uint8_t kTmrc4Hz = 0x99u;
    static constexpr uint8_t kTmrc2Hz = 0x9Au;
    static constexpr uint8_t kTmrc1Hz = 0x9Bu;
    static constexpr uint8_t kTmrc0p6Hz = 0x9Cu;
    static constexpr uint8_t kTmrc0p3Hz = 0x9Du;
    static constexpr uint8_t kTmrc0p15Hz = 0x9Eu;
    static constexpr uint8_t kTmrc0p075Hz = 0x9Fu;

    static constexpr uint8_t kStatusDataReady = 0x80u;

    static constexpr uint8_t kBistSte = 0x80u;
    static constexpr uint8_t kBistZOk = 0x40u;
    static constexpr uint8_t kBistYOk = 0x20u;
    static constexpr uint8_t kBistXOk = 0x10u;
    static constexpr uint8_t kBistBw1 = 0x08u;
    static constexpr uint8_t kBistBw0 = 0x04u;
    static constexpr uint8_t kBistBp1 = 0x02u;
    static constexpr uint8_t kBistBp0 = 0x01u;

    static constexpr uint8_t kHandshakeNack2 = 0x40u;
    static constexpr uint8_t kHandshakeNack1 = 0x20u;
    static constexpr uint8_t kHandshakeNack0 = 0x10u;
    static constexpr uint8_t kHandshakeDrc1 = 0x02u;
    static constexpr uint8_t kHandshakeDrc0 = 0x01u;
    static constexpr uint8_t kHandshakeDefault = 0x1Bu;

    static constexpr uint8_t kRevisionUnknown = 0x00u;
    static constexpr uint16_t kDefaultCycleCount = 200u;
    static constexpr size_t kMeasurementBytes = 9u;

    RM3100Class();

    bool beginSPI(
        uint8_t csPin,
        uint8_t drdyPin = kNoPin,
        SPIClass &spi = SPI,
        const SPISettings &spiSettings = SPISettings(1000000, MSBFIRST, SPI_MODE3));

    bool beginI2C(
        uint8_t address = kI2CAddress00,
        uint8_t drdyPin = kNoPin,
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

    bool singleMeasurement(uint8_t axisMask = kAxisAll);
    bool startContinuous(uint8_t axisMask = kAxisAll, bool drdyWhenAllAxes = true);
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
    bool runBuiltInSelfTest(
        uint8_t axisMask = kAxisAll,
        uint8_t bistConfig = static_cast<uint8_t>(kBistSte | kBistBw0 | kBistBp0),
        uint32_t timeoutMs = 20UL,
        uint8_t *bistResult = nullptr);

    uint8_t readRevision();

    void clearCalibration();
    void clearCalibration(Axis axis);
    void setAxisCalibration(
        Axis axis,
        int64_t gainNumerator,
        uint32_t gainDenominator,
        int32_t offsetPicoTesla = 0,
        uint16_t referenceCycleCount = 0,
        bool scaleWithCycleCount = true,
        bool valid = true);
    void setCalibration(const RM3100Calibration &calibration);
    RM3100AxisCalibration axisCalibration(Axis axis) const;
    bool hasCalibration(Axis axis) const;
    int32_t applyCalibration(Axis axis, int32_t rawCounts) const;

    bool readMeasurementWords(uint32_t &mx, uint32_t &my, uint32_t &mz);
    bool read3(RM3100MagField3 &field, bool waitForReady = true, uint32_t timeoutMs = 0UL);
    RM3100MagField3 read3(bool waitForReady = true, uint32_t timeoutMs = 0UL);
    bool read3Once(RM3100MagField3 &field, uint8_t axisMask = kAxisAll, uint32_t timeoutMs = 0UL);

    static uint8_t axisMask(bool x, bool y, bool z);
    static uint32_t pack24(const uint8_t *data);
    static int32_t rawWordToSigned(uint32_t rawWord);
    static uint32_t magnitudeFromComponentsPicoTesla(int32_t bx, int32_t by, int32_t bz);

private:
    static uint8_t axisToIndex(Axis axis);
    static int32_t clampInt64ToInt32(int64_t value);
    static int64_t divideRounded(int64_t numerator, int64_t denominator);
    static uint32_t absInt32ToUint32(int32_t value);
    static uint32_t integerSqrtU64(uint64_t value);

    void configureDrdyPin();
    void configureI2CClock() const;
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