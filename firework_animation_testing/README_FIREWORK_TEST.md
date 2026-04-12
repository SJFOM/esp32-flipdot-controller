# Firework Animation C Test Harness

This is a standalone C test application that compiles directly against `firework.c/h` and provides mock implementations of all hardware dependencies, allowing you to test the firework animation without physical hardware.

## Features

- **Direct Compilation**: Compiles `firework.c` and `fill.c` directly - no cross-compilation needed
- **Complete Hardware Mocking**: Provides all necessary mocks for:
  - FreeRTOS (`vTaskDelay`)
  - ESP-IDF logging (`ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE`)
  - Hardware interface (`write_dotboard`)
- **ASCII Visualization**: Real-time terminal display of the dot matrix animation (20×14)
- **Random Triggering**: Fireworks fire at random intervals (1–10 seconds) with random scale (1–10)
- **Scale-Based Explosions**: Each firework uses `trigger_firework(scale)` — scale controls particle count, speed, and lifespan
- **Performance Metrics**: Shows frame count, elapsed time, and FPS
- **Customizable Duration**: Run tests for any duration

## Compilation

### Using the Makefile (Recommended)

```bash
# Build the test executable
make -f Makefile.firework

# Build and run for 30 seconds (default)
make -f Makefile.firework run

# Build and run for a custom duration
make -f Makefile.firework run-60    # 60 seconds
make -f Makefile.firework run-120   # 2 minutes
```

### Manual Compilation

```bash
# Compile with gcc
gcc -std=c99 -Wall -Wextra -O2 -o firework_test firework_test.c ../main/firework.c ../main/fill.c \
    -I../main/include -Imock_headers -lm

# Run the test
./firework_test [DURATION_SECONDS]
```

## Usage

### Basic Usage (Default 30 seconds)
```bash
./firework_test
```

### Custom Duration
```bash
# Run for 60 seconds
./firework_test 60

# Run for 2 minutes
./firework_test 120
```

### Interactive Features
- **Ctrl+C**: Stop the animation at any time
- **Terminal Output**: Real-time display of:
  - Current frame number and elapsed time
  - Next firework scale (1–10) and countdown to next trigger
  - 20×14 dot matrix in landscape orientation
  - `█` = LED ON, `·` = LED OFF

## Example Output

```
╔════════════════════════════════════════════════════════════════╗
║  Firework Animation Test                                       ║
║  Frame: 2       |  Time: 1.11s  |  Dots: 20×14               ║
║  Next scale: 7 /10  |  Next firework in: 4.5s                  ║
╠════════════════════════════════════════════════════════════════╣
║     0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1                   ║
║     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9                   ║
║  0  · · · · · · · · · · · · · · · · · · · ·  0                ║
║  1  · · · · · · · · · · █ · █ · · · · · · ·  1                ║
║  2  · · · · · · · · · · · · · · · · · · · ·  2                ║
║  3  · · · · · · · · █ · · · · █ · · · · · ·  3                ║
║  4  · · · · · · · █ · · · · · · · · · · · ·  4                ║
║  5  · · · · █ · █ · █ · · · · · · █ · · · ·  5                ║
║  6  · · · · · · █ · · █ · · · · · · · · · ·  6                ║
║  7  · · · █ █ · · · · · · · · · · █ · · · ·  7                ║
║  8  · █ · · · · · █ · · █ · · █ · · · · · ·  8                ║
║  9  · · · · · · · █ · · · · · · · · · · · ·  9                ║
║ 10  · █ · · · · · · · · █ · · █ · · · · · · 10                ║
║ 11  · · · █ · · · · █ · █ · █ · · · · · · · 11                ║
║ 12  · · · · · · · · █ · · · · · · · · · · · 12                ║
║ 13  · · · █ █ · · · · · · · · · · · · · · · 13                ║
║     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9                   ║
║     0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1                   ║
╠════════════════════════════════════════════════════════════════╣
║  Legend: █ = LED On   |  · = LED Off                           ║
║  Controls: Press Ctrl+C to stop animation                      ║
╚════════════════════════════════════════════════════════════════╝
```

## Display Format

The display is rendered in **landscape orientation** (20 wide × 14 tall):

- **Header Row**: Current frame number, elapsed time, and matrix dimensions
- **Scale Row**: Next firework scale (1–10) and countdown in seconds to the next trigger
- **Column/Row Numbers**: Decimal coordinates for easy reference
- **Matrix Grid**: Visual representation of LED states (20 rows × 14 columns, transposed)
- **Statistics**: FPS and timing information on completion

