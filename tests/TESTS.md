# Testing

## Test suite

This project runs on the `MSP432 microcontroller`.
There is no reliable emulator for this board. Because of this, we cannot test everything on the real hardware, so we created a test suite.

The tests in `test_main.c` check the program logic, not the hardware.

- Hardware functions are replaced with fake functions (mocks)
- Fan, pump, humidifier, and resistor states are stored in variables
- Buttons, levers, sensor values can be set manually
- Output is printed to the console

---

## How to run the test suite

Run this command in the root of the project to compile in `TEST_MODE`:
```bash
gcc -I include -DTEST_MODE tests/test_main.c source/states.c -o tests/test_suite
```
And to execute:
```bash
./tests/test_suite
```

---

## Hardware tests

Hardware tests require human observation. 
The desired output of the Hardware is printed on the console to be compared with the real outcome.
To run these tests currently there is no other way if not running the content of the if as if it was the main file.