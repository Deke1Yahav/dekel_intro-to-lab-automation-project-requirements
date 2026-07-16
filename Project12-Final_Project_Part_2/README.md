# Final Project Preparation: Closed-Loop Temperature Control

## Current Implementation

We are building the automation that updates the heater power based on the temperature error (setpoint − measured temperature).

The most up-to-date sketch is [`auto_PID_heatcontrol_with_temp_read_copy_20260716142933/auto_PID_heatcontrol_with_temp_read_copy_20260716142933.ino`](auto_PID_heatcontrol_with_temp_read_copy_20260716142933/auto_PID_heatcontrol_with_temp_read_copy_20260716142933.ino).

## Reference Materials

### Terms
- **Setpoint** = target temperature you choose.
- **Sensor feedback** = TC or PT100 reading.
- **Controller** = your PI implementation.
- **Actuator** = MOSFET switching power to the heating pad via PWM.
- **Overshoot** = how far above the setpoint the temperature rises before settling in percent relative to setpoint.
- **Settling time** = how long it takes to reach and stay within a small range of the setpoint (e.g., ±1 °C).


### SPI Communication

Both the TC amplifier (MAX31856) and the PT100 amplifier (MAX31865) talk to the Arduino over **SPI (Serial Peripheral Interface)** — a synchronous, master-slave bus using four signals:

- **SCK** (Serial Clock) — the master (Arduino) generates the clock that paces every bit.
- **MOSI** (Master Out, Slave In) — data from Arduino to the sensor module.
- **MISO** (Master In, Slave Out) — data from the sensor module back to Arduino. On these breakouts it's labeled **SDO**.
- **CS** (Chip Select) — pulled low by the master to address a specific slave device; multiple SPI devices can share SCK/MOSI/MISO as long as each has its own CS line.

On the Arduino Uno/Grove Beginner Kit, SCK/MOSI/MISO are fixed hardware pins **13/11/12**; only CS is freely assignable (we use **D4** for both sensors, one at a time).

#### Pins on the Sensor Transducers

Reading the labels on the breakout boards themselves (see photo above):

**MAX31856 (thermocouple amplifier):**
`VIN` (power in) · `GND` · `SDO` (MISO) · `SCK` · `CS` · `SDI` (MOSI) · `DRDY` (data-ready interrupt, optional) · plus the two screw-terminal inputs for the TC wires.

**MAX31865 (PT100/RTD amplifier):**
`VIN` · `3V3` · `SDO` (MISO) · `CS` · `GND` · `CLK` (SCK) · `SDI` (MOSI) · `RDY` (data-ready interrupt, optional) · a jumper selecting **2/3/4-wire** mode · plus screw terminals for the RTD wires.

Both boards expose the same four SPI signals (just named slightly differently: `SDO`/`SDI`/`CLK` vs `SDO`/`SDI`/`SCK`) plus power and an optional ready/fault interrupt pin — which is why they can be swapped in and out of the same Arduino wiring with minimal changes.


### Two Sensor Types: Thermocouple and PT100 (& MOSFET switch on the right)

![RTD and thermocouple amplifier modules](rtd_thermocouple_amplifier_and_relay_modules.jpg)

*Left to right: MAX31865 (PT100/RTD amplifier), MAX31856 (thermocouple amplifier), and the MOSFET power-switching module.*

#### Thermocouple (TC)

