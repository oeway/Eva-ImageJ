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

#include "sync_control.h"
#include "nuts_bolts.h"
#include <avr/io.h>

uint32_t sync_step = 100;  
uint8_t sync_axis = X_AXIS;
uint32_t current_sync_step=0;
uint32_t half_sync_step=50;
void sync_init()
{
   SYNC_CONTROL_DDR |= (1 << SYNC_CONTROL_BIT);
}
/*
void set_sync()
{
  bit_false(SYNC_CONTROL_PORT,bit(SYNC_CONTROL_BIT));
}
void reset_sync()
{
  bit_true(SYNC_CONTROL_PORT,bit(SYNC_CONTROL_BIT));
}
void toggle_sync()
{
  bit_toggle(SYNC_CONTROL_PORT,bit(SYNC_CONTROL_BIT));
}
*/