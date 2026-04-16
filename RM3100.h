#ifndef RM3100_H
#define RM3100_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

/**
 * @brief Sentinel value for "pin is not connected".
 */
#define RM3100_NO_PIN 0xFFu

/** @brief SPI read bit (bit 7 set). */
#define RM3100_READ_FLAG 0x80u
/** @brief SPI write address mask (bit 7 cleared). */
#define RM3100_WRITE_MASK 0x7Fu

/** @brief Fixed upper 5 bits of 7-bit I2C address: 0b01000. */
#define RM3100_I2C_BASE_ADDRESS 0x20u
/** @brief Compose 7-bit I2C address from SA1 and SA0. */
#define RM3100_I2C_ADDRESS(sa1, sa0) (uint8_t)(RM3100_I2C_BASE_ADDRESS | (((sa1) & 0x01u) << 1) | ((sa0) & 0x01u))
/** @brief I2C address with SA1=0, SA0=0. */
#define RM3100_I2C_ADDRESS_00 RM3100_I2C_ADDRESS(0, 0)
/** @brief I2C address with SA1=0, SA0=1. */
#define RM3100_I2C_ADDRESS_01 RM3100_I2C_ADDRESS(0, 1)
/** @brief I2C address with SA1=1, SA0=0. */
#define RM3100_I2C_ADDRESS_10 RM3100_I2C_ADDRESS(1, 0)
/** @brief I2C address with SA1=1, SA0=1. */
#define RM3100_I2C_ADDRESS_11 RM3100_I2C_ADDRESS(1, 1)

/** @brief Register: POLL (single measurement command). */
#define RM3100_POLL 0x00u
/** @brief Register: CMM (continuous measurement mode). */
#define RM3100_CMM 0x01u
/** @brief Register: CCX MSB. */
#define RM3100_CCX1 0x04u
/** @brief Register: CCX LSB. */
#define RM3100_CCX0 0x05u
/** @brief Register: CCY MSB. */
#define RM3100_CCY1 0x06u
/** @brief Register: CCY LSB. */
#define RM3100_CCY0 0x07u
/** @brief Register: CCZ MSB. */
#define RM3100_CCZ1 0x08u
/** @brief Register: CCZ LSB. */
#define RM3100_CCZ0 0x09u
/** @brief Register: TMRC (continuous mode update interval). */
#define RM3100_TMRC 0x0Bu
/** @brief Register: MX2 (X result byte 2, MSB). */
#define RM3100_MX2 0x24u
/** @brief Register: MX1 (X result byte 1). */
#define RM3100_MX1 0x25u
/** @brief Register: MX0 (X result byte 0, LSB). */
#define RM3100_MX0 0x26u
/** @brief Register: MY2 (Y result byte 2, MSB). */
#define RM3100_MY2 0x27u
/** @brief Register: MY1 (Y result byte 1). */
#define RM3100_MY1 0x28u
/** @brief Register: MY0 (Y result byte 0, LSB). */
#define RM3100_MY0 0x29u
/** @brief Register: MZ2 (Z result byte 2, MSB). */
#define RM3100_MZ2 0x2Au
/** @brief Register: MZ1 (Z result byte 1). */
#define RM3100_MZ1 0x2Bu
/** @brief Register: MZ0 (Z result byte 0, LSB). */
#define RM3100_MZ0 0x2Cu
/** @brief Register: BIST. */
#define RM3100_BIST 0x33u
/** @brief Register: STATUS. */
#define RM3100_STATUS 0x34u
/** @brief Register: HSHAKE. */
#define RM3100_HSHAKE 0x35u
/** @brief Register: REVID. */
#define RM3100_REVID 0x36u

/** @brief Axis bit for X in POLL/CMM payloads. */
#define RM3100_AXIS_X 0x10u
/** @brief Axis bit for Y in POLL/CMM payloads. */
#define RM3100_AXIS_Y 0x20u
/** @brief Axis bit for Z in POLL/CMM payloads. */
#define RM3100_AXIS_Z 0x40u
/** @brief Axis mask for X+Y+Z. */
#define RM3100_AXIS_ALL (RM3100_AXIS_X | RM3100_AXIS_Y | RM3100_AXIS_Z)

