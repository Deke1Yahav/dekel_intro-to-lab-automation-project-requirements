"""GUI to configure and monitor the Arduino LED timer over serial (Project 8, mini-project 1)."""

import queue
import threading
import time

import FreeSimpleGUI as sg
import serial

PORT = "COM4"  # Adjust to match the Arduino's serial port
BAUD_RATE = 9600

# State codes the Arduino sends after each button/timer event.
STATE_LABELS = {
    "0": ("LED OFF", "red"),
    "1": ("BUTTON PRESSED / LED ON", "green"),
    "2": ("BUTTON RELEASED", "orange"),
}


def serial_reader_thread(ser, incoming_queue, stop_event):
    """Background thread: blocks on serial reads so the GUI loop never has to."""
    while not stop_event.is_set():
        try:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if line:
                incoming_queue.put(line)
        except serial.SerialException as exc:
            incoming_queue.put(f"ERROR: lost connection to Arduino ({exc})")
            break


def build_window():
    layout = [
        [sg.Text("LED ON duration (ms):"),
         sg.Input(default_text="1000", size=(10, 1), key="-DURATION-"),
         sg.Button("Send", key="-SEND-")],
        [sg.Text("Status:"), sg.Text("—", key="-STATUS-", size=(28, 1), font=("Helvetica", 10, "bold"))],
        [sg.Multiline(size=(50, 12), key="-LOG-", autoscroll=True, disabled=True)],
        [sg.Button("Exit")],
    ]
    return sg.Window("Arduino LED Timer - Project 8", layout, finalize=True)


def main():
    try:
        ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Let the Arduino finish its auto-reset after the port opens
    except serial.SerialException as exc:
        sg.popup_error(f"Could not open {PORT}:\n{exc}")
        return

    incoming_queue = queue.Queue()
    stop_event = threading.Event()
    reader = threading.Thread(
        target=serial_reader_thread, args=(ser, incoming_queue, stop_event), daemon=True
    )
    reader.start()

    window = build_window()
    window["-LOG-"].print("Connected. Ready to send a duration.")

    while True:
        event, values = window.read(timeout=50)

        if event in (sg.WINDOW_CLOSED, "Exit"):
            break

        if event == "-SEND-":
            duration_text = values["-DURATION-"].strip()
            if duration_text.isdigit() and int(duration_text) > 0:
                try:
                    ser.write(f"{duration_text}\n".encode("utf-8"))
                    window["-LOG-"].print(f"Sent duration: {duration_text} ms")
                except serial.SerialException as exc:
                    window["-LOG-"].print(f"ERROR: send failed ({exc})")
            else:
                sg.popup_error("Enter a positive whole number of milliseconds.")

        # Drain everything the background thread has queued up since the last read.
        while not incoming_queue.empty():
            message = incoming_queue.get_nowait()

            if message in STATE_LABELS:
                label, color = STATE_LABELS[message]
                window["-STATUS-"].update(label, text_color=color)
                window["-LOG-"].print(f"[Arduino] state {message}: {label}")
            else:
                window["-LOG-"].print(f"[Arduino] {message}")

    stop_event.set()
    ser.close()
    window.close()


if __name__ == "__main__":
    main()
