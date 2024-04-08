#include "INA219.h"

/*****************************
    Private Functions
*****************************/

uint16_t INA219::I2C_Read(byte addr) {
    Wire.beginTransmission(INA219_I2C_ADDRESS);
    Wire.write(addr);
    Wire.endTransmission();

    Wire.requestFrom(INA219_I2C_ADDRESS,2);
    byte val_H = 0;
    byte val_L = 0;

    while ( !Wire.available() ) {}

    val_H = Wire.read();
    val_L = Wire.read();

    return ( val_L + val_H * 256 );
}

void INA219::I2C_Write(byte addr, uint16_t data) {
    byte data_H = data >> 8;
    byte data_L = byte(data);
    Wire.beginTransmission(INA219_I2C_ADDRESS);
    Wire.write(addr);
    Wire.write(data_H);
    Wire.write(data_L);
    Wire.endTransmission();
}

float INA219::getRawShuntVoltage() {
    return static_cast<int16_t>(I2C_Read(INA219_REG_SHUNT_VOLT));
}

float INA219::getRawBusVoltage() {
    uint16_t ret = I2C_Read(INA219_REG_BUS_VOLT);
    busVoltageOverflow = ret & 0x0001;
    busVoltageConversionReady = ret & 0x0002;

    return ( ret >> 3 );
}

float INA219::getRawPower() {
    setCalibrationValue();
    return I2C_Read(INA219_REG_POWER);
}

float INA219::getRawCurrent() {
    setCalibrationValue();
    return static_cast<int16_t>(I2C_Read(INA219_REG_CURRENT));
}

/*****************************
    Public Functions
*****************************/
INA219::INA219() {
    Wire.begin();
    I2C_Write(INA219_REG_CONFIG, INA219_DEFAULT_CONFIG);
    I2C_Write(INA219_REG_CALIBRATION, INA219_DEFAULT_CALIBATION);
}

void INA219::config(int16_t BRNG, int16_t PGA, int16_t BADC, int16_t SADC, int16_t MODE) {
    settings.BRNG = BRNG;
    settings.PGA = PGA;
    settings.BADC = BADC;
    settings.SADC = SADC;
    settings.MODE = MODE;
    I2C_Write(INA219_REG_CONFIG, BRNG | PGA | BADC | SADC | MODE);
}

void INA219::reset() {
    I2C_Write(INA219_REG_CONFIG, INA219_RST);
    settings.MODE = INA219_SB_CONT;
}

void INA219::setMode(byte mode) {
    settings.MODE = mode;
    config(settings.BRNG, settings.PGA, settings.BADC, settings.SADC, settings.MODE);
}

void INA219::standby() {
    config(settings.BRNG, settings.PGA, settings.BADC, settings.SADC, INA219_POWER_DOWN);
}

void INA219::resume() {
    config(settings.BRNG, settings.PGA, settings.BADC, settings.SADC, settings.MODE);
}

void INA219::measure() {
    measurement.shuntVoltage = getRawShuntVoltage();
    measurement.busVoltage = getRawBusVoltage();
    measurement.current = getRawCurrent();
    measurement.power = getRawPower();
}

uint16_t INA219::getConfig() {
    return I2C_Read(INA219_REG_CONFIG);
}

uint16_t INA219::getCalibrationValue() {
    return I2C_Read(INA219_REG_CALIBRATION);
}

void INA219::setCalibrationValue() {
    I2C_Write(INA219_REG_CALIBRATION, calculations.calibrationValue);
}

void INA219::calCalibrationValue(byte MaxExpectedCurrent_A ) {
    // Data sheet page 12
    // Cal = trunc ( 0,04096 / (Current_LSB * RShunt) )
    // 
    // 0.04096 is an internal fixed value used to ensure scaling is maintained properly
    // Current_LSB = Maximum Expected Current / 2^15
    // R_Shunt = 0.1 ohm

    float VBusMax = 16.0;           // Max Bus Voltage, V
    float VShuntMax = 40.0;         // Max Shunt Voltage, mV
    
    // get VBusMax from BRNG config'ed
    if ( settings.BRNG == INA219_BRNG_32_FSR ) {
        VBusMax = 32.0;
    }

    // get VShuntMax from PGA config'ed
    switch ( settings.PGA ) {
        case 0x0000: VShuntMax = 40.0; break;
        case 0x1000: VShuntMax = 80.0; break;
        case 0x2000: VShuntMax = 160.0; break;
        case 0x3000: VShuntMax = 320.0; break;
    }

    calculations.lsb.current = MaxExpectedCurrent_A / INA219_2_POW_15;
    calculations.lsb.power = 20 * calculations.lsb.current;

    calculations.MaxPossibleCurrent = VShuntMax / INA219_SHUNT_OHM;
    calculations.MaxCurrent = calculations.lsb.current * INA219_2_POW_15;

    if ( calculations.MaxCurrent > calculations.MaxPossibleCurrent ) {
        calculations.MaxCurrentBeforeOverflow = calculations.MaxPossibleCurrent;
    } else {
        calculations.MaxCurrentBeforeOverflow = calculations.MaxCurrent;
    }

    calculations.MaxShuntVoltage = calculations.MaxCurrentBeforeOverflow * INA219_SHUNT_OHM;

    if ( calculations.MaxShuntVoltage > VShuntMax ) {
        calculations.MaxShuntVoltageBeforeOverflow = VShuntMax;
    } else {
        calculations.MaxShuntVoltageBeforeOverflow = calculations.MaxShuntVoltage;
    }

    calculations.MaxPower = calculations.MaxCurrentBeforeOverflow * VBusMax;

    calculations.calibrationValue = trunc(0.04096 / (calculations.lsb.current * INA219_SHUNT_OHM));
}