/** @brief POLL bit for X axis. */
#define RM3100_POLL_PMX RM3100_AXIS_X
/** @brief POLL bit for Y axis. */
#define RM3100_POLL_PMY RM3100_AXIS_Y
/** @brief POLL bit for Z axis. */
#define RM3100_POLL_PMZ RM3100_AXIS_Z

/** @brief CMM START bit. */
#define RM3100_CMM_START 0x01u
/** @brief DRDY goes high after any enabled axis (DRDM=0). */
#define RM3100_CMM_DRDY_WHEN_ANY 0x00u
/** @brief DRDY goes high after all enabled axes (DRDM=1). */
#define RM3100_CMM_DRDY_WHEN_ALL 0x08u
/** @brief CMM axis enable X. */
#define RM3100_CMM_CMX RM3100_AXIS_X
/** @brief CMM axis enable Y. */
#define RM3100_CMM_CMY RM3100_AXIS_Y
/** @brief CMM axis enable Z. */
#define RM3100_CMM_CMZ RM3100_AXIS_Z

/** @brief TMRC presets from datasheet. */
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

/** @brief STATUS bit indicating fresh measurement data available. */
#define RM3100_STATUS_DRDY 0x80u

/** @brief BIST control and result bits. */
#define RM3100_BIST_STE 0x80u
#define RM3100_BIST_ZOK 0x40u
#define RM3100_BIST_YOK 0x20u
#define RM3100_BIST_XOK 0x10u
#define RM3100_BIST_BW1 0x08u
#define RM3100_BIST_BW0 0x04u
#define RM3100_BIST_BP1 0x02u
#define RM3100_BIST_BP0 0x01u

/** @brief HSHAKE control and status bits. */
#define RM3100_HSHAKE_NACK2 0x40u
#define RM3100_HSHAKE_NACK1 0x20u
#define RM3100_HSHAKE_NACK0 0x10u
#define RM3100_HSHAKE_DRC1 0x02u
#define RM3100_HSHAKE_DRC0 0x01u
#define RM3100_HSHAKE_DEFAULT 0x1Bu

/** @brief Unknown REVID placeholder. */
#define RM3100_REVID_UNKNOWN 0x00u
/** @brief Datasheet default cycle count after power-up (decimal 200). */
#define RM3100_DEFAULT_CYCLE_COUNT 200u
/** @brief Total bytes in one full X/Y/Z burst read. */
#define RM3100_MEASUREMENT_BYTES 9u

/**
 * @brief Fixed-point linear calibration parameters for one axis.
 *
 * Physical conversion uses only integer arithmetic:
 * M_i[pT] = gain_i * raw_i + offset_i,
 * where gain_i is encoded as gainNumerator / gainDenominator.
 *
 * If scaleWithCycleCount is true, gain is additionally scaled by
 * referenceCycleCount / currentCycleCount for this axis.
 */
struct RM3100AxisCalibration
{
    /** @brief Gain numerator in pT per count (fixed-point ratio numerator). */
    int64_t gainNumerator;
    /** @brief Gain denominator in pT per count (fixed-point ratio denominator). */
    uint32_t gainDenominator;
    /** @brief Additive offset in picoTesla. */
    int32_t offsetPicoTesla;
    /** @brief Cycle count value where gain was identified. */
    uint16_t referenceCycleCount;
    /** @brief Enable automatic gain scaling based on cycle count ratio. */
    bool scaleWithCycleCount;
    /** @brief Calibration validity flag. */
    bool valid;
};

/**
 * @brief Per-axis calibration container.
 */
struct RM3100Calibration
{
    /** @brief X axis calibration. */
    RM3100AxisCalibration x;
    /** @brief Y axis calibration. */
    RM3100AxisCalibration y;
    /** @brief Z axis calibration. */
    RM3100AxisCalibration z;
};

/**
 * @brief Per-axis measurement payload.
 */
