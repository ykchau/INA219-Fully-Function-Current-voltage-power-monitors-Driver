#ifndef INA219_H_
#define INA219_H_

#include <Arduino.h>
#include <Wire.h>

#define INA219_I2C_ADDRESS  0x40

// Register Address
#define INA219_REG_CONFIG       0x00    // R/W
#define INA219_REG_SHUNT_VOLT   0x01    // R
#define INA219_REG_BUS_VOLT     0x02    // R
#define INA219_REG_POWER        0x03    // R
#define INA219_REG_CURRENT      0x04    // R
#define INA219_REG_CALIBRATION  0x05    // R/W

// Default Value
#define INA219_DEFAULT_CONFIG       0x399F
#define INA219_DEFAULT_CALIBATION   0x0000

// Configuration Settings Flags
// bit 15
#define INA219_RST                  0x8000  // Resets all registers to default values; this bit self-clears.
// bit 13
#define INA219_BRNG_16_FSR          0x0000  // 0 = 16V FSR, Full Scale Range
#define INA219_BRNG_32_FSR          0x2000  // 1 = 32V FSR, Full Scale Range
// bit 12, 11
#define INA219_PGA_40mV             0x0000  // Gain = /1, Range +/- 40mV
#define INA219_PGA_80mV             0x1000  // Gain = /2, Range +/- 80mV
#define INA219_PGA_160mV            0x2000  // Gain = /4, Range +/- 160mV
#define INA219_PGA_320mV            0x3000  // Gain = /8, Range +/- 320mV (default)
// bit 10 ~ 7 (BADC, Bus ADC)
#define INA219_BADC_MODE_9_BIT      0x0000  // 9 bit, conversion time 84us
#define INA219_BADC_MODE_10_BIT     0x0080  // 10 bit, conversion time 148us
#define INA219_BADC_MODE_11_BIT     0x0100  // 11 bit, conversion time 276us
#define INA219_BADC_MODE_12_BIT     0x0180  // 12 bit, conversion time 532us (default)
#define INA219_BADC_SAMPLE_1        0x0400  // 12 bit, conversion time 532us
#define INA219_BADC_SAMPLE_2        0x0480  // 12 bit, conversion time 1.06ms
#define INA219_BADC_SAMPLE_4        0x0500  // 12 bit, conversion time 2.13ms
#define INA219_BADC_SAMPLE_8        0x0580  // 12 bit, conversion time 4.26ms
#define INA219_BADC_SAMPLE_16       0x0600  // 12 bit, conversion time 8.51ms
#define INA219_BADC_SAMPLE_32       0x0680  // 12 bit, conversion time 17.02ms
#define INA219_BADC_SAMPLE_64       0x0700  // 12 bit, conversion time 34.05ms
#define INA219_BADC_SAMPLE_128      0x0780  // 12 bit, conversion time 68.10ms
// bit 6 ~ 3 (SADC, Shunt ADC)
#define INA219_SADC_MODE_9_BIT      0x0000  // 9 bit, conversion time 84us
#define INA219_SADC_MODE_10_BIT     0x0008  // 10 bit, conversion time 148us
#define INA219_SADC_MODE_11_BIT     0x0010  // 11 bit, conversion time 276us
#define INA219_SADC_MODE_12_BIT     0x0018  // 12 bit, conversion time 532us (default)
#define INA219_SADC_SAMPLE_1        0x0040  // 12 bit, conversion time 532us
#define INA219_SADC_SAMPLE_2        0x0048  // 12 bit, conversion time 1.06ms
#define INA219_SADC_SAMPLE_4        0x0050  // 12 bit, conversion time 2.13ms
#define INA219_SADC_SAMPLE_8        0x0058  // 12 bit, conversion time 4.26ms
#define INA219_SADC_SAMPLE_16       0x0060  // 12 bit, conversion time 8.51ms
#define INA219_SADC_SAMPLE_32       0x0068  // 12 bit, conversion time 17.02ms
#define INA219_SADC_SAMPLE_64       0x0070  // 12 bit, conversion time 34.05ms
#define INA219_SADC_SAMPLE_128      0x0078  // 12 bit, conversion time 68.10ms
// bit 0 ~ 2, Operation Mode
#define INA219_POWER_DOWN           0x0000  // Power-down
#define INA219_S_VOLT_TRIG          0x0001  // Shunt voltage, triggered
#define INA219_B_VOLT_TRIG          0x0002  // Bus voltage, triggered
#define INA219_SB_TRIG              0x0003  // Shunt and bus, triggered
#define INA219_ADC_OFF              0x0004  // ADC off (disabled)
#define INA219_S_VOLT_CONT          0x0005  // Shunt voltage, continuous
#define INA219_B_BOLT_CONT          0x0006  // Bus voltage, continuous
#define INA219_SB_CONT              0x0007  // Shunt and bus, continuous (default)

// Calculation
#define INA219_SHUNT_OHM    0.1         // Max Shunt resistance, ohm
#define INA219_2_POW_15     32768.0     // 2^15

typedef struct Settings {
    int BRNG = INA219_BRNG_32_FSR;
    int PGA = INA219_PGA_320mV;
    int BADC = INA219_BADC_MODE_12_BIT;
    int SADC = INA219_SADC_MODE_12_BIT;
    int MODE = INA219_SB_CONT;
};

typedef struct Measuremet {
    float power = 0.0;              // W, LSB = powerLSB mW (Current Register * Bus Voltage Register) / 5000
    float current = 0.0;            // A, LSB = currentLSB uA (Shunt Voltage Register * Calibration Register) / 4096
    float busVoltage = 0.0;         // mV, LSB = 4mV
    float shuntVoltage = 0.0;       // uV, LSB = 10uV
};

typedef struct LSB {
    byte busVoltage = 4;            // mV
    byte shuntVoltage = 10;         // uV
    float power = 0.0;              // mW
    float current = 0.0;            // uA
};

typedef struct Calculations {
    float MaxPossibleCurrent = 0.0;             // A
    float MaxCurrent = 0.0;                     // A
    float MaxCurrentBeforeOverflow = 0.0;       // A
    float MaxShuntVoltage = 0.0;                // V
    float MaxShuntVoltageBeforeOverflow = 0.0;  // V
    float MaxPower = 0.0;                       // W

    LSB lsb;
    unsigned int calibrationValue = 0;
};


class INA219 {
    private:
        uint16_t I2C_Read(byte addr);
        void I2C_Write(byte addr, uint16_t data);

        // get raw data
        float getRawShuntVoltage();
        float getRawBusVoltage();
        float getRawPower();
        float getRawCurrent();
    public:
        Settings settings;
        Measuremet measurement;
        Calculations calculations;
        byte busVoltageOverflow = 0;            // If overflow occur ( = 1), current/power reading may be meaningless 
        byte busVoltageConversionReady = 0;

        INA219();
        void config(int16_t BRNG, int16_t PGA, int16_t BADC, int16_t SADC, int16_t MODE);
        void reset();
        void standby();
        void resume();
        void measure();
        void setMode(byte mode);

        uint16_t getConfig();
        uint16_t getCalibrationValue();
        void setCalibrationValue();
        void calCalibrationValue(byte MaxExpectedCurrent_A);
};

#endif // INA219_H_