## What Gets Tested

✓ Particle creation and physics  
✓ `trigger_firework(scale)` API with scale 1–10  
✓ Scale-derived particle count, launch speed, and lifespan  
✓ Explosion generation at random positions  
✓ Particle velocity and gravity simulation  
✓ Particle lifespan management  
✓ Boundary checking  
✓ Board rendering logic via `firework_get_board()`  
✓ Random trigger intervals (1–10 seconds)  
✓ Multiple simultaneous explosions (up to 3)

## Architecture

### Mock Components

1. **FreeRTOS Mocks**
   - `portTICK_PERIOD_MS`: Timer tick constant
   - `vTaskDelay()`: Calls `usleep()` for equivalent delays

2. **ESP-IDF Logging Mocks**
   - `ESP_LOGI()`: Prints to stdout
   - `ESP_LOGW()`: Prints warnings
   - `ESP_LOGE()`: Prints errors to stderr

3. **Hardware Mocks**
   - `write_dotboard()`: No-op stub — board state is read directly via `firework_get_board()`
   - `fill_on_off()`: Real implementation compiled from `fill.c`

### `trigger_firework(scale)` API

Explosions are not auto-triggered. The caller controls when and how large each firework is:

| Scale | Particles | Max Speed | Lifespan |
|-------|-----------|-----------|----------|
| 1     | 4         | 1.0       | 20 frames (~1.0s) |
| 5     | 10        | 2.3       | 36 frames (~1.8s) |
| 10    | 16        | 4.0       | 56 frames (~2.8s) |

The test harness fires at random scale (1–10) every 1–10 seconds. On the device, `main.c` fires at a random scale every 10 seconds.

### Test Harness Features

- **Timing**: Uses `gettimeofday()` for elapsed time tracking
- **Rendering**: Terminal clear and frame rendering each iteration
- **Signal Handling**: Graceful exit on Ctrl+C
- **Statistics**: Frame counting and performance metrics

## Validation Checklist

After running the test, verify:

- [ ] Animation initialises without errors
- [ ] First firework triggers immediately at frame 0
- [ ] Subsequent fireworks trigger at random 1–10 second intervals
- [ ] Header shows correct "Next scale" and countdown
- [ ] Scale visibly affects explosion size (scale 10 is larger than scale 1)
- [ ] Particles radiate from random explosion centres
- [ ] Particles fade out over time (don't stay on forever)
- [ ] Particle trails show gravity effect (particles arc downward)
- [ ] Multiple explosions can occur simultaneously (up to 3)
- [ ] No particles appear outside the 20×14 bounds
- [ ] Smooth animation with no jitter
- [ ] Program exits cleanly with Ctrl+C
- [ ] Final statistics show reasonable FPS (~20 FPS at 50ms per frame)

## Troubleshooting

### Compilation Errors

**Missing math library**
```
undefined reference to `cosf', `sinf'
```
Solution: Ensure `-lm` flag is included (it's in the Makefile)

**Missing Headers**
```
fatal error: flipdot.h: No such file or directory
```
Solution: Ensure `-I../main/include` is in compilation flags

### Runtime Issues

**Animation appears frozen**
- Check that your terminal supports ANSI escape codes
- Try running on a different terminal emulator
- Resize terminal to ensure it's large enough (80×24 minimum)

**No output visible**
- Ensure terminal is at least 80×24 characters
- Try expanding your terminal window

## Performance Expectations

On a modern system:
- **FPS**: ~20 frames per second (50ms per frame)
- **CPU**: Minimal (animation is lightweight)
- **Memory**: ~5 KB total
- **Startup**: < 100ms

## Files Included

- `firework_test.c` - Test harness and visualization
- `Makefile.firework` -Build configuration
- `mock_headers/` - Mock ESP-IDF and FreeRTOS headers
  - `esp_log.h` - Logging macros
  - `freertos/FreeRTOS.h` - FreeRTOS stubs
  - `freertos/task.h` - Task function stubs

## Next Steps

After validating with this test:

1. Review logs for any warnings or errors
2. Verify animation behavior matches expectations
3. Compile the full ESP-IDF project
4. Flash to device and test with physical hardware
5. Adjust timing/parameters if needed