struct RM3100AxisMeasurement
{
    /**
     * @brief Raw 24-bit register value packed into uint32_t.
     *
     * Layout: [23:16]=byte2, [15:8]=byte1, [7:0]=byte0.
     */
    uint32_t rawWord;
    /** @brief Signed 24-bit value sign-extended to int32_t. */
    int32_t raw;
    /**
     * @brief Calibrated field in pT encoded as two's-complement bits in uint32_t.
     *
     * This exists to satisfy use-cases where unsigned transport/storage is required.
     * For normal numeric calculations use picoTesla.
     */
    uint32_t picoTeslaUnsigned;
    /** @brief Calibrated field in pT as signed int32_t. */
    int32_t picoTesla;
};

/**
 * @brief Full 3-axis magnetic field sample.
 */
struct MagField3
{
    /** @brief X axis sample. */
    RM3100AxisMeasurement x;
    /** @brief Y axis sample. */
    RM3100AxisMeasurement y;
    /** @brief Z axis sample. */
    RM3100AxisMeasurement z;
    /** @brief STATUS register snapshot near the read operation. */
    uint8_t status;
    /** @brief Timestamp from millis() when sample structure was filled. */
    uint32_t timestampMs;
    /** @brief True when sample read and conversion completed successfully. */
    bool success;
};

/**
 * @brief RM3100 driver class with low-level register API and high-level helpers.
 */
class RM3100Class
{
public:
    /** @brief Active transport type. */
    enum BusType
    {
        BUS_NONE = 0,
        BUS_SPI = 1,
        BUS_I2C = 2,
    };

    /** @brief Internal axis index for per-axis arrays. */
    enum Axis
    {
        AXIS_INDEX_X = 0,
        AXIS_INDEX_Y = 1,
        AXIS_INDEX_Z = 2,
    };

    /** @brief Construct driver object with default state and no active bus. */
    RM3100Class();

    /**
     * @brief Initialize driver over SPI.
     * @param csPin Chip select pin.
     * @param drdyPin DRDY pin or RM3100_NO_PIN if unused.
     * @param spi SPI instance.
     * @param spiSettings SPI transaction settings.
     * @return true on successful setup.
     */
    bool beginSPI(
        uint8_t csPin,
        uint8_t drdyPin = RM3100_NO_PIN,
        SPIClass &spi = SPI,
        const SPISettings &spiSettings = SPISettings(1000000, MSBFIRST, SPI_MODE3));

    /**
     * @brief Initialize driver over I2C.
     * @param address 7-bit RM3100 address.
     * @param drdyPin DRDY pin or RM3100_NO_PIN if unused.
     * @param wire TwoWire instance.
     * @param clockHz I2C bus frequency.
     * @return true on successful setup.
     */
    bool beginI2C(
        uint8_t address = RM3100_I2C_ADDRESS_00,
        uint8_t drdyPin = RM3100_NO_PIN,
        TwoWire &wire = Wire,
        uint32_t clockHz = 400000UL);

    /** @brief Deinitialize active transport in driver state. */
    void end();

    /** @brief Get currently selected bus type. */
    BusType busType() const;
    /** @brief Get configured 7-bit I2C address. */
    uint8_t address() const;
    /** @brief Get configured DRDY pin number. */
    uint8_t drdyPin() const;
    /** @brief Get cached cycle count for selected axis. */
    uint16_t cycleCount(Axis axis) const;

    /** @brief Read one register byte (convenience overload). */
    uint8_t readRegister(uint8_t reg);
    /** @brief Read one register byte to output parameter. */
    bool readRegister(uint8_t reg, uint8_t &value);
    /** @brief Write one register byte. */
    bool writeRegister(uint8_t reg, uint8_t value);
    /** @brief Read a consecutive register block. */
    bool readRegisters(uint8_t reg, uint8_t *data, size_t length);
    /** @brief Write a consecutive register block. */
    bool writeRegisters(uint8_t reg, const uint8_t *data, size_t length);

    /** @brief Read CCX/CCY/CCZ from device and update local cache. */
    bool readCycleCounts(uint16_t &x, uint16_t &y, uint16_t &z);
    /** @brief Set CCX/CCY/CCZ in one burst write and update local cache. */
    bool setCycleCounts(uint16_t x, uint16_t y, uint16_t z);
    /** @brief Set same cycle count for all axes. */
    bool setCycleCountAll(uint16_t cycleCount);