A thermocouple is two dissimilar metal wires joined at one end (the **Seebeck effect**): the junction produces a small voltage that depends on the temperature difference between that junction and the "cold junction" (where the wires meet the amplifier's terminals). The MAX31856 measures this voltage, compensates for the cold-junction temperature internally, and reports a temperature over SPI. Our probes are **K-type**.

![Thermocouple probe with wiring](thermocouple_probe_with_wiring.jpg)

#### PT100 RTD

A PT100 is a **Resistance Temperature Detector**: a platinum element whose electrical resistance increases predictably with temperature (100 Ω at 0 °C, hence "PT100"). The MAX31865 passes a small excitation current through it, measures the resulting voltage, computes resistance, and converts that to a temperature over SPI. Our probes use a **3-wire** configuration, which cancels out lead-wire resistance error better than a simple 2-wire hookup.

#### Comparing them

| | Thermocouple (K-type + MAX31856) | PT100 (MAX31865) |
|---|---|---|
| Sensing principle | Seebeck voltage at metal junction | Resistance change of platinum element |
| Typical response | Faster | Slower |
| Robustness to touch/disturbance | More stable | More prone to faults when disturbed |
| Wiring | 2 wires | 3-wire in our setup |

### Controlling Power: The MOSFET and PWM
**Important**: Maximum voltage is 12V!
Resistance is 19 Ohm.

The Arduino's digital pins can only supply a few milliamps — nowhere near enough to drive a heating pad directly. Instead, the Arduino's PWM output drives the **gate** of a **MOSFET** (a voltage-controlled switch), which switches the heater's much larger current on and off.

**PWM (Pulse Width Modulation)** rapidly switches the MOSFET fully on and fully off, varying the *fraction of time* it's on (the **duty cycle**, 0–100%) rather than the voltage itself. Because the heating pad has thermal mass, it effectively averages these fast pulses into a smooth, adjustable amount of heating power — a duty cycle of 50% delivers roughly half the average power of 100%.

```
loop:
    duty_percent = controller_output          # 0-100, from the PID/PI loop
    pwm_value    = map(duty_percent, 0, 100, 0, 255)   # analogWrite is 8-bit
    analogWrite(HEATER_PIN, pwm_value)
```

### The Heating Pad

![Silicone heater pad with aluminum plate](silicone_heater_pad_with_aluminum_plate.jpg)

The heater itself is a resistive **silicone heating pad** bonded to an aluminum plate. Passing current through its internal resistive element converts electrical power directly to heat ($P = I^2 R$); the aluminum plate spreads that heat evenly and provides thermal mass, which is what makes the PWM duty-cycle averaging (Section 9) work smoothly instead of causing visible temperature ripple.


### The Power Supply

Our bench power supply is adjustable, **up to 24 V at 3 A** — a very different source from the Arduino's own **5 V USB** power.

| | Bench power supply | Arduino USB port |
|---|---|---|
| Voltage | Variable, up to 24 V (we run the heater circuit at 12 V) | Fixed 5 V |
| Current capacity | Up to 3 A | Typically ~500 mA (USB spec) |
| Max power | Up to ~72 W (24 V × 3 A) | ~2.5 W |
| Powers | Heating pad, via the MOSFET switch | The Arduino board and logic-level sensors only |

The heating pad draws far more current than USB can supply, which is exactly why it needs its own supply switched by the MOSFET, while the Arduino and sensor amplifiers stay powered from USB.

### Reference Photos

![Grove Beginner Kit board](grove_beginner_kit_board.jpg)

*The Grove Beginner Kit board we build on top of for this project.*

![Connection diagram](connection%20diagram_project12.png)

*Full wiring diagram for the project (sensor amplifier, MOSFET module, heating pad, and Arduino).*


## Project Instructions
**Important**: In this experiment it is possible to damage the heating pad if you exceed 12 V. Do not exceed 12 V. It is also possible to *burn yourselves* or the table if you set the temperature too high. Clamp temperatures and use common sense.

1. Play with this simulation (maximum 15 minutes) to gain a deeper understanding of the meaning of the PID terms and how they affect the system response: [PID Simulator](https://aistudio.google.com/apps/89c8497b-6ef7-47e3-8d81-3ab77d0bbe13?showPreview=true&showAssistant=true&project=gen-lang-client-0073120285)
2. Power supply: Before wiring anything, measure the bench supply output with a multimeter and set it to 12 V. Then measure the voltage on the Arduino's 5V pin while it's powered over USB. Confirm the two are electrically separate — the heater circuit's ground and the Arduino's ground still need to be common, but the *power* comes from two different sources with very different current capability. **Important**: do not set to more than 12V.
2. Wire the full circuit step by step: 
    1.Temperature read: Arduino, MAX31856 (TC) or MAX31865 (PT100) sensor amplifier over SPI. Read live temperature values from either the thermocouple or the PT-100 using grillme skill and GitHub Copilot.
    2. MOSFET switching module, heating pad, and the bench power supply. Drive the heating pad's power with PWM through the MOSFET, controlled from software (0–100% duty cycle). Before testing, make sure you have set an **upper limit** to the temperature of no more than 35 degrees to prevent damage to yourselves and the equipment. using grillme skill and GitHub Copilot.
5. Implement a PI controller in arduino software, including anti-windup clamping on the integral term and a fixed sample interval. 
6. Manually tune $K_p$ and $K_i$ to reach a chosen setpoint with minimal overshoot and no steady-state error. You can use the serial monitor and a simple protocol in arduino cod, using grillme skill and GitHub Copilot.
7. Hold the heating pad at a target temperature for a sustained period despite heat loss to the surrounding air.
8. Compare and document controller performance. Create a table for at least 3 values of Kp and Ki and document overshoot, settling time, and steady-state accuracy.

### Check your understanding:

1. Why can't an open-loop heater controller guarantee a stable temperature if someone opens a window nearby?
2. Why does the derivative term tend to be noisy on a slow thermal system like ours, and why did we skip it?
3. If your temperature overshoots the setpoint and then slowly oscillates with decreasing amplitude before settling, which term would you adjust first, and in which direction?
4. What would happen to the overshoot if we removed the anti-windup clamp on a system that spends a long time at 100% duty cycle before reaching setpoint?
5. If you wanted to read both a TC and a PT100 sensor at the same time on one Arduino, which SPI pins would need to be shared and which would need to be separate?
6. Why does the MAX31856 need to know the "cold junction" temperature to report an accurate reading, while the MAX31865 doesn't have an equivalent requirement?
7. Rather than polling and re-reading a fault flag every cycle, both chips expose a `DRDY`/`RDY` pin. What kind of Arduino input would let you react to that pin without constantly polling it?
8. If the PWM frequency were slow enough that the heating pad could fully heat up and cool down within a single on/off pulse, would this averaging approximation still hold?
9. Why would powering the heating pad from the Arduino's own 5V USB supply be a bad idea, even at low duty cycle?

