#define DRM3100_CALIBRATION RM3100_CALIBRATION_FLOAT
#include <RM3100.h>
#include <Wire.h>

namespace
{
constexpr uint8_t kI2CAddress = RM3100Class::kI2CAddress00;
constexpr uint32_t kI2CClockHz = 400000UL;
constexpr uint16_t kCycleCount = RM3100Class::kDefaultCycleCount;
constexpr uint32_t kReadTimeoutMs = 1000UL;
constexpr uint16_t kPrintPeriodMs = 100;
constexpr uint8_t kTmrcValue = RM3100Class::kTmrc75Hz;

constexpr float kGainPicoTeslaPerCount = 13333.333f;
const RM3100AxisCalibration kAxisCalibration = {
    kGainPicoTeslaPerCount,
    0,
    kCycleCount,
    true,
    true,
};

const RM3100Calibration kCalibration = {
    kAxisCalibration,
    kAxisCalibration,
    kAxisCalibration,
};

uint32_t lastPrintMs = 0;
}

void printCalibration(const RM3100AxisCalibration &calibration)
{
    Serial.print(F("gain="));
    Serial.print(calibration.gain, 3);
    Serial.print(F(", offset="));
    Serial.print(calibration.offsetPicoTesla);
    Serial.print(F(", refCC="));
    Serial.print(calibration.referenceCycleCount);
    Serial.print(F(", scaleWithCC="));
    Serial.print(calibration.scaleWithCycleCount);
    Serial.print(F(", valid="));
    Serial.println(calibration.valid);
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
    Serial.print(field.x.picoTesla);
    Serial.print(',');
    Serial.print(field.y.picoTesla);
    Serial.print(',');
    Serial.println(field.z.picoTesla);
}

void setupRM3100()
{
    Wire.begin();
    Wire.setClock(kI2CClockHz);

    delay(100);

    if (!RM3100.beginI2C(kI2CAddress, RM3100Class::kNoPin, Wire, kI2CClockHz))
    {
        Serial.println(F("RM3100 init failed"));
        while (1)
        {
        }
    }

    Serial.print(F("RM3100 REVID=0x"));
    Serial.println(RM3100.readRevision(), HEX);

    Serial.println(F("Calibration mode: float"));
    printCalibration(kAxisCalibration);

    RM3100.setCalibration(kCalibration);
    RM3100.writeHandshake(RM3100Class::kHandshakeDefault);

    if (!RM3100.setCycleCountAll(kCycleCount))
    {
        Serial.println(F("setCycleCountAll FAIL"));
        while (1)
        {
        }
    }

    if (!RM3100.setTMRC(kTmrcValue))
    {
        Serial.println(F("setTMRC FAIL"));
        while (1)
        {
        }
    }

    printCycleCounts();

    if (!RM3100.startContinuous(RM3100Class::kAxisAll, true))
    {
        Serial.println(F("Continuous FAIL"));
        while (1)
        {
        }
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000UL)
    {
    }
    delay(200);

    Serial.println(F("BOOT"));
    setupRM3100();
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
        Serial.print(F("Read error, status=0x"));
        Serial.println(RM3100.readStatus(), HEX);
        return;
    }

    printField(field);
}