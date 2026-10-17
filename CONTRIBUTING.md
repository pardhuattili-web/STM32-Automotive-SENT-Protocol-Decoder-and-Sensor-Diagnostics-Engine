# Contributing

## Local workflow

1. Create a topic branch for each change.
2. Build with CMake and run CTest before submitting.
3. Keep the protocol decoder independent of STM32 HAL headers.
4. Add regression tests for new timing, frame-layout, CRC, or error-handling behavior.
5. Do not claim hardware validation without recording the exact MCU, sensor, setup, raw capture, and observed results.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Reporting a bug

Include the compiler, host OS, commit SHA, test command/output, sensor profile when applicable, and a minimal capture fixture. Redact proprietary or safety-sensitive device data before posting public traces.

## Protocol changes

Document tick interpretation, accepted frame layout, CRC seed/table, nibble-inclusion rules, and tolerance. Never silently change the default profile without updating tests and documentation.
