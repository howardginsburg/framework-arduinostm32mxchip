# SPILoopback

Demonstrates SPI communication on the MXChip AZ3166.

## Loopback Test

For a true loopback test, connect the MOSI pin to the MISO pin. The received bytes should match the sent bytes.

Without the loopback wire, the example still demonstrates SPI initialization, configuration, and transfer calls.

## Running

1. Open the Serial Monitor at 115200 baud.
2. The test runs once at startup and prints sent vs. received byte values.
