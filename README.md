# SCC.300 Firmware

Firmware for the Arduino Opta, designed to simulate a traffic management system.

## Building

First, update your configuration in `src/config.h`. Of particular importance is the junction ID (`JUNCTION`) and spoofing/repudiation protections (`NON_REPUDIATION`: 0 is off, 1 is on).

This firmware can either be used as a Physical Twin, or as a Semi-Digital Twin. This is controlled by `SIMULATION`: 0 is the former and 1 is the latter. The Physical Twin operates standalone; for the Semi-Digital Twin, you should ensure the [world state server](https://github.com/j-leeming-uni/scc.300-world_state) is running first.

To compile and upload the code to your Opta board, you can use the `upload` target in the Makefile. You can run this target from the terminal in CLion by running the following command:

```shell
make upload
```

This will compile the code and upload it to the Opta board connected to your computer.

Run the `clean` target to remove the compiled files:

```shell
make clean
```

You can set the port that the Opta board is connected to by setting the `PORT` variable in the Makefile. By default, this is set to `/dev/ttyACM0`.

If the [Arduino CLI](https://arduino.github.io/arduino-cli/1.0/) is not on your path, you can set the `ARDUINO_CLI` variable in the Makefile to the path of the Arduino CLI executable.
