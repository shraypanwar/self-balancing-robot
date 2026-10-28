# Electronics

## Power
- 11.1 V 3S1P Li-ion pack -> Waveshare serial bus servo driver board -> 2 x ST3020 servos
- 11.1 V 3S1P Li-ion pack -> LM2596S buck converter -> ESP32

## Signals
| From | To | Connection |
|------|----|-----------|
| ESP32 GPIO 21 (SDA), GPIO 22 (SCL) | BNO085 IMU | I2C (STEMMA QT / Qwiic) |
| ESP32 GPIO 17 (TX), GPIO 16 (RX) | Servo driver board | UART2, 1 Mbaud |
| Servo driver board | ST3020 servos (ID 1 and 2) | Serial bus |

The servos use a serial bus protocol, so they cannot be driven from PWM pins and need the UART driver board.

A wiring diagram for the final ESP32 design is planned.
