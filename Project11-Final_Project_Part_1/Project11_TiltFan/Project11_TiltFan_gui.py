"""Real-time GUI + CSV logger for Project 11's tilt-aware fan system.

Reads "angle,buzzer_state" lines from the Arduino over serial, draws a tilt
compass and a short angle history, shows the fan/buzzer state as an LED-style
chip, and logs every sample to a CSV alongside the elapsed time.
"""

import csv
import math
import queue
import threading
import time
from collections import deque

import FreeSimpleGUI as sg
import serial

SERIAL_PORT = "COM4"
SERIAL_BAUD = 9600
TILT_THRESHOLD_DEG = 30

DIAL_SIZE = 220
DIAL_RADIUS = 90
HISTORY_SECONDS = 30
HISTORY_LENGTH = 200  # samples kept for the sparkline, at the Arduino's ~50ms tick

CSV_PATH = f"project11_log_{time.strftime('%Y-%m-%d_%H-%M-%S')}.csv"


def serial_reader_thread(ser, incoming_queue, stop_event):
    """Background thread: blocks on serial reads so the GUI event loop never has to."""
    while not stop_event.is_set():
        try:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if line:
                incoming_queue.put(line)
        except serial.SerialException as exc:
            incoming_queue.put(f"ERROR:{exc}")
            break


def parse_line(line):
    """Turns 'angle,buzzer_state' into (angle: float, alarm: bool), or None if malformed."""
    try:
        angle_text, state_text = line.split(",")
        return float(angle_text), state_text.strip() == "1"
    except ValueError:
        return None


def draw_dial(graph, angle_deg, alarm):
    """Redraws the tilt compass: a safe/alarm background band plus a needle at angle_deg."""
    graph.erase()
    cx = cy = DIAL_SIZE // 2

    # Graph coordinates are set up so 0deg = east and angles increase counter-clockwise,
    # matching standard math convention; "up" (no tilt) therefore sits at 90deg.
    graph.draw_arc(
        (cx - DIAL_RADIUS, cy - DIAL_RADIUS), (cx + DIAL_RADIUS, cy + DIAL_RADIUS),
        360 - 2 * TILT_THRESHOLD_DEG, 90 + TILT_THRESHOLD_DEG,
        style="pieslice", fill_color="#c0392b", arc_color="#c0392b",
    )
    graph.draw_arc(
        (cx - DIAL_RADIUS, cy - DIAL_RADIUS), (cx + DIAL_RADIUS, cy + DIAL_RADIUS),
        2 * TILT_THRESHOLD_DEG, 90 - TILT_THRESHOLD_DEG,
        style="pieslice", fill_color="#2f8f5b", arc_color="#2f8f5b",
    )

    theta = math.radians(90 - angle_deg)
    needle_color = "#c0392b" if alarm else "#163a63"
    graph.draw_line(
        (cx, cy),
        (cx + DIAL_RADIUS * 0.85 * math.cos(theta), cy + DIAL_RADIUS * 0.85 * math.sin(theta)),
        color=needle_color, width=3,
    )
    graph.draw_circle((cx, cy), 5, fill_color=needle_color, line_color=needle_color)
    graph.draw_text(f"{angle_deg:+.1f} deg", (cx, 20), font=("Consolas", 12, "bold"))


def draw_history(graph, history):
    """Redraws the rolling angle sparkline from the last HISTORY_LENGTH samples."""
    graph.erase()
    if len(history) < 2:
        return

    w, h = 300, 70
    lo, hi = -90, 90
    points = [
        (i / (len(history) - 1) * w, (angle - lo) / (hi - lo) * h)
        for i, angle in enumerate(history)
    ]
    graph.draw_line((0, h / 2), (w, h / 2), color="#c8c8c8")  # zero-tilt reference line
    for (x1, y1), (x2, y2) in zip(points, points[1:]):
        graph.draw_line((x1, y1), (x2, y2), color="#b8720a", width=2)
    graph.draw_circle(points[-1], 3.5, fill_color="#b8720a", line_color="#b8720a")


def build_window():
    dial_graph = sg.Graph(
        canvas_size=(DIAL_SIZE, DIAL_SIZE),
        graph_bottom_left=(0, 0), graph_top_right=(DIAL_SIZE, DIAL_SIZE),
        key="-DIAL-",
    )
    history_graph = sg.Graph(
        canvas_size=(300, 70),
        graph_bottom_left=(0, 0), graph_top_right=(300, 70),
        key="-HISTORY-",
    )
    layout = [
        [sg.Text(f"{SERIAL_PORT} @ {SERIAL_BAUD}", font=("Consolas", 9)),
         sg.Push(), sg.Text("", key="-FANSTATE-", font=("Helvetica", 11, "bold"))],
        [dial_graph],
        [sg.Text("Angle, last 30s:", font=("Helvetica", 9))],
        [history_graph],
        [sg.Text("", key="-LOGSTATUS-", font=("Consolas", 9))],
        [sg.Button("Exit")],
    ]
    return sg.Window("Project 11 - Fan Monitor", layout, finalize=True)


def main():
    try:
        ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=1)
        time.sleep(2)  # let the Arduino finish its reset after the port opens
    except serial.SerialException as exc:
        sg.popup_error(f"Could not open {SERIAL_PORT}:\n{exc}")
        return

    incoming_queue = queue.Queue()
    stop_event = threading.Event()
    reader = threading.Thread(
        target=serial_reader_thread, args=(ser, incoming_queue, stop_event), daemon=True
    )
    reader.start()

    history = deque(maxlen=HISTORY_LENGTH)
    start_time = time.time()
    rows_written = 0

    csv_file = open(CSV_PATH, "w", newline="")
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(["elapsed_s", "angle_deg", "buzzer_state"])

    window = build_window()

    try:
        while True:
            event, _values = window.read(timeout=50)

            if event in (sg.WINDOW_CLOSED, "Exit"):
                break

            while not incoming_queue.empty():
                message = incoming_queue.get_nowait()

                if message.startswith("ERROR:"):
                    window["-FANSTATE-"].update("DISCONNECTED", text_color="red")
                    continue

                parsed = parse_line(message)
                if parsed is None:
                    continue  # ignore malformed/partial lines rather than crash

                angle_deg, alarm = parsed
                history.append(angle_deg)

                elapsed_s = time.time() - start_time
                csv_writer.writerow([f"{elapsed_s:.3f}", f"{angle_deg:.1f}", int(alarm)])
                rows_written += 1

                window["-FANSTATE-"].update(
                    "ALARM" if alarm else "FAN RUNNING",
                    text_color="red" if alarm else "#2f8f5b",
                )
                draw_dial(window["-DIAL-"], angle_deg, alarm)
                draw_history(window["-HISTORY-"], history)
                window["-LOGSTATUS-"].update(f"{CSV_PATH} - {rows_written} rows written")
    finally:
        stop_event.set()
        ser.close()
        csv_file.close()
        window.close()


if __name__ == "__main__":
    main()
