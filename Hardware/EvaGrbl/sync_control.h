/*
  sync_control.h 

  Copyright (c) 2013 Wei Ouyang

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef sync_control_h
#define sync_control_h 
#include "nuts_bolts.h"
#include <avr/io.h>

extern uint32_t sync_step;  
extern uint8_t sync_axis;
extern uint32_t current_sync_step;
extern uint32_t half_sync_step;
void sync_init();
//void sync_set();
//void sync_reset();
//void toggle_sync();
#endif