import threading
import queue
import time
import serial
import FreeSimpleGUI as sg

# Serial communication settings - Adjust PORT to match your system (e.g., 'COM3', '/dev/ttyUSB0')
PORT = 'COM4'  
BAUD_RATE = 9600

# Thread-safe queue to share incoming messages from background thread to GUI loop
serial_queue = queue.Queue()

def serial_reader_thread(ser, msg_queue):
    """
    Worker function running in a separate daemon thread.
    Continuously listens for incoming data from the serial port.
    """
    while True:
        try:
            if ser.in_waiting > 0:
                # Read line, decode to UTF-8, and strip newline/whitespace
                line = ser.readline().decode('utf-8').strip()
                if line:
                    msg_queue.put(line)
        except Exception as e:
            msg_queue.put(f"ERROR: Serial read failed: {e}")
            break
        time.sleep(0.01)  # Prevent high CPU consumption

def main():
    # Initialize serial connection
    try:
        ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Wait for Arduino to auto-reset after opening the port
    except Exception as e:
        sg.popup_error(f"Could not open serial port {PORT}.\nError: {e}\n\nPlease check connection and port name.")
        return

    # Define the graphical user interface layout
    layout = [
        [sg.Text("Configure LED ON Duration (ms):", font=("Helvetica", 11, "bold"))],
        [sg.Input(key="-MS_INPUT-", size=(15, 1), default_text="1000"), 
         sg.Button("Send to Arduino", key="-SEND_BTN-", button_color=("white", "blue"))],
        [sg.HSeparator(pad=(0, 15))],
        [sg.Text("Arduino Live Status Logs:", font=("Helvetica", 10, "bold"))],
        [sg.Multiline(size=(45, 10), key="-LOG-", autoscroll=True, disabled=True, background_color="black", text_color="green", font=("Courier", 10))],
        [sg.Button("Exit", size=(10, 1), button_color=("white", "red"))]
    ]

    # Create the application window
    window = sg.Window("Arduino LED Control - Project 8", layout, finalize=True)

    # Start background listener thread
    reader_thread = threading.Thread(target=serial_reader_thread, args=(ser, serial_queue), daemon=True)
    reader_thread.start()

    window["-LOG-"].print("[SYSTEM]: Connected to Arduino. Ready.")

    # Main event loop
    while True:
        # Check for user input events with a 50ms timeout to keep checking the queue
        event, values = window.read(timeout=50)

        # Handle app closing
        if event == sg.WINDOW_CLOSED or event == "Exit":
            break

        # Handle 'Send to Arduino' button click
        if event == "-SEND_BTN-":
            duration_str = values["-MS_INPUT-"].strip()
            
            # Validate input (must be positive integer)
            if duration_str.isdigit() and int(duration_str) > 0:
                try:
                    # Send input terminated by a newline character
                    ser.write(f"{duration_str}\n".encode('utf-8'))
                    window["-LOG-"].print(f"[SYSTEM]: Sent {duration_str} ms to Arduino...")
                except Exception as e:
                    window["-LOG-"].print(f"[ERROR]: Failed to send data: {e}")
            else:
                sg.popup_error("Please enter a valid positive integer.")

        # Non-blocking fetch of messages from the background thread queue
        try:
            while not serial_queue.empty():
                message = serial_queue.get_nowait()
                
                # Update GUI logs depending on states received from Arduino
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

    # Clean up and close connections
    try:
        ser.close()
    except:
        pass
    window.close()

if __name__ == "__main__":
    main()