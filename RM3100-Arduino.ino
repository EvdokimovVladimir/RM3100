#include <Arduino.h>
#include "RM3100.h"

static const uint8_t PIN_CS = 10;
static const uint8_t PIN_DRDY = 9;
static const uint16_t CC_VALUE = 200;

static const RM3100AxisCalibration CAL_X = {0, 1, 0, CC_VALUE, true, false};
static const RM3100AxisCalibration CAL_Y = {0, 1, 0, CC_VALUE, true, false};
static const RM3100AxisCalibration CAL_Z = {0, 1, 0, CC_VALUE, true, false};

// ================= SETUP =================

void setup()
{
    Serial.begin(115200);
    RM3100.beginSPI(PIN_CS, PIN_DRDY);

    RM3100Calibration calibration = {CAL_X, CAL_Y, CAL_Z};
    RM3100.setCalibration(calibration);

    uint8_t id = RM3100.readRevision();
    Serial.print("REVID = 0x");
    Serial.println(id, HEX);

    RM3100.setCycleCountAll(CC_VALUE);

    uint16_t CycleCountsX = 0;
    uint16_t CycleCountsY = 0;
    uint16_t CycleCountsZ = 0;
    RM3100.readCycleCounts(CycleCountsX, CycleCountsY, CycleCountsZ);

    Serial.print("CC = ");
    Serial.print(CycleCountsX);
    Serial.print(",");
    Serial.print(CycleCountsY);
    Serial.print(",");
    Serial.println(CycleCountsZ);

    RM3100.setTMRC(RM3100_TMRC_75HZ);
    RM3100.startContinuous(RM3100_AXIS_ALL, true);

    Serial.println("---- START ----");
}

// ================= LOOP =================

void loop()
{
    MagField3 mf = RM3100.read3(true, 1000);
    if (!mf.success)
    {
        Serial.println("read3 timeout/error");
        return;
    }

    Serial.print("raw24=");
    Serial.print(mf.x.rawWord, HEX);
    Serial.print(",");
    Serial.print(mf.y.rawWord, HEX);
    Serial.print(",");
    Serial.print(mf.z.rawWord, HEX);
    Serial.print(" raw=");
    Serial.print(mf.x.raw);
    Serial.print(",");
    Serial.print(mf.y.raw);
    Serial.print(",");
    Serial.print(mf.z.raw);
    Serial.print(" pT=");
    Serial.print(mf.x.picoTesla);
    Serial.print(",");
    Serial.print(mf.y.picoTesla);
    Serial.print(",");
    Serial.println(mf.z.picoTesla);
}