/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <AP_HAL/AP_HAL.h>
#if AP_PERIPH_RC_OUT_ENABLED
#include "AP_Periph.h"
#if AP_SIM_ENABLED
#include <dronecan_msgs.h>
#endif

#include "../../libraries/AP_ESC_Telem/AP_ESC_Telem.h"
#include "AP_ESC_Telem_HW-FOC.h"

// magic value from UAVCAN driver packet
// dsdl/uavcan/equipment/esc/1030.RawCommand.uavcan
// Raw ESC command normalized into [-8192, 8191]
#define UAVCAN_ESC_MAX_VALUE    8191

#define SERVO_OUT_RCIN_MAX      32  // note that we allow for more than is in the enum
#ifndef SERVO_OUT_MOTOR_MAX
#define SERVO_OUT_MOTOR_MAX     32  // SRV_Channel::k_motor1 ... SRV_Channel::k_motor8, SRV_Channel::k_motor9 ... SRV_Channel::k_motor12, SRV_Channel::k_motor13 ... SRV_Channel::k_motor32
#endif

extern const AP_HAL::HAL &hal;

void AP_Periph_FW::rcout_init()
{
#if AP_PERIPH_SAFETY_SWITCH_ENABLED
    // start up with safety enabled. This disables the pwm output until we receive an packet from the rempte system
    hal.rcout->force_safety_on();
#else
    hal.rcout->force_safety_off();
#endif

#if HAL_WITH_ESC_TELEM && !HAL_GCS_ENABLED
    if (g.esc_telem_port >= 0) {
        serial_manager.set_protocol_and_baud(g.esc_telem_port, AP_SerialManager::SerialProtocol_ESCTelemetry, /* default: 115200 */ 19200 );
    }
#endif

#if HAL_PWM_COUNT > 0
    for (uint8_t i=0; i<HAL_PWM_COUNT; i++) {
        servo_channels.set_default_function(i, SRV_Channel::Function(SRV_Channel::k_rcin1 + i));
    }
#endif

    for (uint8_t i=0; i<SERVO_OUT_RCIN_MAX; i++) {
        SRV_Channels::set_angle(SRV_Channel::Function(SRV_Channel::k_rcin1 + i), 1000);
    }

    uint32_t esc_mask = 0;
    for (uint8_t i=0; i<SERVO_OUT_MOTOR_MAX; i++) {
        SRV_Channels::set_range(SRV_Channels::get_motor_function(i), UAVCAN_ESC_MAX_VALUE);
        uint8_t chan;
        if (SRV_Channels::find_channel(SRV_Channels::get_motor_function(i), chan)) {
            esc_mask |= 1U << chan;
        }
    }

    // run this once and at 1Hz to configure aux and esc ranges
    rcout_init_1Hz();

#if HAL_DSHOT_ENABLED
    hal.rcout->set_dshot_esc_type(SRV_Channels::get_dshot_esc_type());
#endif

    // run PWM ESCs at configured rate
    hal.rcout->set_freq(esc_mask, g.esc_rate.get());

    // setup ESCs with the desired PWM type, allowing for DShot
    AP::srv().init(esc_mask, (AP_HAL::RCOutput::output_mode)g.esc_pwm_type.get());

    // run DShot at 1kHz
    hal.rcout->set_dshot_rate(SRV_Channels::get_dshot_rate(), 400);
#if HAL_WITH_ESC_TELEM
    esc_telem_update_period_ms = 1000 / constrain_int32(g.esc_telem_rate.get(), 1, 1000);
#endif
};      // ---------------------------- rcout_init() ---------------------------

void AP_Periph_FW::rcout_init_1Hz()
{
    // this runs at 1Hz to allow for run-time param changes
    AP::srv().enable_aux_servos();

    for (uint8_t i=0; i<SERVO_OUT_MOTOR_MAX; i++) {
        servo_channels.set_esc_scaling_for(SRV_Channels::get_motor_function(i));
    }
}

HW_FOC_ESC_Telem_t      HW_FOC_Telem =                                  // Packet of HW_FOC Telemetry
    {
    HW_FOC_Val_HEAD,                                                // Header
    HW_FOC_Val_Len,                                                 // Data frame length, from here, excluding CRC
    HW_FOC_Val_Ver,                                                 // Version of the Protocol
    HW_FOC_Val_Cmd,                                                 // Command: "Real-time data"        
    0,
    0
    };                                       

