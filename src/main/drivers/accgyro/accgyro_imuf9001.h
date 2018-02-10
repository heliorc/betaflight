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

#pragma once

#define WHOAMI_9001 101

#include "drivers/bus.h"

int imuf9001SpiDetect(const busDevice_t *bus);
int imuf9001AccDetect(accDev_t *acc);
int imuf9001GyroDetect(gyroDev_t *gyro);

void imuf9001SpiAccInit(accDev_t *acc);
void imuf9001SpiGyroInit(gyroDev_t *gyro);