    /** @brief Trigger single measurement on selected axes. */
    bool singleMeasurement(uint8_t axisMask = RM3100_AXIS_ALL);
    /** @brief Enable continuous mode on selected axes. */
    bool startContinuous(uint8_t axisMask = RM3100_AXIS_ALL, bool drdyWhenAllAxes = true);
    /** @brief Write raw CMM byte directly. */
    bool setContinuousModeRaw(uint8_t cmmValue);
    /** @brief Disable continuous mode. */
    bool stopContinuous();

    /** @brief Write TMRC register. */
    bool setTMRC(uint8_t value);
    /** @brief Read TMRC register. */
    uint8_t readTMRC();

    /** @brief Read STATUS register. */
    uint8_t readStatus();
    /** @brief Check DRDY via STATUS bit. */
    bool statusDataReady();
    /** @brief Check DRDY via configured pin. */
    bool pinDataReady() const;
    /** @brief Check data-ready using pin if configured, otherwise STATUS. */
    bool dataReady();
    /** @brief Block until data is ready or timeout expires (0 means no timeout). */
    bool waitForDataReady(uint32_t timeoutMs = 0UL);

    /** @brief Read HSHAKE register. */
    uint8_t readHandshake();
    /** @brief Write HSHAKE register. */
    bool writeHandshake(uint8_t value);

    /** @brief Read BIST register. */
    uint8_t readBIST();
    /** @brief Write BIST register. */
    bool writeBIST(uint8_t value);
    /**
     * @brief Run built-in self test and optionally return BIST byte.
     * @return true when all requested axis OK flags are set.
     */
    bool runBuiltInSelfTest(uint8_t axisMask = RM3100_AXIS_ALL, uint8_t bistConfig = (RM3100_BIST_STE | RM3100_BIST_BW0 | RM3100_BIST_BP0), uint32_t timeoutMs = 20UL, uint8_t *bistResult = 0);

    /** @brief Read REVID register. */
    uint8_t readRevision();

    /** @brief Clear calibration for all axes. */
    void clearCalibration();
    /** @brief Clear calibration for one axis. */
    void clearCalibration(Axis axis);
    /** @brief Set linear calibration for one axis in fixed-point form. */
    void setAxisCalibration(Axis axis, int64_t gainNumerator, uint32_t gainDenominator, int32_t offsetPicoTesla = 0, uint16_t referenceCycleCount = 0, bool scaleWithCycleCount = true, bool valid = true);
    /** @brief Set full 3-axis calibration container. */
    void setCalibration(const RM3100Calibration &calibration);
    /** @brief Get axis calibration copy. */
    RM3100AxisCalibration axisCalibration(Axis axis) const;
    /** @brief Check whether axis calibration is valid. */
    bool hasCalibration(Axis axis) const;
    /** @brief Apply axis calibration to raw signed counts and return pT. */
    int32_t applyCalibration(Axis axis, int32_t rawCounts) const;

    /** @brief Read raw MX/MY/MZ words as packed uint32_t values. */
    bool readMeasurementWords(uint32_t &mx, uint32_t &my, uint32_t &mz);
    /** @brief Read and convert 3-axis sample into provided object. */
    bool read3(MagField3 &field, bool waitForReady = true, uint32_t timeoutMs = 0UL);
    /** @brief Read and return 3-axis sample by value. */
    MagField3 read3(bool waitForReady = true, uint32_t timeoutMs = 0UL);
    /** @brief Trigger single-shot then read converted 3-axis sample. */
    bool read3Once(MagField3 &field, uint8_t axisMask = RM3100_AXIS_ALL, uint32_t timeoutMs = 0UL);

    /** @brief Build axis bitmask from booleans. */
    static uint8_t axisMask(bool x, bool y, bool z);
    /** @brief Pack three bytes [MSB..LSB] into uint32_t word. */
    static uint32_t pack24(const uint8_t *data);
    /** @brief Sign-extend packed 24-bit two's-complement word to int32_t. */
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