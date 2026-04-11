#ifndef FIREWORK_H
#define FIREWORK_H

#include "flipdot.h"

/**
 * Initialize the firework animation system.
 * Must be called before firework_update().
 */
void firework_init();

/**
 * Trigger a firework explosion at a random position.
 * scale: explosion size from 1 (small) to 10 (large).
 * Affects particle count, launch speed, and lifespan.
 * Must be called by the caller to initiate each explosion.
 */
void trigger_firework(uint8_t scale);

/**
 * Update the firework animation.
 * Call this repeatedly from the main loop.
 * Does NOT auto-trigger explosions — call trigger_firework() separately.
 */
void firework_update();

/**
 * Get the current firework board state.
 * Returns a const pointer to the internal board data structure.
 * Useful for testing and reading display state without hardware access.
 */
const dotboard_t *firework_get_board();

#endif
