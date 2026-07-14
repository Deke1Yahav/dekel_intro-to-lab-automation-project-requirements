"""Turn on an LED on button press via pymata4/Firmata, off after a delay via threading.Timer."""

import threading
import time

from pymata4 import pymata4

BUTTON_PIN = 2
LED_PIN = 4
LED_ON_MS = 1000  # How long the LED stays on after a press

board = pymata4.Pymata4()

# Holds the pending "turn off" timer so a new press can cancel/replace it
# instead of stacking up multiple timers.
led_off_timer = None


def turn_off_led():
    board.digital_write(LED_PIN, 0)
    print("LED off")


def button_callback(data):
    """pymata4 invokes this on its own thread whenever BUTTON_PIN changes state.
    data is [pin_mode, pin_number, value, timestamp]."""
    global led_off_timer

    _, pin_number, value, _ = data
    if value != 1:
        return  # Only react to the press (rising edge), not the release

    print(f"Button pressed on pin {pin_number}")
    board.digital_write(LED_PIN, 1)

    if led_off_timer is not None:
        led_off_timer.cancel()
    led_off_timer = threading.Timer(LED_ON_MS / 1000, turn_off_led)
    led_off_timer.start()


def main():
    board.set_pin_mode_digital_output(LED_PIN)
    board.set_pin_mode_digital_input(BUTTON_PIN, callback=button_callback)

    print("Monitoring button presses. Press Ctrl+C to exit.")
    try:
        # button_callback runs on pymata4's own thread, so the main thread
        # just has to stay alive; it's free to do other work here too.
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass
    finally:
        if led_off_timer is not None:
            led_off_timer.cancel()
        board.shutdown()


if __name__ == "__main__":
    main()
