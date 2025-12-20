#pragma once

#include <AP_HAL/AP_HAL.h>
#include <AP_Param/AP_Param.h>
#include <SRV_Channel/SRV_Channel_config.h>
// #include "AP_ESC_Telem_Backend.h"

#if HAL_WITH_ESC_TELEM

#ifndef ESC_TELEM_MAX_ESCS
    #define ESC_TELEM_MAX_ESCS NUM_SERVO_CHANNELS
#endif

// #define Slow_CAN                    1                               ///< Flag whether to slow down CAN packets' transmission
#define Slow_CAN_Del                1                               ///< Base Delay for slowed-down CAN, in ms

#define HW_FOC_Val_HEAD             ( (uint8_t)0x9B)                ///< Header
#define HW_FOC_Val_Len              0x16                            ///< Data frame length, from "Head", excluding CRC
#define HW_FOC_Val_Ver              0x01                            ///< Version of Proto
#define HW_FOC_Val_Cmd              0x02                            ///< Command: Real-time data

// Example:  9B 16 01 02 33 3B 00 87 00 87 04 10 01 CB 00 03 00 23 D2 D2 00 00 DA 04

    typedef struct HW_FOC_ESC_Telem_s 
        {
        uint8_t     Head;                                           ///< Header
        uint8_t     Len;                                            ///< Data frame length, from "Head", excluding CRC
        uint8_t     Ver;                                            ///< Version of the Protocol
        uint8_t     Cmd;                                            ///< Command
        uint8_t     PktNum_H;                                       ///< Packet Number -- high          :: 33
        uint8_t     PktNum_L;                                       ///< Packet Number -- low,    
        uint8_t     iThrot_H;                                       ///< Input Throttle -- high   |
        uint8_t     iThrot_L;                                       ///< Input Throttle -- low     |__ Real val = <reg> *100 /1024
        uint8_t     oThrot_H;                                       ///< Output Throttle -- high   /
        uint8_t     oThrot_L;                                       ///< Output Throttle -- low   /
        uint8_t     eRPM_H;                                         ///< "electronics" RPM -- high
        uint8_t     eRPM_L;                                         ///< "electronics" RPM -- low;  Real val = <reg> *10 / <poles pairs> num
        uint8_t     iVolt_H;                                        ///< Input voltage -- high
        uint8_t     iVolt_L;                                        ///< Input voltage -- low;  Real val = <reg> /10
        uint8_t     iCurr_H;                                        ///< Input Current -- high
        uint8_t     iCurr_L;                                        ///< Input Current -- low;  Real val = <reg> / 64
        uint8_t     pCurr_H;                                        ///< Phase Current -- high
        uint8_t     pCurr_L;                                        ///< Phase Current -- low;  Real val = <reg> / 64
        uint8_t     mTemp;                                          ///< MOSFET temperature, see LookUp table
        uint8_t     cTemp;                                          ///< Capacitor temperature, see LookUp table
        uint8_t     Status_H;                                       ///< Status code -- high, see masks
        uint8_t     Status_L;                                       ///< Status code -- low, see masks
        uint8_t     CRC_L;                                          ///< LOW  byte of CRC -- Little-Endian here!
        uint8_t     CRC_H;                                          ///< HIGH byte of CRC -- Little-Endian here!
        }   HW_FOC_ESC_Telem_t __attribute__((packed)) ;

typedef struct ADC_2_Temp_s
    {
        uint8_t     ADC_Val;
        uint8_t     Temp_Val;
    }   ADC_2_Temp_t  __attribute__((packed)) ;

    /// @brief  Union to convert signed bytes to 16-bit int and then to Float
typedef union Bytes2int_u
    {
    int16_t      Int16;
    struct bytes_s
        {
        int8_t b1;
        int8_t b2;
        }       Int8s;
    } Bytes2int_t;

/// Modes of MultiSkid operation:
typedef enum MultiSkid_e
    {
    msk_Single,                                                         ///< Single-Shot mode, compatible
    msk_Multi,                                                          ///< Multi-Skid mode, by Mask
                                                                        ///< ...possible: 2-first, intervaled-next.
    msk_Transit,                                                        ///< Unknown by now, transiting
    } MultiSkid_t;

// ============================== C O N S T A N T S : ============================

#define                 GC_Version                  0xAA5B              ///< GrayCat local version

#define                 NUM_Telems                  4                   ///< Quantity of Telemetry channels

#define                 Use_ExtRC                   1                   ///< Flag whether to use RC Extention

// #define                 GC_AlwaysRemap              1                   ///< Flag whether to Always Do Channels Remapping
#define                 GC_Remap19                  1                   ///< Flag whether to use Ch20...27 => S5...S12 remap

#define                 RC_IndirStartPWM            900                 ///< Starting "PWM-setting" from which RC-remapping begins
#define                 RC_DisArmedVal              1495                ///< Value for "DisArmed" thumbler
#define                 MultiSkidMode_TO            500                 ///< Milliseconds for MultiSkid mode timeout after Syndrome appearance

#if ( 3995 == APJ_BOARD_ID   )                                          // :: (CHIBIOS_BOARD_NAME == "BVMT_CanEx") 
    #define                 RC_IndirCh1                 1                   ///< Number of the first Indirect Channel
#else       // testbed from Lastivka:
    #define                 RC_IndirCh1                 5                   ///< Number of the first Indirect Channel
#endif        // --------- CHIBIOS_BOARD_NAME 

#define                 RC_RemapOfs                 15                  ///< Quantity of channels to shift Servos 5...12 to: #20-#5 = 15

#define                 ReadBufSize                 64                 ///< Size of the intermediate buffer
#define                 HW_FOC_INTER_PACKET_TO      2                  ///< TimeOut between HW_FOC packets

// ============================== P R O C E D U R E S : ==========================

/// @brief Decode "HobbyWing" ADC value into temperature:
int FOC_temp_decode(int temp_raw);

/// Convert under-16bit values from UART Telemetry to Int16:
int16_t Convert_FOC2Int( uint8_t RegLow, uint8_t RegHi   );

/// Procedure to read, convert and put into internal structures of HW FOC packets:
void ReadTelem_N(int Uart_N );

void FireSkidsMask( uint8_t Mask );

/// Save Min/Max limits from Servo31
void SaveLimits( float ch_val );

/// Insert a New ContrValue, and calculate Tremor
int CalcDiff_ContrVals( float NewContrVal );

#endif // HAL_WITH_ESC_TELEM
