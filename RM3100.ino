#include <Arduino.h>
#include <SPI.h>

#define DIGITS 3

// --- pins ---
#define PIN_CS   10
#define PIN_DRDY 9

// --- registers ---
#define REG_REVID  0x36
#define REG_STATUS 0x34
#define REG_CMM    0x01
#define REG_CCX1   0x04
#define REG_MX2    0x24
#define REG_TMRC   0x0B

#define CC_VALUE 200

// !!! ключевой момент
SPISettings spiSettings(1000000, MSBFIRST, SPI_MODE3);

float gain;

// ================= LOW LEVEL =================

uint8_t readReg(uint8_t reg)
{
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(reg | 0x80);
    uint8_t val = SPI.transfer(0x00);
    digitalWrite(PIN_CS, HIGH);
    return val;
}

void writeReg(uint8_t reg, uint8_t val)
{
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(reg & 0x7F);
    SPI.transfer(val);
    digitalWrite(PIN_CS, HIGH);
}

// ================= CONFIG =================

void setCycleCount(uint16_t cc)
{
    digitalWrite(PIN_CS, LOW);

    SPI.transfer(REG_CCX1);

    for (int i = 0; i < 3; i++)
    {
        SPI.transfer(cc >> 8);
        SPI.transfer(cc & 0xFF);
    }

    digitalWrite(PIN_CS, HIGH);
}

// ================= READ =================

void readRawBurst(uint8_t *buf)
{
    digitalWrite(PIN_CS, LOW);

    SPI.transfer(REG_MX2 | 0x80);

    for (int i = 0; i < 9; i++)
        buf[i] = SPI.transfer(0x00);

    digitalWrite(PIN_CS, HIGH);
}

int32_t convert24(uint8_t b2, uint8_t b1, uint8_t b0)
{
    int32_t v = ((int32_t)b2 << 16) | ((int32_t)b1 << 8) | b0;
    if (v & 0x800000) v |= 0xFF000000;
    return v;
}

// ================= SETUP =================

void setup()
{
    pinMode(PIN_CS, OUTPUT);
    digitalWrite(PIN_CS, HIGH);

    pinMode(PIN_DRDY, INPUT);

    Serial.begin(115200);
    SPI.begin();

    SPI.beginTransaction(spiSettings);

    // --- REVID ---
    uint8_t id = readReg(REG_REVID);
    Serial.print("REVID = 0x");
    Serial.println(id, HEX);

    // --- CC ---
    setCycleCount(CC_VALUE);

    delay(10);

    uint16_t cc_read =
        ((uint16_t)readReg(0x04) << 8) |
         readReg(0x05);

    Serial.print("CC read = ");
    Serial.println(cc_read);

    // --- gain ---
    gain = 0.3671f * CC_VALUE + 1.5f;
    Serial.print("Gain = ");
    Serial.println(gain);

    // --- continuous ---
    writeReg(REG_TMRC, 0x95);
    writeReg(REG_CMM, 0x79);
    
    SPI.endTransaction();

    Serial.println("---- START ----");
}

// ================= LOOP =================

void loop()
{
    SPI.beginTransaction(spiSettings);

    // ожидание данных
    while (!(readReg(REG_STATUS) & 0x80));

    uint8_t buf[9];
    readRawBurst(buf);

    SPI.endTransaction();

    // --- dump байтов ---
    // Serial.print("RAW BYTES: ");
    // for (int i = 0; i < 9; i++)
    // {
    //     Serial.print(buf[i], HEX);
    //     Serial.print(" ");
    // }
    // // Serial.println();
    // Serial.print(".");

    // --- конверсия ---
    int32_t x = convert24(buf[0], buf[1], buf[2]);
    int32_t y = convert24(buf[3], buf[4], buf[5]);
    int32_t z = convert24(buf[6], buf[7], buf[8]);

    // Serial.print("RAW: ");
    // Serial.print(x); Serial.print(" ");
    // Serial.print(y); Serial.print(" ");
    // Serial.println(z);

    float fx = x / gain;
    float fy = y / gain;
    float fz = z / gain;

    float mag = sqrt(fx*fx + fy*fy + fz*fz);

    // Serial.print("uT: ");
    Serial.print(fx, DIGITS); Serial.print(",");
    Serial.print(fy, DIGITS); Serial.print(",");
    Serial.print(fz, DIGITS); Serial.print(",");

    Serial.println(mag, DIGITS);

    // Serial.println();

    // delay(100);
}