#include "INA219.h"
#include "tools.h"

INA219 powerMeter;
#define PM (powerMeter.measurement)
#define PC (powerMeter.calculations)
#define PS (powerMeter.settings)

String str;
byte value = 0;
byte maxCurrent = 1;

void setup() {
    Serial.begin(115200);
    powerMeter.config(INA219_BRNG_16_FSR, INA219_PGA_40mV, INA219_BADC_MODE_12_BIT, INA219_SADC_MODE_12_BIT, INA219_SB_CONT);
}

void loop() {
    if ( Serial.available() ) {
        str = Serial.readStringUntil('\n');

        switch (str.charAt(0)) {
            case 'm':
                powerMeter.measure();
                SPT("Shunt Voltage        [", PM.shuntVoltage);
                SPL("][/1000][uV -> mV] : ", PM.shuntVoltage * PC.lsb.shuntVoltage / 1000);
                SPT("Bus Voltage          [", PM.busVoltage);
                SPL("][/1000][mV ->  V] : ", PM.busVoltage * PC.lsb.busVoltage / 1000);
                SPT("Current              [", PM.current);
                SPL("][x1000][A  -> mA] : ", PM.current * PC.lsb.current * 1000);
                SPT("Power                [", PM.power);
                SPL("][x1000][W  -> mW] : ", PM.power * PC.lsb.power * 1000);
                if ( powerMeter.busVoltageOverflow ) {
                    SPL("Overflow, power & current value maybe wrong"," ");
                }
                SPL("----------------------------------------------", "-");
                break;
            case 'c':
                powerMeter.calCalibrationValue(maxCurrent);  // Expected 4.0 A max current
                SPL("Max Possible Current              [mA] : ", PC.MaxPossibleCurrent);
                SPL("Max Current                        [A] : ", PC.MaxCurrent);
                SPL("Max Current before Overflow        [A] : ", PC.MaxCurrentBeforeOverflow);
                SPL("Max Shunt Voltage                  [V] : ", PC.MaxShuntVoltage);
                SPL("Max Shunt Voltage before Overflow  [V] : ", PC.MaxShuntVoltageBeforeOverflow);
                SPL("Max Power                          [W] : ", PC.MaxPower);
                SPL("Power LSB             [x1000][W -> mW] : ", PC.lsb.power * 1000);
                SPL("Current LSB           [x1000][A -> mA] : ", PC.lsb.current * 1000);
                SPL("Calibration Value                      : ", PC.calibrationValue);
                SPL("----------------------------------------------", "-");
                powerMeter.setCalibrationValue();
                break;
            case '5':
                SPHL("Current Config : ", powerMeter.getConfig());
                break;
            case '6':
                SPHL("Current Calibration Value : ", powerMeter.getCalibrationValue());
                break;
            case '7':
                SPL("Please enter mode : ", " ");
                SPL("1 - Power-down"," ");
                SPL("2 - Shunt voltage, triggered"," ");
                SPL("3 - Bus voltage, triggered"," ");
                SPL("4 - Shunt and bus, triggered"," ");
                SPL("5 - ADC off (disabled)"," ");
                SPL("6 - Shunt voltage, continuous"," ");
                SPL("7 - Bus voltage, continuous"," ");
                SPL("8 - Shunt and bus, continuous"," ");
                readValue();
                powerMeter.setMode(value);
                SPL("Mode Updated : ", value);
                break;
            case '8':
                powerMeter.standby();
                SPL("Standby"," ");
                break;
            case '9':
                powerMeter.resume();
                SPL("Resume"," ");
                break;
            case '0':
                powerMeter.reset();
                SPL("Reset"," ");
                break;
            case 'u':
                powerMeter.config(PS.BRNG, PS.PGA, PS.BADC, PS.SADC, PS.MODE);
                SPL("Config Updated."," ");
                break;
            case '1':
                SPL("Please enter Full Scale Range BRNG (1 - 16v, 2 - 32v)"," ");
                readValue();
                switch (value){
                    case 1: PS.BRNG = INA219_BRNG_16_FSR; SPL("- 16V FSR selected, Press 'u' to update when ready.", " "); break;
                    case 2: PS.BRNG = INA219_BRNG_32_FSR; SPL("- 32V FSR selected, Press 'u to update when ready.", " "); break;
                    default: SPL("- Wrong input, accept value (1,2)"," "); break;
                }
                break;
            case '2':
                SPL("Please enter Programmable Gain PGA  (1 - 40mV, 2 - 80mV, 3 - 160mV, 4 - 320mV)", " ");
                readValue();
                switch (value) {
                    case 1: PS.PGA = INA219_PGA_40mV; SPL("- 40mV selected, Press 'u' to update when ready.", " "); break;
                    case 2: PS.PGA = INA219_PGA_80mV; SPL("- 80mV selected, Press 'u' to update when ready.", " "); break;
                    case 3: PS.PGA = INA219_PGA_160mV; SPL("- 160mV selected, Press 'u' to update when ready.", " "); break;
                    case 4: PS.PGA = INA219_PGA_320mV; SPL("- 320mV selected, Press 'u' to update when ready.", " "); break;
                    default: SPL("- Wrong input, accept value (1,2,3,4)"," "); break;
                }
                break;
            case '3':
                SPL("Please enter Maximum Expected Current [A] (in Integer)"," ");
                readValue();
                maxCurrent = value;
                SPL("- Entered Maximum expected current [A] : ", value);
                break;
            case 'p':
                printMenu();
                break;
        }
    }
}

void printMenu() {
    SPL("Function List"," ");
    SPL("------------------"," ");
    SPL("m - Measure and print the result"," ");
    SPL("------------------"," ");
    SPL("You can change the BRNG and PGA and Expected current(for calibration)"," ");
    SPL("Once you enter these value, please Press 'u'/'c' to upload to INA219", " ");
    SPL("BADC and SADC is use default 12-bit, to change it, "," ");
    SPL("you should modify the value at setup() in this program."," ");
    SPL(" "," ");
    SPL("1 - Set Full Scale Range BRNG, press 'u' to update the config value", " ");
    SPL("2 - Set Programmable Gain, press 'u' to update the config value", " ");
    SPL("3 - Set Expected Current, press 'c' to update the calibration value"," ");
    SPL("u - Update the Config value"," ");
    SPL("c - Update the Calibration value", " ");
    SPL("------------------"," ");
    SPL("5 - Read current config from INA219 (in HEX)", " ");
    SPL("6 - Read current calibration value from INA219", " ");
    SPL("7 - Set operation mode", " ");
    SPL("8 - Power down", " ");
    SPL("9 - Resume from power down to last mode"," ");
    SPL("0 - Reset INA219", " ");
    SPL("------------------"," ");
    SPL("P - Print this list"," ");
    SPL("------------------"," ");
}

void readValue() {
    while ( !Serial.available() ) {
        continue;
    }

    str = Serial.readStringUntil('\n');
    value = byte(str.toInt());
}