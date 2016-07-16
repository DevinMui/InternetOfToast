#include "mbed.h"
#include "sensors.h"

//I2C for pmod sensors:
#define Si1145_PMOD_I2C_ADDR   0xC0 //this is for 7-bit addr 0x60 for the Si7020
#define Si7020_PMOD_I2C_ADDR   0x80 //this is for 7-bit addr 0x4 for the Si7020

#include "hardware.h"
//I2C i2c(PTC11, PTC10); //SDA, SCL

#include "FXOS8700CQ.h"
// Pin names for the motion sensor FRDM-K64F board:
FXOS8700CQ fxos(PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); // SDA, SCL, (addr << 1)
// Storage for the data from the sensor
SRAWDATA accel_data;
SRAWDATA magn_data;
//InterruptIn fxos_int1(PTC6); // unused, common with SW2 on FRDM-K64F
InterruptIn fxos_int2(PTC13); // should just be the Data-Ready interrupt
bool fxos_int2_triggered = false;
void trigger_fxos_int2(void)
{
    fxos_int2_triggered = true;
    //us_ellapsed = t.read_us();
}

/*------------------------------------------------------------------------------
 * Perform I2C single read.
 *------------------------------------------------------------------------------*/
unsigned char I2C_ReadSingleByte(unsigned char ucDeviceAddress)
{
    char rxbuffer [1];
    i2c.read(ucDeviceAddress, rxbuffer, 1 );
    return (unsigned char)rxbuffer[0];
} //I2C_ReadSingleByte()

/*------------------------------------------------------------------------------
 * Perform I2C single read from address.
 *------------------------------------------------------------------------------*/
unsigned char I2C_ReadSingleByteFromAddr(unsigned char ucDeviceAddress, unsigned char Addr)
{
    char txbuffer [1];
    char rxbuffer [1];
    txbuffer[0] = (char)Addr;
    i2c.write(ucDeviceAddress, txbuffer, 1 );
    i2c.read(ucDeviceAddress, rxbuffer, 1 );
    return (unsigned char)rxbuffer[0];
} //I2C_ReadSingleByteFromAddr()

/*------------------------------------------------------------------------------
 * Perform I2C read of more than 1 byte.
 *------------------------------------------------------------------------------*/
int I2C_ReadMultipleBytes(unsigned char ucDeviceAddress, char *ucData, unsigned char ucLength)
{
    int status;
    status = i2c.read(ucDeviceAddress, ucData, ucLength);
    return status;
} //I2C_ReadMultipleBytes()

/*------------------------------------------------------------------------------
 * Perform I2C write of a single byte.
 *------------------------------------------------------------------------------*/
int I2C_WriteSingleByte(unsigned char ucDeviceAddress, unsigned char Data, bool bSendStop)
{
    int status;
    char txbuffer [1];
    txbuffer[0] = (char)Data; //data
    status = i2c.write(ucDeviceAddress, txbuffer, 1, !bSendStop); //true: do not send stop
    return status;
} //I2C_WriteSingleByte()

/*------------------------------------------------------------------------------
 * Perform I2C write of 1 byte to an address.
 *------------------------------------------------------------------------------*/
int I2C_WriteSingleByteToAddr(unsigned char ucDeviceAddress, unsigned char Addr, unsigned char Data, bool bSendStop)
{
    int status;
    char txbuffer [2];
    txbuffer[0] = (char)Addr; //address
    txbuffer[1] = (char)Data; //data
    //status = i2c.write(ucDeviceAddress, txbuffer, 2, false); //stop at end
    status = i2c.write(ucDeviceAddress, txbuffer, 2, !bSendStop); //true: do not send stop
    return status;
} //I2C_WriteSingleByteToAddr()

/*------------------------------------------------------------------------------
 * Perform I2C write of more than 1 byte.
 *------------------------------------------------------------------------------*/
int I2C_WriteMultipleBytes(unsigned char ucDeviceAddress, char *ucData, unsigned char ucLength, bool bSendStop)
{
    int status;
    status = i2c.write(ucDeviceAddress, ucData, ucLength, !bSendStop); //true: do not send stop
    return status;
} //I2C_WriteMultipleBytes()

