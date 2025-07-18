#pragma once

#include <AP_HAL/AP_HAL.h>
#include <AP_Param/AP_Param.h>
#include <SRV_Channel/SRV_Channel_config.h>
// #include "AP_ESC_Telem_Backend.h"

#if HAL_WITH_ESC_TELEM

#ifndef ESC_TELEM_MAX_ESCS
    #define ESC_TELEM_MAX_ESCS NUM_SERVO_CHANNELS
#endif

#define HW_FOC_Val_HEAD             ( (uint8_t)0x9B)                ///< Header
#define HW_FOC_Val_Len              0x16                            ///< Data frame length, from "Head", excluding CRC
#define HW_FOC_Val_Ver              0x01                            ///< Version of Proto
#define HW_FOC_Val_Cmd              0x02                            ///< Command: Real-time data

// Example:  9B 16 01 02 33 3B 00 87 00 87 04 10 01 CB 00 03 00 23 D2 D2 00 00 DA 04

MAVPACKED(
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
        int8_t      eRPM_H;                                         ///< "electronics" RPM -- high
        int8_t      eRPM_L;                                         ///< "electronics" RPM -- low;  Real val = <reg> *10 / <poles pairs> num
        int8_t      iVolt_H;                                        ///< Input voltage -- high
        int8_t      iVolt_L;                                        ///< Input voltage -- low;  Real val = <reg> /10
        int8_t      iCurr_H;                                        ///< Input Current -- high
        int8_t      iCurr_L;                                        ///< Input Current -- low;  Real val = <reg> / 64
        int8_t      pCurr_H;                                        ///< Phase Current -- high
        int8_t      pCurr_L;                                        ///< Phase Current -- low;  Real val = <reg> / 64
        int8_t      mTemp;                                          ///< MOSFET temperature, see LookUp table
        int8_t      cTemp;                                          ///< Capacitor temperature, see LookUp table
        uint8_t     Status_H;                                       ///< Status code -- high, see masks
        uint8_t     Status_L;                                       ///< Status code -- low, see masks
        uint8_t     CRC_L;                                          ///< LOW  byte of CRC -- Little-Endian here!
        uint8_t     CRC_H;                                          ///< HIGH byte of CRC -- Little-Endian here!
        })  HW_FOC_ESC_Telem_t;

#endif // HAL_WITH_ESC_TELEM