/// ::  Called from rcout_update() 
void AP_Periph_FW::rcout_esc(int16_t *rc, uint8_t num_channels)
    {
    int     AllZeros = 1;

    if (rc == nullptr) 
        return;

    const uint8_t channel_count = MIN(num_channels, SERVO_OUT_MOTOR_MAX);
    for (uint8_t i=0; i<channel_count; i++) 
        {
        // we don't support motor reversal yet on ESCs in AP_Periph
        SRV_Channels::set_output_scaled(SRV_Channels::get_motor_function(i), MAX(0,rc[i]));
// GC_Debug:
#ifndef GC_Debug_UART2
         // ::  rc[i] :: [-8192, 8191]
        if ( 2 == i)
            {                
            if ( rc[i]) AllZeros = 0;
            int16_t TmpVal =  rc[i] / 10;                                            // Correct for DroneCAN_GUI multiplier
            HW_FOC_Telem.eRPM_H = TmpVal >> 8;
            HW_FOC_Telem.eRPM_L = TmpVal & 0xFF;
            };      // ---------- if ( 2 == i)
        
        // AP_ESC_Telem_Backend::TelemetryData tdata {};

        if ( 0 == i)
            {
// GC_Debug:
            // int OldPWM =  SRV_Channels::srv_channel(0)->get_output_pwm();
            // SRV_Channels::set_output_scaled(SRV_Channels::get_motor_function(i), 20 );
            // SRV_Channels::set_output_pwm_chan( /* uint8_t chan */ 0 ,  /* uint16_t value */ /*  OldPWM+1 */ (rc[i] & 0x3FFF) );
            int16_t NewVal = rc[i] & 0x3FFF;
            HW_FOC_Telem.iVolt_L = NewVal & 0xFF;
            HW_FOC_Telem.iVolt_H = NewVal / 256;
            if ( rc[i]) AllZeros = 0;
            };
        if ( 1 == i)
            {            
#if 1
            if ( rc[i]) AllZeros = 0;
            int16_t NewVal = rc[i] & 0x3FFF;
            // SRV_Channels::set_output_pwm_chan( /* uint8_t chan */ 1 ,  /* uint16_t value */ NewVal );
            HW_FOC_Telem.iCurr_L = NewVal & 0xFF;
            HW_FOC_Telem.iCurr_H = NewVal >> 8;            
#else
            // rc[i] = HW_FOC_Telem.PktNum_L * 32;
            HW_FOC_Telem.iCurr_H = rc[i] / 256;
            HW_FOC_Telem.iCurr_L = rc[i] & 0xFF;
#endif            
            };
        if ( 3 == i)
            {            
            HW_FOC_Telem.mTemp = rc[i] & 0xFF;
            HW_FOC_Telem.cTemp = (rc[i] + 17) >> 6;
            if ( rc[i]) AllZeros = 0;
            };

#endif      //  GC_Debug_UART2
        };      // --------- for (uint8_t i=0;)
    
    if ( 0 &&  !AllZeros )
        {
        auto    *uart_dbg = hal.serial(7);                // Serial 8
        
        // GC_Debug:
        if ( uart_dbg->get_baud_rate() != 19200 )
            {
            uart_dbg->end();
            uart_dbg->begin( /* Default: 115200 */ 19200, /* rxSpace */ 128,  /* txSpace */ 128 );
            };

        uart_dbg->write( (const uint8_t*) &HW_FOC_Telem, /* len */ sizeof(HW_FOC_Telem) );
        HW_FOC_Telem.PktNum_L++;
        };
    rcout_has_new_data_to_update = true;
};      // ---------------------------------- rcout_esc() --------------------------

int8_t      Channel_PreSet = -1;                                ///< Index of Servo Channel preselected by Ch16
int8_t      Remap19_en = 1;                                     ///< Flag whether Remap Ch19...22 => Ch9...12 is enabled
MultiSkid_t Skid_Mode = msk_Single;                             ///< Skid mode