bool bSi7020_present = false;
void Init_Si7020(void)
{
    char SN_7020 [8];
    //SN part 1:
    I2C_WriteSingleByteToAddr(Si7020_PMOD_I2C_ADDR, 0xFA, 0x0F, false);
    I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &SN_7020[0], 4);

    //SN part 1:
    I2C_WriteSingleByteToAddr(Si7020_PMOD_I2C_ADDR, 0xFC, 0xC9, false);
    I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &SN_7020[4], 4);

    char Ver_7020 [2];
    //FW version:
    I2C_WriteSingleByteToAddr(Si7020_PMOD_I2C_ADDR, 0x84, 0xB8, false);
    I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &Ver_7020[0], 2);

    if (SN_7020[4] != 0x14)
    {
        bSi7020_present = false;
        printf("Si7020 sensor not found\n");
    }
    else 
    {
        bSi7020_present = true;
        printf("Si7020 SN = 0x%02X%02X%02X%02X%02X%02X%02X%02X\n", SN_7020[0], SN_7020[1], SN_7020[2], SN_7020[3], SN_7020[4], SN_7020[5], SN_7020[6], SN_7020[7]);
        printf("Si7020 Version# = 0x%02X\n", Ver_7020[0]);
    } //bool bSi7020_present = true

} //Init_Si7020()

void Read_Si7020(void)
{
    if (bSi7020_present)
    {
        char Humidity [2];
        char Temperature [2];
        //Command to measure humidity (temperature also gets measured):
        I2C_WriteSingleByte(Si7020_PMOD_I2C_ADDR, 0xF5, false); //no hold, must do dummy read
        I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &Humidity[0], 1); //dummy read, should get an nack until it is done
        wait (0.05); //wait for measurement.  Can also keep reading until no NACK is received
        //I2C_WriteSingleByte(Si7020_PMOD_I2C_ADDR, 0xE5, false); //Hold mod, the device does a clock stretch on the read until it is done (crashes the I2C bus...
        I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &Humidity[0], 2); //read humidity
        //printf("Read Si7020 Humidity = 0x%02X%02X\n", Humidity[0], Humidity[1]);
        int rh_code = (Humidity[0] << 8) + Humidity[1];
        float fRh = (125.0*rh_code/65536.0) - 6.0; //from datasheet
        //printf("Si7020 Humidity = %*.*f %%\n", 4, 2, fRh); //double % sign for escape //printf("%*.*f\n", myFieldWidth, myPrecision, myFloatValue);
        sprintf(SENSOR_DATA.Humidity_Si7020, "%0.2f", fRh);
        
        //Command to read temperature when humidity is already done:
        I2C_WriteSingleByte(Si7020_PMOD_I2C_ADDR, 0xE0, false);
        I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &Temperature[0], 2); //read temperature
        //printf("Read Si7020 Temperature = 0x%02X%02X\n", Temperature[0], Temperature[1]);
        int temp_code = (Temperature[0] << 8) + Temperature[1];
        float fTemp = (175.72*temp_code/65536.0) - 46.85; //from datasheet in Celcius
        //printf("Si7020 Temperature = %*.*f deg C\n", 4, 2, fTemp);
        sprintf(SENSOR_DATA.Temperature_Si7020, "%0.2f", fTemp);
    } //bool bSi7020_present = true

} //Read_Si7020()

/*------------------------------------------------------------------------------
 * The following are aliases so that the Si1145 coding examples can be used as-is.
 *------------------------------------------------------------------------------*/
unsigned char ReadFrom_Si1145_Register(unsigned char reg) //returns byte from I2C Register 'reg'
{
    unsigned char result = I2C_ReadSingleByteFromAddr(Si1145_PMOD_I2C_ADDR, reg);
    return (result);
} //ReadFrom_Si1145_Register()

void WriteTo_Si1145_Register(unsigned char reg, unsigned char value) //writes 'value' into I2C Register reg'
{
    I2C_WriteSingleByteToAddr(Si1145_PMOD_I2C_ADDR, reg, value, true);
} //WriteTo_Si1145_Register()

#define REG_PARAM_WR 0x17
#define REG_PARAM_RD 0x2E
#define REG_COMMAND 0x18
#define REG_RESPONSE 0x20
#define REG_HW_KEY 0x07
#define HW_KEY_VAL0 0x17
#define REG_MEAS_RATE_LSB 0x08
#define REG_MEAS_RATE_MSB 0x09
#define REG_PS_LED21 0x0F
#define REG_PS_LED3 0x10
#define MAX_LED_CURRENT 0xF
#define PARAM_CH_LIST 0x01
#define REG_ALS_VIS_DATA0 0x22
#define REG_ALS_VIS_DATA1 0x23
#define REG_ALS_IR_DATA0 0x24
#define REG_ALS_IR_DATA1 0x25
#define REG_PS1_DATA0 0x26
#define REG_PS1_DATA1 0x27
#define REG_PS2_DATA0 0x28
#define REG_PS2_DATA1 0x29
#define REG_PS3_DATA0 0x2A
#define REG_PS3_DATA1 0x2B
#define REG_UVINDEX0 0x2C
#define REG_UVINDEX1 0x2D
int Si1145_ParamSet(unsigned char address, unsigned char value) //writes 'value' into Parameter 'address'
{
    char txbuffer [3];
    txbuffer[0] = (char)REG_PARAM_WR; //destination
    txbuffer[1] = (char)value;
    txbuffer[2] = (char)(0xA0 + (address & 0x1F));
    int retval;
    //if((retval = _waitUntilSleep(si114x_handle))!=0) return retval;
    retval = I2C_WriteMultipleBytes(Si1145_PMOD_I2C_ADDR, &txbuffer[0], 3, true);
    if(retval!=0) return retval;
    while(1)
    {
        retval=ReadFrom_Si1145_Register(REG_PARAM_RD);
        if (retval==value) break;
    }
    return (0);
} //Si1145_ParamSet()

