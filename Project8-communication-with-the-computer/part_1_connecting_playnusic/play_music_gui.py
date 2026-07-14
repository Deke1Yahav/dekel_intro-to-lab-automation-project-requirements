import threading
import queue
import time
import serial
import FreeSimpleGUI as sg

# Serial settings
PORT = 'COM4'  
BAUD_RATE = 9600

# Mapping of musical notes to their frequency in Hz
NOTE_MAP = {
    'C4': 262, 'D4': 294, 'E4': 330, 'F4': 349, 'G4': 392, 'A4': 440, 'B4': 494,
    'C5': 523, 'D5': 587, 'E5': 659, 'F5': 698, 'G5': 784, 'A5': 880, 'B5': 988,
    'C6': 1047, 'D6': 1175, 'E6': 1319, 'F6': 1397, 'G6': 1568, 'SILENCE': 0
}

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

    # Updated layout containing the Melody Input Box
    layout = [
        [sg.Text("Configure LED / Buzzer Settings:", font=("Helvetica", 11, "bold"))],
        [sg.Text("Duration (ms):"), sg.Input(key="-MS_INPUT-", size=(10, 1), default_text="1000")],
        [sg.Text("Brightness / Volume (0-255):")],
        [sg.Slider(range=(0, 255), default_value=255, orientation='h', size=(20, 15), 
                   key="-BRIGHTNESS_SLIDER-", enable_events=True)], 
        [sg.Button("Send Config", key="-SEND_BTN-", button_color=("white", "blue"))],
        [sg.HSeparator(pad=(0, 10))],
        
        # New Melody Creation Section
        [sg.Text("Compose Custom Melody:", font=("Helvetica", 11, "bold"))],
        [sg.Text("Type notes (e.g., C5 D5 E5) with duration in ms (e.g., C5:250):")],
        [sg.Input(key="-MELODY_INPUT-", size=(40, 1), 
                  default_text="E5:250 E5:250 F5:250 G5:250 G5:250 F5:250 E5:250 D5:250 C5:250 C5:250 D5:250 E5:250 E5:350 D5:120 D5:500")],
        [sg.Button("Play Melody from PC", key="-PLAY_MELODY_BTN-", button_color=("white", "purple"))],
        
        [sg.HSeparator(pad=(0, 10))],
        [sg.Text("Arduino Live Status Logs:", font=("Helvetica", 10, "bold"))],
        [sg.Multiline(size=(45, 8), key="-LOG-", autoscroll=True, disabled=True, background_color="black", text_color="green", font=("Courier", 10))],
        [sg.Button("Exit", size=(10, 1), button_color=("white", "red"))]
    ]

    window = sg.Window("Arduino Symphony & Controller", layout, finalize=True)

    reader_thread = threading.Thread(target=serial_reader_thread, args=(ser, serial_queue), daemon=True)
    reader_thread.start()

    window["-LOG-"].print("[SYSTEM]: Connected to Arduino COM4. Ready.")

    def send_settings(duration_str, brightness_val):
        if duration_str.isdigit() and int(duration_str) > 0:
            try:
                payload = f"{duration_str},{brightness_val}\n"
                ser.write(payload.encode('utf-8'))
            except Exception as e:
                window["-LOG-"].print(f"[ERROR]: Failed to send data: {e}")

    last_sent_brightness = 255

    while True:
        event, values = window.read(timeout=50)

        if event == sg.WINDOW_CLOSED or event == "Exit":
            break

        if event == "-SEND_BTN-":
            duration_str = values["-MS_INPUT-"].strip()
            brightness_val = int(values["-BRIGHTNESS_SLIDER-"])
            send_settings(duration_str, brightness_val)
            window["-LOG-"].print(f"[SYSTEM]: Manual Send -> {duration_str} ms, Brightness {brightness_val}")

        # Real-time dimmer adjustment
        if event == "-BRIGHTNESS_SLIDER-":
            current_brightness = int(values["-BRIGHTNESS_SLIDER-"])
            if current_brightness != last_sent_brightness:
                duration_str = values["-MS_INPUT-"].strip()
                send_settings(duration_str, current_brightness)
                last_sent_brightness = current_brightness

        # Play custom melody typed in the input box
        if event == "-PLAY_MELODY_BTN-":
            raw_melody = values["-MELODY_INPUT-"].strip().split()
            melody_payload = []
            
            for item in raw_melody:
                note_name = "C5"
                duration = 250 # default 250ms
                
                if ":" in item:
                    parts = item.split(":")
                    note_name = parts[0].upper()
                    if parts[1].isdigit():
                        duration = int(parts[1])
                else:
                    note_name = item.upper()
                
                # Convert Note name to Frequency
                freq = NOTE_MAP.get(note_name, 0)
                if freq > 0 or note_name == 'SILENCE':
                    melody_payload.append(f"{freq},{duration}")
            
            if melody_payload:
                # Format: "M:freq,dur,freq,dur,freq,dur...\n"
                final_payload = "M:" + ",".join(melody_payload) + "\n"
                try:
                    ser.write(final_payload.encode('utf-8'))
                    window["-LOG-"].print("[SYSTEM]: Sent melody payload to Arduino...")
                except Exception as e:
                    window["-LOG-"].print(f"[ERROR]: Failed to send melody: {e}")
            else:
                sg.popup_error("No valid notes found! Use format like: C5 D5 E5 or C5:200")

        try:
            while not serial_queue.empty():
                message = serial_queue.get_nowait()
                
                if message == "1":
                    window["-LOG-"].print("🟢 [Arduino State 1]: Button pressed! LED/Buzzer is ON.")
                elif message == "0":
                    window["-LOG-"].print("🔴 [Arduino State 0]: Timer finished. LED/Buzzer is OFF.")
                elif message.startswith("I received:"):
                    window["-LOG-"].print(f"📥 [Arduino Config]: {message}")
                elif "Playing custom melody" in message:
                    window["-LOG-"].print("🎼 [Arduino Status]: 🎶 Playing your custom melody... 🎶")
                elif "Melody Finished" in message:
                    window["-LOG-"].print("🎼 [Arduino Status]: Custom melody playback complete!")
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