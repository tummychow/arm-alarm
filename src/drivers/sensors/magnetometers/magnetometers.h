/**************************************************************************/
/*!
    @file     magnetometers.h
    @author   Nguyen Quang Huy, Nguyen Thien Tin
    @ingroup  Sensors

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2013, K. Townsend
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/
#ifndef _MAGNETOMETERS_H_
#define _MAGNETOMETERS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "projectconfig.h"
#include "drivers/sensors/sensors.h"

#define SENSORS_CAL_MAG_DATA_PRESENT     (1 << 0)

typedef struct
{
  float32_t scale;   /**< scale factor */
  float32_t offset;  /**< offset error */
} mag_calib_params_t;

typedef struct
{
  uint16_t config;
  uint16_t sensorID;
  mag_calib_params_t x;
  mag_calib_params_t y;
  mag_calib_params_t z;
} mag_calib_data_t;

error_t magLoadCalData        ( mag_calib_data_t *calib_data );
error_t magCalibrateEventData ( sensors_axis_t axis, sensors_event_t *event, mag_calib_data_t *calib_data);
error_t magTiltCompensation   ( sensors_axis_t axis, sensors_event_t *mag_event, sensors_event_t *accel_event );
error_t magGetOrientation     ( sensors_axis_t axis, sensors_event_t *event, sensors_vec_t *mag_orientation );

#ifdef __cplusplus
}
#endif

#endif // _MAGNETOMETERS_H_