void PsAlsForce(void) //equivalent to WriteTo_Si1145_Register(REG_COMMAND,0x07).  This forces PS and ALS measurements
{
    WriteTo_Si1145_Register(REG_COMMAND,0x07);
} //PsAlsForce()

bool bSi1145_present = false;
void Init_Si1145(void)
{
    unsigned char readbyte;
    //Read Si1145 part ID:
    readbyte = ReadFrom_Si1145_Register(0x00);
    if (readbyte != 0x45)
    {
        bSi1145_present = false;
        printf("Si1145 sensor not found\n");
    }
    else
    {
        bSi1145_present = true;
        printf("Si1145 Part ID : 0x%02X\n", readbyte);
        //Initialize Si1145 by writing to HW_KEY (I2C Register 0x07 = 0x17)
        WriteTo_Si1145_Register(REG_HW_KEY, HW_KEY_VAL0);
    
        // Initialize LED Current
        // I2C Register 0x0F = 0xFF
        // I2C Register 0x10 = 0x0F
        WriteTo_Si1145_Register(REG_PS_LED21,(MAX_LED_CURRENT<<4) + MAX_LED_CURRENT);
        WriteTo_Si1145_Register(REG_PS_LED3, MAX_LED_CURRENT);

        // Parameter 0x01 = 0x37
        //Si1145_ParamSet(PARAM_CH_LIST, ALS_IR_TASK + ALS_VIS_TASK + PS1_TASK + PS2_TASK + PS3_TASK);
        //Si1145_ParamSet(0x01, 0x37); //CHLIST is address 0x01 in the parameter RAM.  It defines which sensors are enabled (here, some)
        Si1145_ParamSet(0x01, 0x7F); //CHLIST is address 0x01 in the parameter RAM.  It defines which sensors are enabled (here, all but UV.  One can only use AUX or UV but here we use AUX because UV does not work...)
        // I2C Register 0x18 = 0x0x07 //This is PSALS_FORCE to the Command register => Force a single PS (proximity sensor) and ALS (ambient light sensor) reading - The factory code has this as 0x05 which only does PS...
        PsAlsForce(); // can also be written as WriteTo_Si1145_Register(REG_COMMAND,0x07);
        WriteTo_Si1145_Register(REG_COMMAND, 0x0F);//command to put it into auto mode
        //Set MES_RATE to 0x1000.  I.e. the device will automatically wake up every 16 * 256* 31.25 us = 0.128 seconds to measure
        WriteTo_Si1145_Register(REG_MEAS_RATE_LSB, 0x00);
        WriteTo_Si1145_Register(REG_MEAS_RATE_MSB, 0x10);
    } //bSi1145_present = true
} //Init_Si1145()

