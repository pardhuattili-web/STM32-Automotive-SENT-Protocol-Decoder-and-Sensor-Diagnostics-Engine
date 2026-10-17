# Test Plan

## Host tests

| Scenario | Expected outcome |
|---|---|
| Synthetic valid short frame | Decoded status/data, CRC accepted, valid counter increment |
| Corrupted CRC nibble | CRC error counter increments |
| Sync below configured range | Sync error result |
| Impossible data interval | Pulse error result |
| Null capture pointer | Invalid-argument result |
| Application-reported timeout | Timeout counter increments, signal-present clears |
| Zero nominal tick configuration | Initialization rejected |

## Before physical integration
- [ ] Confirm sensor output voltage and safe input conditioning.
- [ ] Verify exact STM32 timer input-capture and DMA mapping.
- [ ] Derive timing and CRC profile from the selected sensor documentation.
- [ ] Capture a known-good waveform with a logic analyzer.
- [ ] Compare decoded fields against an independent reference.
- [ ] Test timer rollover, missing edges, buffer overrun and timeout.
- [ ] Measure processing time under expected interrupt/RTOS load.
- [ ] Commit real captures with expected decoded outputs as regression fixtures.

Only report measured hardware results with the board/MCU, sensor part number, setup and raw trace. Host simulation is not physical validation.
