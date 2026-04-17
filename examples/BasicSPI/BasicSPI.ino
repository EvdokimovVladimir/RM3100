#include <SPI.h>
#include <RM3100.h>

namespace
{
constexpr uint8_t kChipSelectPin = 10;
constexpr uint8_t kDataReadyPin = 9;
constexpr uint32_t kReadTimeoutMs = 1000UL;
}

void printField(const RM3100MagField3 &field)
{
    Serial.print(F("raw="));
    Serial.print(field.x.raw);
    Serial.print(',');
    Serial.print(field.y.raw);
    Serial.print(',');
    Serial.print(field.z.raw);
    Serial.print(F(" status=0x"));
    Serial.println(field.status, HEX);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000UL)
    {
    }

    SPI.begin();

    RM3100.beginSPI(kChipSelectPin, kDataReadyPin, SPI, SPISettings(1000000, MSBFIRST, SPI_MODE3));
    RM3100.setCycleCountAll(RM3100Class::kDefaultCycleCount);
    RM3100.setTMRC(RM3100Class::kTmrc75Hz);
    RM3100.startContinuous(RM3100Class::kAxisAll, true);

    Serial.print(F("RM3100 REVID: 0x"));
    Serial.println(RM3100.readRevision(), HEX);
}

void loop()
{
    const RM3100MagField3 field = RM3100.read3(true, kReadTimeoutMs);
    if (!field.success)
    {
        Serial.println(F("RM3100 read timeout"));
        delay(250);
        return;
    }

    printField(field);
    delay(100);
}