int         MultiSkid_TS = 0;                                   ///< TimeStamp for TimeOuting of Multi-Skid mode
uint16_t    SkidsMask = 0;                                      ///< Mask of Enabled Multi-Skid channels
bool        Firing = false;

    // .......... MultiSkid mode detection by Syndrome:
#define     ContrValsBufSize            8                       ///< Size of the Buffer
float       ContrHistory[ ContrValsBufSize ]    = {0.0};          ///< History of Control Vals
uint        ContrHist_Idx = 0;                                  ///< Index into History of Control Vals
float       AvgContrVal = 0.0;                                  ///< Average calculated

void AP_Periph_FW::rcout_srv_unitless(uint8_t actuator_id, const float command_value)
{
#if HAL_PWM_COUNT > 0
    const SRV_Channel::Function function = SRV_Channel::Function(SRV_Channel::k_rcin1 + actuator_id - 1);

// GC_Debug:
#ifdef Use_ExtRC_UnitLess
    if ( actuator_id < HAL_PWM_COUNT )
        {       // +++++++++++++++++++ Default output:        
        SRV_Channels::set_output_norm(function, command_value);
        }       // ------------------- Default output:
        else
            {       // +++++++++++++++++++ GrayCat Extended : remapped Output
            int  RemapCh = actuator_id - RC_RemapOfs;                                            // Do remap;
            const SRV_Channel::Function function_rm = SRV_Channel::Function(SRV_Channel::k_rcin1 + RemapCh - 1);
            SRV_Channels::set_output_norm( function_rm, command_value);
            };      // ------------------- GrayCat Extended : remapped Output

// GC_Debug:
    if ( 15 == actuator_id )
        {       // ............. Process channel_16 as Shifter:
        uint8_t ControlVal  = ( (command_value+1.0) * 127.9) ;                                   // Extract controlling value (byte)...
        uint8_t IndirectCh = ControlVal / (256/8);

        if ( /*  ( command_value > -1.0 )  && */ ( IndirectCh < 7) )                                     // Got valid channel:
            {       // ++++++++++++++ One of Indirect Channels pre-selected:
#if 1
            if ( (-1 != Channel_PreSet) && ( Channel_PreSet != (IndirectCh+ RC_IndirCh1 -1 ) ) )                    // Was selected another?...
                {       // ++++++++++++++ Clear previously selected  Channel;
                const SRV_Channel::Function function_indir = SRV_Channel::Function(SRV_Channel::k_rcin1 + Channel_PreSet /* - 1 */ );
                SRV_Channels::set_output_norm( function_indir, /* command_value */ -1.0 );
                };      // -------------- Clear previously selected  Channel;
#endif
            Channel_PreSet = IndirectCh + RC_IndirCh1 - 1;                                           // Store selection
            
            }       // ------------- One of Indirect Channels pre-selected:
            else
                {
                if ( ( IndirectCh >= 6) && (-1 != Channel_PreSet) )                             // Action command! 
                    {
                    const SRV_Channel::Function function_indir = SRV_Channel::Function(SRV_Channel::k_rcin1 + Channel_PreSet /* - 1 */ );
                    // SRV_Channels::set_output_norm( function_indir, /* command_value */ (ControlVal % (256/8) ) / (32.0 / 2.0) - 0.99 );
                    SRV_Channels::set_output_norm( function_indir, /* command_value */ 0.92 );
                        // .... Add to mask of channels that will be cleared if no commands are received
                    actuator.mask |= SRV_Channels::get_output_channel_mask( function_indir );
                    }
                    else
                        Channel_PreSet = -1;                                                              //  Finally, clear Pre-Select
                }
        };      // ------------- Process channel_16 as Shifter:
#endif      // -----  Use_ExtRC_UnitLess

    // Add to mask of channels that will be cleared if no commands are received
    actuator.mask |= SRV_Channels::get_output_channel_mask(function);

    rcout_has_new_data_to_update = true;
#if AP_SIM_ENABLED
    sim_update_actuator(actuator_id);
#endif
#endif
};      // --------------------------- rcout_srv_unitless() -----------------------------------


#ifdef Use_ExtRC

