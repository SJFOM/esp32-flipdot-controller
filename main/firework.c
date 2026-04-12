#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "flipdot.h"
#include "fill.h"
#include "firework.h"

static const char *TAG = "Firework";

// Maximum number of particles per explosion
#define MAX_PARTICLES_PER_EXPLOSION 16

// Maximum number of simultaneous explosions
#define MAX_EXPLOSIONS 3

// Base lifespan of a particle in iterations (scale 1)
#define PARTICLE_LIFESPAN_MIN 20

// Maximum lifespan of a particle in iterations (scale 10)
#define PARTICLE_LIFESPAN_MAX 60

// Particle structure
typedef struct
{
    float x;      // Current X position
    float y;      // Current Y position
    float vx;     // Velocity in X direction
    float vy;     // Velocity in Y direction
    uint8_t life; // Remaining lifespan (0 = dead)
} particle_t;

// Explosion structure
typedef struct
{
    particle_t particles[MAX_PARTICLES_PER_EXPLOSION];
    uint8_t particle_count;
    bool active;
} explosion_t;

// Global state
static dotboard_t firework_board;
static explosion_t explosions[MAX_EXPLOSIONS];

/**
 * Initialize a new explosion at the given center point with scale-derived parameters.
 * particle_count: number of particles to emit (1-MAX_PARTICLES_PER_EXPLOSION)
 * speed_max: maximum particle launch speed
 * lifespan: particle lifespan in iterations
 */
static void create_explosion(uint8_t center_x, uint8_t center_y,
                             uint8_t particle_count, float speed_max, uint8_t lifespan)
{
    // Find an inactive explosion slot
    for (uint8_t e = 0; e < MAX_EXPLOSIONS; e++)
    {
        if (!explosions[e].active)
        {
            explosions[e].active = true;
            explosions[e].particle_count = particle_count;

            // Create particles radiating outward
            for (uint8_t i = 0; i < particle_count; i++)
            {
                // Distribute particles evenly around 360 degrees
                float angle = (2.0f * 3.14159265f * i) / particle_count;

                // Speed varies between half and full speed_max
                float speed = (speed_max * 0.5f) + ((rand() % 100) / 100.0f) * (speed_max * 0.5f);

                explosions[e].particles[i].x = (float)center_x;
                explosions[e].particles[i].y = (float)center_y;
                explosions[e].particles[i].vx = cosf(angle) * speed;
                explosions[e].particles[i].vy = sinf(angle) * speed;
                explosions[e].particles[i].life = lifespan;
            }

            // Zero out unused particle slots
            for (uint8_t i = particle_count; i < MAX_PARTICLES_PER_EXPLOSION; i++)
            {
                explosions[e].particles[i].life = 0;
            }
            break;
        }
    }
}

/**
 * Update all active particles.
 * Move particles, decay their lifespan, and mark explosions as inactive when empty.
 */
static void update_particles()
{
    for (uint8_t e = 0; e < MAX_EXPLOSIONS; e++)
    {
        if (!explosions[e].active)
            continue;

        for (uint8_t p = 0; p < explosions[e].particle_count; p++)
        {
            particle_t *particle = &explosions[e].particles[p];

            if (particle->life > 0)
            {
                // Update position
                particle->x += particle->vx;
                particle->y += particle->vy;

                // Apply slight gravity/damping
                particle->vy += 0.1f;

                // Decay lifespan
                particle->life--;
            }
        }

        // Check if all particles are dead
        bool has_living_particles = false;
        for (uint8_t p = 0; p < explosions[e].particle_count; p++)
        {
            if (explosions[e].particles[p].life > 0)
            {
                has_living_particles = true;
                break;
            }
        }
        explosions[e].active = has_living_particles;
    }
}

/**
 * Render all active particles to the firework board.
 */
static void render_particles()
{
    // Clear the board
    fill_on_off(&firework_board, false);

    // Draw each active particle
    for (uint8_t e = 0; e < MAX_EXPLOSIONS; e++)
    {
        if (!explosions[e].active)
            continue;

        for (uint8_t p = 0; p < explosions[e].particle_count; p++)
        {
            particle_t *particle = &explosions[e].particles[p];

            if (particle->life == 0)
                continue;

            // Get integer coordinates
            int32_t x = (int32_t)particle->x;
            int32_t y = (int32_t)particle->y;

            // Check bounds and draw if valid
            if (x >= 0 && x < DOT_COLUMNS && y >= 0 && y < DOT_ROWS)
            {
                firework_board[x][y] = 1;
            }
        }
    }

    // Write the board to the display
    write_dotboard(&firework_board, false);
}

void firework_init()
{
    ESP_LOGI(TAG, "Firework animation initialized");

    // Initialize all explosions as inactive
    for (uint8_t e = 0; e < MAX_EXPLOSIONS; e++)
    {
        explosions[e].active = false;
        explosions[e].particle_count = 0;
    }

    // Clear the board
    fill_on_off(&firework_board, false);
}

void trigger_firework(uint8_t scale)
{
    // Clamp scale to valid range 1-10
    if (scale < 1)
        scale = 1;
    if (scale > 10)
        scale = 10;

    // Derive explosion parameters from scale
    uint8_t particle_count = 4 + ((scale - 1) * (MAX_PARTICLES_PER_EXPLOSION - 4)) / 9;
    float speed_max = 1.0f + (scale - 1) * 0.33f;               // 1.0 (scale 1) to ~4.0 (scale 10)
    uint8_t lifespan = PARTICLE_LIFESPAN_MIN + (scale - 1) * 4; // 20 (scale 1) to 56 (scale 10)

    uint8_t center_x = rand() % DOT_COLUMNS;
    uint8_t center_y = rand() % (DOT_ROWS/2); // Keep fireworks in upper half for better visibility

    ESP_LOGI(TAG, "Triggering firework: scale=%d, particles=%d, speed_max=%.1f, lifespan=%d",
             scale, particle_count, speed_max, lifespan);

    create_explosion(center_x, center_y, particle_count, speed_max, lifespan);
}

void firework_update()
{
    // Update particle positions and lifespan
    update_particles();

    // Render current state to display
    render_particles();
}

const dotboard_t *firework_get_board()
{
    return &firework_board;
}
