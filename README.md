# Arduino Sketches

Microcontroller sketches written for ESP32 boards — LED and buzzer output, an accelerometer tap sensor, and a four-wheel-drive robot with a motor driver.

**Personal project (not coursework)**

## Sketches

| Folder | Hardware | What it does |
|---|---|---|
| `esp32-led-blink` | ESP32, onboard LED | Blinks the built-in LED on GPIO 2 |
| `esp32-buzzer-player` | ESP32, passive buzzer, button | Plays short melodies (Mario, Tetris) from a frequency table, with button debouncing to switch tracks |
| `accelerometer-tap` | LSM6DS3 over I2C | Configures the accelerometer at a 416 Hz output data rate, enables tap detection on all three axes and sets the tap threshold |
| `accelerometer-tap-tuning` | LSM6DS3 over I2C | The same tap detection with the sensitivity threshold and interrupt routing tuned |
| `robot-motor-drive` | L298N driver, 2 motors | Drives two motors with a ramped acceleration profile, updating motion on a 20 ms tick |
| `robot-four-wheel-drive` | L298N driver, 4 motors | Extends the above to front-left / front-right / back-left / back-right channels |
| `robot-strafe-and-buzzer` | L298N, 4 motors, buzzer, button | Adds a strafe mode (hold a button to slide sideways) and a buzzer whose tone follows the drive state |

## Running

Open a folder in the Arduino IDE with the ESP32 board package installed. The accelerometer
sketches additionally need `SparkFunLSM6DS3` and `Wire`.

---

Submitted reports, worksheets and lecture material are archived outside this
repository rather than committed, so the repo stays code-only.