/// Insert a New ContrValue, and calculate Tremor
int CalcDiff_ContrVals( float NewContrVal )
    {
    float       ArMin=3000.0, ArMax = 0.0;

    if ( NewContrVal < 899.0)                                                       // Lower than low?....
        return(1);                                                                  // .... "Fire!" value

    ContrHistory[ ContrHist_Idx++ ] = NewContrVal;                                  // Put new val into array;
    ContrHist_Idx %= ContrValsBufSize;                                              // Wrap index

    AvgContrVal = 0.0;                                                              // Init filter
    for ( int acnt=0; acnt<ContrValsBufSize; acnt++ )
        {       // +++++++++++++++++++++ Walk all buffer loop
        float NextVal = ContrHistory[ acnt ];

        if ( NextVal < ArMin )
            ArMin = NextVal;
        if ( NextVal > ArMax )
            ArMax = NextVal;
        AvgContrVal += NextVal;                                                     // Accumulate filter
        };      // --------------------- Walk all buffer loop
    AvgContrVal /= ContrValsBufSize;                                                // Get average
    if ( ( (ArMax - ArMin) >= 1.0 ) && ( (ArMax - ArMin) < 5.0  ) && ( ArMin >= 899.0 ) && (ArMax < 2099.0 ) )
        return 1;
    return 0;
    };      // ---------------------------------- CalcDiff_ContrVals() --------------------------

void  AP_Periph_FW::ProcessSingleSkid( int32_t ControlVal )
    {
    int16_t IndirectCh = (ControlVal - int(RC_IndirStartPWM) ) / 100 ;             // Every 100 "1"-s...

    if (  ( ControlVal >= RC_IndirStartPWM )  && ( ControlVal < 1899 ) )                                     // Got valid channel:
        {       // ++++++++++++++ One of Indirect Channels pre-selected:
            if  (       ( (Channel_PreSet >= 0) && ( Channel_PreSet != (IndirectCh+ RC_IndirCh1 -1 ) ) )   // Was selected another?...
#if 1
                ||  ( (ControlVal > RC_DisArmedVal-5) &&  (ControlVal < RC_DisArmedVal+5) &&  (Channel_PreSet >= 0) )   // DisArmed by Thumbler?...
#endif
            )
            {       // ++++++++++++++ Clear previously selected  Channel;
            const SRV_Channel::Function function_indir = SRV_Channel::Function(SRV_Channel::k_rcin1 + Channel_PreSet );
            SRV_Channels::set_output_limit( function_indir, SRV_Channel::Limit::MIN );
            };      // -------------- Clear previously selected  Channel;
        Channel_PreSet = IndirectCh + RC_IndirCh1 - 1;                                           // Store selection
        }       // ------------- One of Indirect Channels pre-selected:
        else
            {       // +++++++++++++++++++ Outside the Indirect Channels range
            if ( ( ControlVal >= 1910 ) && ( Channel_PreSet >= 0)  )                             // Action command! 
                {
                const SRV_Channel::Function function_indir = SRV_Channel::Function(SRV_Channel::k_rcin1 + Channel_PreSet );                    
                SRV_Channels::set_output_limit( function_indir, SRV_Channel::Limit::MAX );                        
                actuator.mask |= SRV_Channels::get_output_channel_mask( function_indir );             // Add to mask of channels that will be cleared if no commands are received
                }
                else
                    {       // +++++++++++++++++++ Non-Valid  Control Val and Preset channel
                    if (Channel_PreSet >= 0)                                                            // DisArmed by Thumbler?...
                        {       // ++++++++++++++ Clear previously selected  Channel;
                        const SRV_Channel::Function function_indir = SRV_Channel::Function(SRV_Channel::k_rcin1 + Channel_PreSet );
                        SRV_Channels::set_output_limit( function_indir, SRV_Channel::Limit::MIN );
                        };      // -------------- Clear previously selected  Channel;
                    Channel_PreSet = -1;                                                              //  Finally, clear Pre-Select
                    };      // ------------------ Non-Valid  Control Val and Preset channel
            };      // ------------------ Outside the Indirect Channels range
    };      // ------------------------ ProcessSingleSkid() --------------------------------------

