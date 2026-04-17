#include <RM3100.h>
#include <SPI.h>
#include <Wire.h>

#define RM3100_TRANSPORT_SPI 1

namespace
{
constexpr uint8_t kChipSelectPin = 10;
constexpr uint8_t kDataReadyPin = 9;
constexpr uint8_t kI2CAddress = RM3100Class::kI2CAddress00;
constexpr uint32_t kI2CClockHz = 400000UL;
constexpr uint16_t kCycleCount = RM3100Class::kDefaultCycleCount;
constexpr uint32_t kReadTimeoutMs = 1000UL;
constexpr uint32_t kPrintPeriodMs = 250UL;

const RM3100AxisCalibration kCalibrationDisabled = {0, 1, 0, kCycleCount, true, false};
const RM3100Calibration kDefaultCalibration = {
    kCalibrationDisabled,
    kCalibrationDisabled,
    kCalibrationDisabled,
};

uint32_t lastPrintMs = 0;
}

void printCycleCounts()
{
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t z = 0;

    if (!RM3100.readCycleCounts(x, y, z))
    {
        Serial.println(F("Не удалось прочитать CCX/CCY/CCZ"));
        return;
    }

    Serial.print(F("Cycle counts: "));
    Serial.print(x);
    Serial.print(',');
    Serial.print(y);
    Serial.print(',');
    Serial.println(z);
}

void printField(const RM3100MagField3 &field)
{
    Serial.print(F("raw24=0x"));
    Serial.print(field.x.rawWord, HEX);
    Serial.print(F(",0x"));
    Serial.print(field.y.rawWord, HEX);
    Serial.print(F(",0x"));
    Serial.print(field.z.rawWord, HEX);

    Serial.print(F(" raw="));
    Serial.print(field.x.raw);
    Serial.print(',');
    Serial.print(field.y.raw);
    Serial.print(',');
    Serial.print(field.z.raw);

    Serial.print(F(" pT="));
    Serial.print(field.x.picoTesla);
    Serial.print(',');
    Serial.print(field.y.picoTesla);
    Serial.print(',');
    Serial.print(field.z.picoTesla);

    Serial.print(F(" |B|="));
    Serial.print(field.magnitudePicoTesla);
    Serial.print(F(" status=0x"));
    Serial.println(field.status, HEX);
}

void configureTransport()
{
#if RM3100_TRANSPORT_SPI
    Serial.println(F("Транспорт: SPI"));
    SPI.begin();
    RM3100.beginSPI(kChipSelectPin, kDataReadyPin, SPI, SPISettings(1000000, MSBFIRST, SPI_MODE3));
#else
    Serial.println(F("Транспорт: I2C"));
    Wire.begin();
    RM3100.beginI2C(kI2CAddress, kDataReadyPin, Wire, kI2CClockHz);
#endif
}

void runSelfTest()
{
    uint8_t bistResult = 0;
    const bool passed = RM3100.runBuiltInSelfTest(
        RM3100Class::kAxisAll,
        static_cast<uint8_t>(RM3100Class::kBistBw0 | RM3100Class::kBistBp0),
        20UL,
        &bistResult);

    Serial.print(F("BIST: "));
    Serial.print(passed ? F("OK") : F("FAIL"));
    Serial.print(F(", result=0x"));
    Serial.println(bistResult, HEX);
}

void setupSensor()
{
    RM3100.setCalibration(kDefaultCalibration);
    RM3100.writeHandshake(RM3100Class::kHandshakeDefault);
    RM3100.setCycleCountAll(kCycleCount);
    RM3100.setTMRC(RM3100Class::kTmrc75Hz);

    Serial.print(F("REVID: 0x"));
    Serial.println(RM3100.readRevision(), HEX);
    printCycleCounts();
    runSelfTest();

    if (!RM3100.startContinuous(RM3100Class::kAxisAll, true))
    {
        Serial.println(F("Не удалось включить continuous mode"));
    }
}

void printSingleShotExample()
{
    RM3100MagField3 field;
    if (!RM3100.read3Once(field, RM3100Class::axisMask(true, true, true), kReadTimeoutMs))
    {
        Serial.println(F("Single-shot измерение не удалось"));
        return;
    }

    Serial.println(F("Single-shot чтение:"));
    printField(field);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000UL)
    {
    }

    Serial.println(F("RM3100 full demo"));
    Serial.println(F("Калибровка по умолчанию отключена, поэтому picoTesla = 0 до загрузки реальных коэффициентов."));

    configureTransport();
    setupSensor();
    printSingleShotExample();
}

void loop()
{
    const uint32_t now = millis();
    if ((now - lastPrintMs) < kPrintPeriodMs)
    {
        return;
    }
    lastPrintMs = now;

    const RM3100MagField3 field = RM3100.read3(true, kReadTimeoutMs);
    if (!field.success)
    {
        Serial.println(F("Ошибка чтения continuous mode"));
        return;
    }

    printField(field);
}