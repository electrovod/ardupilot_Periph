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

#include "AP_TemperatureSensor_config.h"

#if AP_TEMPERATURE_SENSOR_AD7414_ENABLED

#include "AP_TemperatureSensor_AD7414.h"

#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/I2CDevice.h>

extern const AP_HAL::HAL &hal;

#define AD7414_REG_TEMPERATURE  0x00
#define AD7414_DEFAULT_ADDRESS  0x49
#define AD7414_UPDATE_INTERVAL_MS 1000U

void AP_TemperatureSensor_AD7414::init()
{
    // AD7414-0 with AS tied to GND uses 7-bit I2C address 0x49.
    // AD7414-1 with AS tied to GND uses 0x4D; override TEMP1_ADDR if required.
    _params.bus_address.set_default(AD7414_DEFAULT_ADDRESS);

    _dev = hal.i2c_mgr->get_device_ptr(_params.bus, _params.bus_address);
    if (_dev == nullptr) {
        return;
    }

    _dev->set_retries(3);
}

void AP_TemperatureSensor_AD7414::update()
{
    if (_dev == nullptr) {
        return;
    }

    const uint32_t now_ms = AP_HAL::millis();
    if (now_ms - _last_read_ms < AD7414_UPDATE_INTERVAL_MS) {
        return;
    }
    _last_read_ms = now_ms;

    uint8_t data[2];
    {
        WITH_SEMAPHORE(_dev->get_semaphore());
        if (!_dev->read_registers(AD7414_REG_TEMPERATURE, data, sizeof(data))) {
            return;
        }
    }

    // AD7414 register layout:
    // first byte:  B8..B2 in bits 7..0
    // second byte: B1 in bit 7, B0 in bit 6; lower bits are status flags/reserved.
    const uint16_t raw10 = (uint16_t(data[0]) << 2) | (data[1] >> 6);

    // Sign-extend the 10-bit two's-complement value.
    const int16_t signed_raw = (raw10 & 0x0200U) ?
        int16_t(raw10 - 1024U) : int16_t(raw10);

    // One LSB = 0.25 degrees Celsius.
    set_temperature(float(signed_raw) * 0.25f);
}

#endif // AP_TEMPERATURE_SENSOR_AD7414_ENABLED