void FireSkidsMask( uint8_t Mask )
    {
    for ( uint8_t ocnt=0; ocnt<8 ; ocnt++ )
        {       // ++++++++++++++++++++++++ Outputs setting loop
        const SRV_Channel::Function function_indir = SRV_Channel::Function(SRV_Channel::k_rcin1 + RC_IndirCh1 -1 + ocnt );
        SRV_Channels::set_output_limit( function_indir, ( Mask & ( 1 << ocnt ) ) ?  SRV_Channel::Limit::MAX : SRV_Channel::Limit::MIN  );
        };      // ------------------------ Outputs setting loop
// GC_Debug2:
    AP::esc_telem().update_rpm( 9-1, (1000+Mask) * 1.0f , 0.0);                                     // Feedback through Telemetry

    }

void AP_Periph_FW::ProcessMultiSkids( int32_t ControlVal, const float command_value )
    {
    if (    
          ( ( ControlVal >= RC_DisArmedVal-5) && ( ControlVal <= RC_DisArmedVal+5) )               // receiving Neutral
        )
        {
        if ( Firing )
            FireSkidsMask( 0 );                                                               // Release all!
        Firing = false;
        return;
        }    

    if (  ( ControlVal >= RC_IndirStartPWM )  && ( ControlVal < 2099 ) )                      // Got valid channel:
        {       // ++++++++++++++ One of Indirect Channels pre-selected:        
        if ( Firing )
            FireSkidsMask( 0 );                                                               // Release all!
        Firing = false;

        if ( command_value < (RC_DisArmedVal*1.0) )
            {       // ++++++++++++++++ First half of PWM Range
            uint16_t IndirectCh = ( ( command_value -  (RC_IndirStartPWM*1.0)  ) / 4.5  );             // Every 4.5 "1"-s... ; Try: round( 
            SkidsMask = IndirectCh;
            }       // ---------------- First half of PWM Range
            else
                {       // ++++++++++++++++ Second half of PWM Range
                uint16_t IndirectCh = ( ( command_value - 1511.0 ) / 4.5  );             // Every 4.5 "1"-s... ; Try: round( 
                SkidsMask = IndirectCh + 128;
                };      // ---------------- Second half of PWM Range

// GC_Debug2:
    AP::esc_telem().update_rpm( 9-1, SkidsMask * 1.0f , 0.0);                                // Feedback through Telemetry
        }       // -------------- One of Indirect Channels pre-selected:
        else
            {       // ++++++++++++++ check for "Fire" command
            if (  ( ControlVal >= 800 )  && ( ControlVal < 990 ) )                      // Got valid "Fire" command"
                {       // ++++++++++++++++++ Execute "Fire" command!

                if ( (SkidsMask<=255u) )                                 // Valid mask?...
                    {       // +++++++++++++++++++ Process Array of Outputs 
                    FireSkidsMask( SkidsMask );
                    Firing = true;
                    };      // ------------------- Process Array of Outputs                 
                };      // ------------------ Execute "Fire" command!
            };      // -------------- check for "Fire" command

    };      // ---------------------------- ProcessMultiSkids() ----------------------------
#endif          // ----  Use_ExtRC

