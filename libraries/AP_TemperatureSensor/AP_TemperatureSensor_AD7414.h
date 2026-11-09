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

#pragma once

#include "AP_TemperatureSensor_Backend.h"

#if AP_TEMPERATURE_SENSOR_AD7414_ENABLED

class AP_TemperatureSensor_AD7414 : public AP_TemperatureSensor_Backend {
    using AP_TemperatureSensor_Backend::AP_TemperatureSensor_Backend;

public:
    __INITFUNC__ void init() override;
    void update() override;

private:
    uint32_t _last_read_ms;
};

#endif // AP_TEMPERATURE_SENSOR_AD7414_ENABLED
