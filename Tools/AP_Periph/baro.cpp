#include "AP_Periph.h"

#if AP_PERIPH_BARO_ENABLED

/*
  barometer support
 */

#include <dronecan_msgs.h>

/*
  update CAN baro
 */
void AP_Periph_FW::can_baro_update(void)
{
    static int32_t      LastPackTS; 
    int32_t             now_ms = AP_HAL::millis();

    if (!periph.g.baro_enable) {
        return;
    }

    baro.update();
    if ( 0 && (last_baro_update_ms == baro.get_last_update() ) ) {
        return;
    }

// GC_Debug:
const float press = 1;              // baro.get_pressure();
    float temp_deg = 0.0;
    periph.temperature_sensor.get_temperature(temp_deg, /* index */ 0 ) ;
    const float temp = temp_deg    ; //baro.get_temperature();
    if ( ((now_ms - LastPackTS) & 0x7FFFFFFF ) > 501 )                       // Too long timeout?...
{       // +++++++++++++++ Transmit Baro Temp:
        uavcan_equipment_air_data_StaticTemperature pkt {};
        pkt.static_temperature = C_TO_KELVIN(temp);
        pkt.static_temperature_variance = 0; // should we make this a parameter?

        uint8_t buffer[UAVCAN_EQUIPMENT_AIR_DATA_STATICTEMPERATURE_MAX_SIZE];
        uint16_t total_size = uavcan_equipment_air_data_StaticTemperature_encode(&pkt, buffer, !periph.canfdout());

        canard_broadcast(UAVCAN_EQUIPMENT_AIR_DATA_STATICTEMPERATURE_SIGNATURE,
                        UAVCAN_EQUIPMENT_AIR_DATA_STATICTEMPERATURE_ID,
                        CANARD_TRANSFER_PRIORITY_LOW,
                        &buffer[0],
                        total_size);
// ................... Pressure:
            {
            uavcan_equipment_air_data_StaticPressure pktp {};
            pktp.static_pressure = press;
            pktp.static_pressure_variance = 0; // should we make this a parameter?

            uint8_t bufferp[UAVCAN_EQUIPMENT_AIR_DATA_STATICPRESSURE_MAX_SIZE];
            uint16_t total_sizep = uavcan_equipment_air_data_StaticPressure_encode(&pktp, bufferp, !periph.canfdout());

            canard_broadcast(UAVCAN_EQUIPMENT_AIR_DATA_STATICPRESSURE_SIGNATURE,
                            UAVCAN_EQUIPMENT_AIR_DATA_STATICPRESSURE_ID,
                            CANARD_TRANSFER_PRIORITY_LOW,
                            &bufferp[0],
                            total_sizep);
            }

        LastPackTS = now_ms;
};      // --------------- Transmit Baro Temp:

    last_baro_update_ms = baro.get_last_update();
// GC_Debug:
    if ( 0 &&  (!baro.healthy() )  )
        {
        // don't send any data
        return;
        }
    
    
    
}

#endif // AP_PERIPH_BARO_ENABLED
