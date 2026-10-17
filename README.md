# STM32 Automotive SENT Protocol Decoder & Sensor Diagnostics Engine

[![Host tests](https://github.com/pardhuattili-web/STM32-Automotive-SENT-Protocol-Decoder-and-Sensor-Diagnostics-Engine/actions/workflows/ci.yml/badge.svg)](https://github.com/pardhuattili-web/STM32-Automotive-SENT-Protocol-Decoder-and-Sensor-Diagnostics-Engine/actions/workflows/ci.yml)
![Language](https://img.shields.io/badge/language-C11-blue)
![Target](https://img.shields.io/badge/target-STM32%20%7C%20host-lightgrey)
![Status](https://img.shields.io/badge/status-host%20core%20%2B%20hardware%20port-orange)

A portable Embedded C core for decoding SENT (Single Edge Nibble Transmission) pulse captures, checking frame integrity, and tracking sensor-link diagnostics. Protocol logic is separated from STM32 timer/DMA acquisition so the decoder can be compiled and tested on a host before target integration.

> **Validation status:** the portable core and synthetic host tests are the first milestone. No physical STM32 or sensor validation is claimed yet. Configure the timing and CRC profile from the selected sensor documentation before using real captures.

## Features

- Configurable timer-count-to-nibble conversion with timing tolerance.
- Sync-window validation and bounded short-frame parsing.
- CRC-4 verification through a configurable 16-entry transition table.
- Diagnostics for valid frames, sync errors, pulse errors, CRC errors, and timeouts.
- Deterministic synthetic capture generator for development and regression tests.
- CMake build, CTest test suite, strict compiler warnings, and GitHub Actions CI.
- Documented interface boundary for future STM32 timer input-capture/DMA integration.

## Architecture

```text
SENT sensor signal
      |
Timer input capture + DMA (target adapter; planned)
      |
Edge intervals in timer counts
      |
Portable decoder: sync -> nibble timing -> CRC
      |
Decoded status/data + diagnostics counters
      |
Application calibration / UART reporting (target integration)
```

## Repository layout

```text
include/sent_decoder.h       Public portable API
src/sent_decoder.c           Pulse decoder, CRC gate and diagnostics
simulator/sent_sim.h         Synthetic capture generator API
simulator/sent_sim.c         Frame-to-interval simulator
app/main.c                   Host demonstration program
tests/test_sent_decoder.c    Deterministic unit tests
docs/architecture.md         Design and STM32 integration boundary
docs/test-plan.md             Fault matrix and hardware acceptance plan
docs/benchmarks.md            Results template (no invented measurements)
.github/workflows/ci.yml      Automated build and tests
```

## Build, test and run

Requirements: CMake 3.16+, GCC or Clang with C11 support, and CTest.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/sent_demo
```

The project compiles with `-Wall -Wextra -Wpedantic -Werror` by default. To turn off warnings-as-errors explicitly:

```bash
cmake -S . -B build-relaxed -DSENT_ENABLE_WERROR=OFF
cmake --build build-relaxed --parallel
```

## Frame and capture model

The starter profile expects six interval counts: one synchronization interval followed by five nibble pulses representing status, three data nibbles, and CRC. Data pulse timing is modeled as:

```text
interval_counts ≈ (12 + nibble_value) × nominal_tick_counts
```

The synthetic example uses a sync interval of 56 nominal ticks and configurable min/max sync limits. Interval counts are rounded to the nearest nominal tick and rejected if the residual exceeds the configured tolerance.

**This is an intentionally narrow software test profile, not a universal decoder for every SENT sensor.** Real configurations may differ in frame length, pause pulse handling, serial-message channel behavior, tick timing, edge convention, and CRC rules. Expand or specialize the frame profile for the selected device.

### CRC configuration

CRC variants depend on the applicable SENT profile and sensor. This project accepts an explicit 16-entry transition table, seed, and status-nibble inclusion setting; the example table exists to make the simulated pipeline testable. Before using real data, populate the parameters from the sensor datasheet and applicable SENT specification, and add regression fixtures from a known waveform.

## STM32 integration plan

Suggested target: an STM32 Nucleo board whose exact MCU supports timer input capture and a suitable DMA route. Timer, pin, DMA channel, and counter width depend on the specific part and board.

1. Verify sensor output voltage and provide appropriate input conditioning.
2. Configure timer capture resolution for the sensor's nominal tick.
3. Convert consecutive capture register values to unsigned delta counts and account for timer rollover.
4. Use DMA circular/double buffering if available; keep interrupt handlers short.
5. Pass a completed capture window to `sent_decoder_process()` from foreground code or a dedicated RTOS task.
6. Report decoded values and counters over a non-blocking or rate-limited UART path.
7. Verify with a known-good physical trace and document the sensor, board, timing, CRC profile, and test results.

See [docs/architecture.md](docs/architecture.md) and [docs/test-plan.md](docs/test-plan.md).

## Test coverage

The included host tests cover:
- valid synthetic frame decoding and diagnostics,
- invalid CRC,
- out-of-range synchronization interval,
- impossible data interval,
- invalid arguments,
- timeout notification,
- invalid configuration.

Run all tests with `ctest --test-dir build --output-on-failure`. CI repeats the host build, tests, and demo for pushes and pull requests.

## Roadmap

- [x] Portable C decoder core and API
- [x] Synthetic capture generator
- [x] Unit tests and strict-warning host build
- [x] GitHub Actions CI
- [x] Architecture, test plan, and validation record template
- [ ] Board-specific STM32CubeIDE timer/DMA adapter
- [ ] Sensor-specific timing and CRC profile independently verified
- [ ] Physical waveform capture and regression fixture
- [ ] Measured processing latency and fault-injection evidence

## Safety and intended use

Educational embedded-firmware and protocol-decoding prototype only. It is not certified or safety-qualified, and must not be used to control safety-critical vehicle functions.

## License

MIT. See [LICENSE](LICENSE).