void AP_Periph_FW::rcout_srv_PWM(uint8_t actuator_id, const float command_value)
{
    int32_t                 now_ms = AP_HAL::millis();
#if HAL_PWM_COUNT > 0
    const SRV_Channel::Function function = SRV_Channel::Function(SRV_Channel::k_rcin1 + actuator_id - 1);
    SRV_Channels::set_output_pwm(function, uint16_t(command_value+0.5));

#ifdef Use_ExtRC

// GC_Debug: Output to the pre-selected channel:
    if ( actuator_id < HAL_PWM_COUNT )
        {       // +++++++++++++++++++ Default output:        
        SRV_Channels::set_output_pwm(function, uint16_t(command_value+0.5));
        }       // ------------------- Default output:
        else
            {       // +++++++++++++++++++ GrayCat Extended : remapped Output
#ifdef GC_Remap19
            int  RemapCh = actuator_id - RC_RemapOfs;                                            // Do remap;
            if ( Remap19_en && (RemapCh >= RC_IndirCh1) && (RemapCh <= 13) )
                {
                const SRV_Channel::Function function_rm = SRV_Channel::Function(SRV_Channel::k_rcin1 + RemapCh - 1);            
                SRV_Channels::set_output_pwm(function_rm,  uint16_t(command_value+0.5) );        // Remapped
                };
#endif      //  GC_Remap19
            };      // ------------------- GrayCat Extended : remapped Output

// GC_Debug:   Pre-Select channel from RCIn16
    if ( 16 == actuator_id )
        {       // ............. Process channel_16 as Shifter:            
        int32_t ControlVal  = ( int(command_value) ) ;                                     // Extract controlling value (byte)...
// GC_Debug2:
        AP::esc_telem().update_rpm( 10-1, command_value , 0.0);                                // Feedback through Telemetry

            // ..... ReMap: .....
        if ( ControlVal >= 2299 )                                                          // PWM higher-then-High?...
            Remap19_en = 1;
            else
                {       // +++++++++++++++++++ Finished Remap19 state, zero-down
#ifndef GC_AlwaysRemap
                if ( Remap19_en )
                    {       // ++++++++++++++++++++ Was ReMapped, Undo!
                    for ( int ccnt=5; ccnt<=13 ; ccnt++)
                        {       // +++++++++++++++++++++++ Zero-down Outputs 
                        const SRV_Channel::Function function_z = SRV_Channel::Function(SRV_Channel::k_rcin1 + ccnt-1 );
                        SRV_Channels::set_output_limit( function_z, SRV_Channel::Limit::MIN );
                        };      // ----------------------- Zero-down Outputs
                    Channel_PreSet = -1;                                                              //  clear Pre-Select
                    };      // ------------------- Was ReMapped, Undo!
                // try without: 
                Remap19_en = 0;
#endif          // --  ndef GC_AlwaysRemap

                if ( CalcDiff_ContrVals( ControlVal ) )                                                          // PWM lower-then-min-SBUS ?...
                    {       // ++++++++++++++++++ Got Syndrome, Initiate Multi-Skid mode!
                    Skid_Mode = msk_Multi;                                                      // Start MultiSkid mode!
                    MultiSkid_TS = now_ms;                                                      // ReArm timeout;
                    }       // ------------------ Initiate Multi-Skid mode!
                    else
                        {       // +++++++++++++++++++ No Multi-Skid mode syndrome....
                        if ( (now_ms - MultiSkid_TS) > MultiSkidMode_TO )                           // TimeOut of MultiSkid mode expired?...
                            {       // +++++++++++++++++++ Finalize Multi-Skid mode!
                            Skid_Mode = msk_Single;                                                     // Start SingleSkid mode!
                            MultiSkid_TS = 0;
                            if ( Firing )
                                FireSkidsMask( 0 );                                                               // Release all!
                            Firing = false;
                            };      // ------------------- Finalize Multi-Skid mode!
                        }       // ------------------- No Multi-Skid mode syndrome....

                };      // ------------------- Finished Remap19 state, zero-down        

        switch (Skid_Mode) 
            {
            case msk_Single : ProcessSingleSkid( ControlVal ); break;
            case msk_Multi :  ProcessMultiSkids( ControlVal,  /* command_value */ AvgContrVal ); break;
            default: break;
            };      // -------------------- switch Skid_Mode         
        };      // ------------- Process channel_16 as Shifter:
#endif     // Use_ExtRC

    // Add to mask of channels that will be cleared if no commands are received
    actuator.mask |= SRV_Channels::get_output_channel_mask(function);

    rcout_has_new_data_to_update = true;
#if AP_SIM_ENABLED
    sim_update_actuator(actuator_id);
#endif
#endif
};      // ------------------------------- rcout_srv_PWM() -------------------------------------

void AP_Periph_FW::rcout_handle_safety_state(uint8_t safety_state)
{
    if (safety_state == 255) {
        hal.rcout->force_safety_off();
    } else {
        hal.rcout->force_safety_on();
    }
    rcout_has_new_data_to_update = true;
}

