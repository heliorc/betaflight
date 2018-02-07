/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include <platform.h>

#include "flight/pid.h"
#include "config/config_eeprom.h"
#include "common/filter.h"
#include "sensors/gyro.h"
#include "fc/config.h"

void targetConfiguration(void) {
    pidConfigMutable()->pid_process_denom = 1; // 16kHz PID
    gyroConfigMutable()->gyro_use_32khz = 1;
    gyroConfigMutable()->gyro_sync_denom = 2;  // 16kHz gyro
    gyroConfigMutable()->gyro_soft_lpf_hz = 0;
    gyroConfigMutable()->gyro_soft_notch_hz_1 = 0;
    gyroConfigMutable()->gyro_soft_notch_hz_2 = 0;

    for (uint8_t pidProfileIndex = 0; pidProfileIndex < MAX_PROFILE_COUNT; pidProfileIndex++) {
        pidProfile_t *pidProfile = pidProfilesMutable(pidProfileIndex);

        pidProfile->pid[PID_PITCH].P = 22;
        pidProfile->pid[PID_PITCH].I = 22;
        pidProfile->pid[PID_PITCH].D = 6;
        pidProfile->pid[PID_ROLL].P = 22;
        pidProfile->pid[PID_ROLL].I = 22;
        pidProfile->pid[PID_ROLL].D = 6;
        pidProfile->pid[PID_YAW].P = 22;
        pidProfile->pid[PID_YAW].I = 22;

        /* Setpoints */
        // pidProfile->dtermSetpointWeight = 100;
        // pidProfile->setpointRelaxRatio = 100; // default to snappy for racers
        pidProfile->itermAcceleratorGain = 3500;
        pidProfile->yaw_lpf_hz = 0;
        pidProfile->dterm_lpf_hz = 0;    
        pidProfile->dterm_notch_hz = 0;
        pidProfile->dterm_notch_cutoff = 0;
        pidProfile->dterm_filter_type = FILTER_PT1;
    }
}

