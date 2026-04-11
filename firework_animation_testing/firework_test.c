/**
 * Firework Animation Test Harness (C Implementation)
 *
 * This is a standalone C test program that compiles against firework.c/h
 * and provides mock implementations of hardware dependencies.
 *
 * Compile with:
 *   gcc -o firework_test firework_test.c ../main/firework.c ../main/fill.c \
 *       -I../main/include -Imock_headers -lm
 *
 * Run with:
 *   ./firework_test [DURATION_SECONDS]
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

/* Include flipdot hardware definitions and utilities */
#include "flipdot.h"
#include "fill.h"
#include "firework.h"

/* ============================================================================
 * Global Test State
 * ============================================================================ */

static uint32_t frame_number = 0;
static time_t start_time_sec = 0;
static long start_time_nsec = 0;
static bool should_exit = false;
static uint8_t next_scale = 0;        /* Scale that will be used for the next firework */
static int next_trigger_frame = 0;    /* Frame number at which the next firework fires */
static int next_trigger_interval = 0; /* Interval in frames until the next trigger */

/* ============================================================================
 * Hardware Stub Implementations
 * ============================================================================ */

/**
 * Mock write_dotboard: Required for linking against firework.c but test
 * reads board state directly via firework_get_board() instead.
 */
void write_dotboard(dotboard_t *dots, bool is_keyframe)
{
    (void)dots;        /* Unused parameter */
    (void)is_keyframe; /* Unused parameter */
    /* Test harness reads board state directly via firework_get_board() */
}

/* ============================================================================
 * Visualization Functions
 * ============================================================================ */

/**
 * Get elapsed time since test started in seconds.
 */
static double get_elapsed_seconds(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);

    double elapsed = (now.tv_sec - start_time_sec);
    elapsed += (now.tv_usec - start_time_nsec) / 1.0e6;

    return elapsed;
}

/**
 * Clear the terminal screen (Unix-like systems).
 */
static void clear_screen(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}

/**
 * Render the current frame to the terminal with ASCII art.
 */
static void render_frame_terminal(void)
{
    clear_screen();

    /* Get current board state from firework engine */
    const dotboard_t *board = firework_get_board();

    /* Print header with frame info */
    double elapsed = get_elapsed_seconds();
    int frames_until_next = next_trigger_frame - (int)frame_number;
    if (frames_until_next < 0)
        frames_until_next = 0;
    double secs_until_next = frames_until_next * 0.05;

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  Firework Animation Test                                       ║\n");
    printf("║  Frame: %-6u  |  Time: %.2fs  |  Dots: %d×%d (landscape)    ║\n",
           frame_number, elapsed, DOT_ROWS, DOT_COLUMNS);
    if (next_scale > 0)
    {
        printf("║  Next scale: %-2d/10  |  Next firework in: %.1fs                  ║\n",
               next_scale, secs_until_next);
    }
    else
    {
        printf("║  Waiting for first firework...                                ║\n");
    }
    printf("╠════════════════════════════════════════════════════════════════╣\n");

    /* Print the board (landscape orientation: rows as width, columns as height) */
    /* Column labels (top) */
    printf("║    ");
    for (uint8_t row = 0; row < DOT_ROWS; row++)
    {
        printf(" %X", (row / 10) % 10);
    }
    printf("    \t\t║\n");

    printf("║    ");
    for (uint8_t row = 0; row < DOT_ROWS; row++)
    {
        printf(" %X", row % 10);
    }
    printf("     \t\t║\n");

    /* Render each column as a row (transposed landscape view) */
    for (uint8_t col = 0; col < DOT_COLUMNS; col++)
    {
        printf("║ %2d ", col);
        for (uint8_t row = 0; row < DOT_ROWS; row++)
        {
            if ((*board)[col][row])
            {
                printf(" █");
            }
            else
            {
                printf(" ·");
            }
        }
        printf(" %2d\t\t║\n", col);
    }

    printf("║    ");
    for (uint8_t row = 0; row < DOT_ROWS; row++)
    {
        printf(" %X", row % 10);
    }
    printf("    \t\t║\n");

    printf("║    ");
    for (uint8_t row = 0; row < DOT_ROWS; row++)
    {
        printf(" %X", (row / 10) % 10);
    }
    printf("    \t\t║\n");

    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║  Legend: █ = LED On   |  · = LED Off                           ║\n");
    printf("║  Controls: Press Ctrl+C to stop animation                      ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");

    fflush(stdout);
}

/**
 * Handle Ctrl+C gracefully.
 */
static void signal_handler(int sig)
{
    (void)sig;
    should_exit = true;
}

/* ============================================================================
 * Main Test Program
 * ============================================================================ */

int main(int argc, char *argv[])
{
    int duration_seconds = 30;

    /* Parse command line arguments */
    if (argc > 1)
    {
        duration_seconds = atoi(argv[1]);
        if (duration_seconds <= 0)
        {
            fprintf(stderr, "Error: duration must be a positive number\n");
            return 1;
        }
    }

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  Firework Animation Test (C Implementation)                    ║\n");
    printf("║                                                                ║\n");
    printf("║  Display: %d×%d dots (landscape)                             ║\n",
           DOT_ROWS, DOT_COLUMNS);
    printf("║  Duration: %d seconds                                          ║\n",
           duration_seconds);
    printf("║  Frame Rate: 50ms per frame                                    ║\n");
    printf("║                                                                ║\n");
    printf("║  Press Ctrl+C to stop                                          ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\nInitializing firework system...\n");

    /* Set up signal handler for clean exit */
    signal(SIGINT, signal_handler);

    /* Seed the random number generator */
    srand((unsigned int)time(NULL));

    /* Pre-generate the scale for the first firework */
    next_scale = 1 + rand() % 10;

    /* Initialize board and timing */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    start_time_sec = tv.tv_sec;
    start_time_nsec = tv.tv_usec;

    /* Initialize the firework animation system */
    firework_init();

    printf("Animation starting...\n");
    sleep(1);

    /* Main animation loop */
    int total_frames = (duration_seconds * 1000) / 50; /* 50ms per frame */

    for (int frame = 0; frame < total_frames && !should_exit; frame++)
    {
        frame_number = frame;

        /* Trigger a new firework if it's time */
        if (frame >= next_trigger_frame)
        {
            trigger_firework(next_scale);

            /* Schedule the next trigger: random interval of 1-10 seconds */
            next_trigger_interval = (1 + rand() % 10) * 20; /* 1-10s at 20fps */
            next_trigger_frame = frame + next_trigger_interval;

            /* Pre-generate scale for the following firework */
            next_scale = 1 + rand() % 10;
        }

        /* Update the animation */
        firework_update();

        /* Render every frame */
        render_frame_terminal();

        /* Sleep for frame time (50ms) */
        usleep(50000);
    }

    /* Print final statistics */
    double final_elapsed = get_elapsed_seconds();
    clear_screen();

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  Animation Test Complete                                       ║\n");
    printf("║                                                                ║\n");
    printf("║  Total Frames Rendered: %u                                    ║\n",
           frame_number);
    printf("║  Total Time: %.2f seconds                                      ║\n",
           final_elapsed);
    printf("║  Average FPS: %.2f                                             ║\n",
           frame_number / final_elapsed);
    printf("║                                                                ║\n");
    printf("║  Test completed successfully!                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    return 0;
}