/// @brief Called from #can.cpp #can_update() :
void AP_Periph_FW::rcout_update()
{
    uint32_t now_ms = AP_HAL::millis();

    // Timeout for ESC commands
    const uint16_t esc_timeout_ms = g.esc_command_timeout_ms >= 0 ? g.esc_command_timeout_ms : 0; // Don't allow negative timeouts!
    const bool has_esc_rawcommand_timed_out = esc_timeout_ms != 0 && ((now_ms - last_esc_raw_command_ms) >= esc_timeout_ms);
    if (last_esc_num_channels > 0 && has_esc_rawcommand_timed_out) {
        // If we've seen ESCs previously, and a timeout has occurred, then zero the outputs
        int16_t esc_output[last_esc_num_channels];
        memset(esc_output, 0, sizeof(esc_output));
        rcout_esc(esc_output, last_esc_num_channels);                               // Here is our "ESC_Cmd -> UART" translator

        // Don't need to run again until new commands have been received
        last_esc_num_channels = 0;
    }

    // Timeout for servo actuator commands
    const uint16_t servo_timeout_ms = g.servo_command_timeout_ms >= 0 ? g.servo_command_timeout_ms : 0; // Don't allow negative timeouts!
    const bool has_servo_timed_out = servo_timeout_ms != 0 && ((now_ms - actuator.last_command_ms) >= servo_timeout_ms);
    if (has_servo_timed_out && (actuator.mask != 0)) {
#if HAL_PWM_COUNT > 0
        // Output 0 PWM for each channel in the mask
        for (uint8_t i = 0; i < HAL_PWM_COUNT; i++) {
            if (((1U<<i) & actuator.mask) != 0) {
                SRV_Channels::set_output_pwm_chan(i, 0);
            }
        }

        // register that the output has been changed
        rcout_has_new_data_to_update = true;
#endif

        // Don't need to run again until new commands have been received
        actuator.mask = 0;
    };      // ------------------------ if (has_servo_timed_out && (actuator.mask != 0))

// +-+- Moved from End-of-Function by GrayCat:
#if HAL_WITH_ESC_TELEM
    if (    (now_ms - last_esc_telem_update_ms >= esc_telem_update_period_ms) 
        
        )
        {
        last_esc_telem_update_ms = now_ms;
        esc_telem_update();
// GC_Debug:
// rcout_has_new_data_to_update = true;
        }
#if AP_EXTENDED_ESC_TELEM_ENABLED
    esc_telem_extended_update(now_ms);
#endif
#endif

    if (!rcout_has_new_data_to_update) {
        return;
    }
    rcout_has_new_data_to_update = false;

    auto &srv = AP::srv();
    SRV_Channels::calc_pwm();
    srv.cork();
    SRV_Channels::output_ch_all();
    srv.push();
}

#if AP_SIM_ENABLED
/*
  update simulation of servos, sending actuator status
*/
void AP_Periph_FW::sim_update_actuator(uint8_t actuator_id)
{
    sim_actuator.mask |= 1U << (actuator_id - 1);

    // send status at 10Hz
    const uint32_t period_ms = 100;
    const uint32_t now_ms = AP_HAL::millis();

    if (now_ms - sim_actuator.last_send_ms < period_ms) {
        return;
    }
    sim_actuator.last_send_ms = now_ms;

    for (uint8_t i=0; i<NUM_SERVO_CHANNELS; i++) {
        if ((sim_actuator.mask & (1U<<i)) == 0) {
            continue;
        }
        const SRV_Channel::Function function = SRV_Channel::Function(SRV_Channel::k_rcin1 + i);
        uavcan_equipment_actuator_Status pkt {};
        pkt.actuator_id = i + 1;
        // assume 45 degree angle for simulation
        pkt.position = radians(SRV_Channels::get_output_norm(function) * 45);
        pkt.force = 0;
        pkt.speed = 0;
        pkt.power_rating_pct = UAVCAN_EQUIPMENT_ACTUATOR_STATUS_POWER_RATING_PCT_UNKNOWN;

        uint8_t buffer[UAVCAN_EQUIPMENT_ACTUATOR_STATUS_MAX_SIZE];
        uint16_t total_size = uavcan_equipment_actuator_Status_encode(&pkt, buffer, !canfdout());

        canard_broadcast(UAVCAN_EQUIPMENT_ACTUATOR_STATUS_SIGNATURE,
                         UAVCAN_EQUIPMENT_ACTUATOR_STATUS_ID,
                         CANARD_TRANSFER_PRIORITY_LOW,
                         &buffer[0],
                         total_size);
    }
}
#endif // AP_SIM_ENABLED

#endif // AP_PERIPH_RC_OUT_ENABLED
