# Architecture and STM32 Integration

## Software boundary
The portable core consumes a bounded window of timer interval counts. It has no STM32 HAL calls, heap allocation, UART output, or interrupt-specific logic.

## Capture profile in this prototype
- Input: interval counts between equivalent active edges.
- Window: sync interval, status nibble, three data nibbles, CRC nibble.
- Data pulse convention: approximately `(12 + nibble) * nominal_tick_counts`.
- Example sync interval: 56 nominal ticks.
- Nibble intervals are rounded to the nearest nominal tick and rejected outside configured tolerance.

This is an explicit educational profile, not a universal SENT decoder. A real sensor can use a different number of data nibbles, CRC convention, pause pulse, serial-message channel or timing configuration. Confirm the frame layout and CRC table against the sensor datasheet and applicable SENT specification before integration.

## Recommended STM32 capture path
1. Configure a timer channel for input capture.
2. Convert consecutive capture register values to unsigned delta counts, correctly handling timer rollover.
3. Use DMA circular/double-buffer capture where supported; keep interrupt handlers short.
4. Pass completed bounded windows to `sent_decoder_process()` from foreground or a dedicated task.
5. Keep UART formatting/output out of the capture ISR and rate-limit diagnostics.
6. Track DMA overruns and capture errors independently from decoder errors.

Timer/DMA channel mapping is MCU-specific; check the exact device reference manual and board pinout. Verify voltage levels before connecting a sensor output.

## Diagnostics
- `valid_frames`: frames that pass pulse checks and configured CRC.
- `sync_errors`: sync intervals outside configured bounds.
- `pulse_errors`: an interval cannot map to a nibble.
- `crc_errors`: decodable frames with a mismatched CRC.
- `timeouts`: timeouts reported by the capture/application layer.
- `signal_present`: cleared when the application explicitly reports timeout.

## Safety
This is an educational acquisition/diagnostics prototype, not a certified automotive component. Do not use it to control safety-critical vehicle functions.