void Read_Si1145(void)
{
    if (bSi1145_present)
    {
        // Once the measurements are completed, here is how to reconstruct them
        // Note very carefully that 16-bit registers are in the 'Little Endian' byte order
        // It may be more efficient to perform block I2C Reads, but this example shows
        // individual reads of registers
    
        int PS1 = ReadFrom_Si1145_Register(REG_PS1_DATA0) + 256 * ReadFrom_Si1145_Register(REG_PS1_DATA1);
        int PS2 = ReadFrom_Si1145_Register(REG_PS2_DATA0) + 256 * ReadFrom_Si1145_Register(REG_PS2_DATA1);
        int PS3 = ReadFrom_Si1145_Register(REG_PS3_DATA0) + 256 * ReadFrom_Si1145_Register(REG_PS3_DATA1);
        //printf("PS1_Data = %d\n", PS1);
        //printf("PS2_Data = %d\n", PS2);
        //printf("PS3_Data = %d\n", PS3);
        //OBJECT PRESENT?
        if(PS1 < 22000){
            //printf("Object Far\n");
            sprintf(SENSOR_DATA.Proximity, "Object Far");
        }
        else if(PS1 < 24000)
        {
            //printf("Object in Vicinity\n");
            sprintf(SENSOR_DATA.Proximity, "Object in Vicinity");
        }
        else if (PS1 < 30000)
        {
            //printf("Object Near\n");
            sprintf(SENSOR_DATA.Proximity, "Object Near");
        }
        else
        {
            //printf("Object Very Near\n");
            sprintf(SENSOR_DATA.Proximity, "Object Very Near");
        }
    
        //Force ALS read:
        //WriteTo_Si1145_Register(REG_COMMAND, 0x06);
        //wait (0.1);
        int ALS_VIS = ReadFrom_Si1145_Register(REG_ALS_VIS_DATA0) + 256 * ReadFrom_Si1145_Register(REG_ALS_VIS_DATA1);
        int ALS_IR = ReadFrom_Si1145_Register(REG_ALS_IR_DATA0) + 256 * ReadFrom_Si1145_Register(REG_ALS_IR_DATA1);
        int UV_INDEX = ReadFrom_Si1145_Register(REG_UVINDEX0) + 256 * ReadFrom_Si1145_Register(REG_UVINDEX1);
        //printf("ALS_VIS_Data = %d\n", ALS_VIS);
        //printf("ALS_IR_Data = %d\n", ALS_IR);
        //printf("UV_INDEX_Data = %d\n", UV_INDEX);
    
        //printf("Ambient Light Visible  Sensor = %d\n", ALS_VIS);
        sprintf(SENSOR_DATA.AmbientLightVis, "%d", ALS_VIS);
        //printf("Ambient Light Infrared Sensor = %d\n", ALS_IR);
        sprintf(SENSOR_DATA.AmbientLightIr, "%d", ALS_IR);
        //float fUV_value = (UV_INDEX -50.0)/10000.0;
        float fUV_value = (UV_INDEX)/100.0; //this is the aux reading
        //printf("UV_Data = %0.2f\n", fUV_value);
        sprintf(SENSOR_DATA.UVindex, "%0.2f", fUV_value);
    } //bSi1145_present = true
} //Read_Si1145()

//********************************************************************************************************************************************
//* Read the FXOS8700CQ - 6-axis combo Sensor Accelerometer and Magnetometer
//********************************************************************************************************************************************
bool bMotionSensor_present = false;
void init_motion_sensor()
{
    int iWhoAmI = fxos.get_whoami();
    
    printf("FXOS8700CQ WhoAmI = %X\r\n", iWhoAmI);
    // Iterrupt for active-low interrupt line from FXOS
    // Configured with only one interrupt on INT2 signaling Data-Ready
    //fxos_int2.fall(&trigger_fxos_int2);
    if (iWhoAmI != 0xC7)
    {
        bMotionSensor_present = false;
        printf("FXOS8700CQ motion sensor not found\n");
    }
    else
    {
        bMotionSensor_present = true;
        fxos.enable();
    }
} //init_motion_sensor

void read_motion_sensor()
{
    if (bMotionSensor_present)
    {
        fxos.get_data(&accel_data, &magn_data);
        //printf("Roll=%5d, Pitch=%5d, Yaw=%5d;\r\n", magn_data.x, magn_data.y, magn_data.z);
        sprintf(SENSOR_DATA.MagnetometerX, "%5d", magn_data.x);
        sprintf(SENSOR_DATA.MagnetometerY, "%5d", magn_data.y);
        sprintf(SENSOR_DATA.MagnetometerZ, "%5d", magn_data.z);
    
        //Try to normalize (/2048) the values so they will match the eCompass output:
        float fAccelScaled_x, fAccelScaled_y, fAccelScaled_z;
        fAccelScaled_x = (accel_data.x/2048.0);
        fAccelScaled_y = (accel_data.y/2048.0);
        fAccelScaled_z = (accel_data.z/2048.0);
        //printf("Acc: X=%2.3f Y=%2.3f Z=%2.3f;\r\n", fAccelScaled_x, fAccelScaled_y, fAccelScaled_z);
        sprintf(SENSOR_DATA.AccelX, "%2.3f", fAccelScaled_x);
        sprintf(SENSOR_DATA.AccelY, "%2.3f", fAccelScaled_y);
        sprintf(SENSOR_DATA.AccelZ, "%2.3f", fAccelScaled_z);
    } //bMotionSensor_present
} //read_motion_sensor

void sensors_init(void)
{
    Init_Si7020();
    Init_Si1145();
    init_motion_sensor();
} //sensors_init

void read_sensors(void)
{
    Read_Si7020();
    Read_Si1145();
    read_motion_sensor();
} //read_sensors
