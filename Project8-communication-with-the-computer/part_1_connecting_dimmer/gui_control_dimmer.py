import threading
import queue
import time
import serial
import FreeSimpleGUI as sg

# Serial settings
PORT = 'COM4'  
BAUD_RATE = 9600

serial_queue = queue.Queue()

def serial_reader_thread(ser, msg_queue):
    while True:
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').strip()
                if line:
                    msg_queue.put(line)
        except Exception as e:
            msg_queue.put(f"ERROR: Serial read failed: {e}")
            break
        time.sleep(0.01)

def main():
    try:
        ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Wait for Arduino boot
    except Exception as e:
        sg.popup_error(f"Could not open serial port {PORT}.\nError: {e}")
        return

    # Added enable_events=True to the slider for real-time updates
    layout = [
        [sg.Text("Configure LED Settings:", font=("Helvetica", 11, "bold"))],
        [sg.Text("Duration (ms):"), sg.Input(key="-MS_INPUT-", size=(10, 1), default_text="5000")], # Default to 5s for easier dimmer testing
        [sg.Text("Brightness (0-255):")],
        [sg.Slider(range=(0, 255), default_value=255, orientation='h', size=(20, 15), 
                   key="-BRIGHTNESS_SLIDER-", enable_events=True)], 
        [sg.Button("Send to Arduino", key="-SEND_BTN-", button_color=("white", "blue"))],
        [sg.HSeparator(pad=(0, 15))],
        [sg.Text("Arduino Live Status Logs:", font=("Helvetica", 10, "bold"))],
        [sg.Multiline(size=(45, 10), key="-LOG-", autoscroll=True, disabled=True, background_color="black", text_color="green", font=("Courier", 10))],
        [sg.Button("Exit", size=(10, 1), button_color=("white", "red"))]
    ]

    window = sg.Window("Arduino PWM LED Control", layout, finalize=True)

    reader_thread = threading.Thread(target=serial_reader_thread, args=(ser, serial_queue), daemon=True)
    reader_thread.start()

    window["-LOG-"].print("[SYSTEM]: Connected to Arduino COM4. Ready.")

    # Helper function to send the data safely
    def send_settings(duration_str, brightness_val):
        if duration_str.isdigit() and int(duration_str) > 0:
            try:
                payload = f"{duration_str},{brightness_val}\n"
                ser.write(payload.encode('utf-8'))
            except Exception as e:
                window["-LOG-"].print(f"[ERROR]: Failed to send data: {e}")

    # Track last sent brightness to avoid overloading the serial buffer
    last_sent_brightness = 255

    while True:
        event, values = window.read(timeout=50)

        if event == sg.WINDOW_CLOSED or event == "Exit":
            break

        # Send when the explicit "Send" button is clicked
        if event == "-SEND_BTN-":
            duration_str = values["-MS_INPUT-"].strip()
            brightness_val = int(values["-BRIGHTNESS_SLIDER-"])
            send_settings(duration_str, brightness_val)
            window["-LOG-"].print(f"[SYSTEM]: Manual Send -> {duration_str} ms, Brightness {brightness_val}")

        # Real-time dimmer adjustment: Triggered automatically whenever the slider is dragged!
        if event == "-BRIGHTNESS_SLIDER-":
            current_brightness = int(values["-BRIGHTNESS_SLIDER-"])
            # Only send if the value actually changed to save serial bandwidth
            if current_brightness != last_sent_brightness:
                duration_str = values["-MS_INPUT-"].strip()
                send_settings(duration_str, current_brightness)
                last_sent_brightness = current_brightness

        try:
            while not serial_queue.empty():
                message = serial_queue.get_nowait()
                
                if message == "1":
                    window["-LOG-"].print("🟢 [Arduino State 1]: Button pressed! LED is ON.")
                elif message == "0":
                    window["-LOG-"].print("🔴 [Arduino State 0]: Timer finished. LED is OFF.")
                elif message.startswith("I received:"):
                    window["-LOG-"].print(f"📥 [Arduino Config]: {message}")
                elif "ERROR" in message:
                    window["-LOG-"].print(f"⚠️ {message}")
                else:
                    window["-LOG-"].print(f"💬 [Arduino Raw]: {message}")
                    
        except queue.Empty:
            pass

    try:
        ser.close()
    except:
        pass
    window.close()

if __name__ == "__main__":
    main()