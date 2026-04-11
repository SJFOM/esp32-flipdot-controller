#include <stdlib.h>
#include "flipdot.h"

/**
 * Set/Unset the entire dotboard.
 */
void fill_on_off(dotboard_t* dotboard, bool on_off)
{
  for (uint8_t c = 0; c < DOT_COLUMNS; c ++) {
    for (uint8_t r = 0; r < DOT_ROWS; r ++) {
      (*dotboard)[c][r] = on_off ? 1 : 0;
    }
  }
}