#!/usr/bin/env python3
"""
Serial port tester for CR_115_Turret.
Lists available serial ports, lets user select one, and sends a sample moveto command.
"""

import serial
import serial.tools.list_ports


def list_serial_ports():
    """Return a list of available serial ports."""
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]


def main():
    print("Serial Port Tester for CR_115_Turret")
    print("=" * 40)

    # List available serial ports
    ports = list_serial_ports()
    
    if not ports:
        print("No serial ports found!")
        return

    print("Available serial ports:")
    for i, port in enumerate(ports, 1):
        print(f"  {i}. {port}")

    # Prompt user to select a port
    while True:
        try:
            choice = input(f"\nSelect port (1-{len(ports)}): ")
            index = int(choice) - 1
            if 0 <= index < len(ports):
                selected_port = ports[index]
                break
            else:
                print(f"Please enter a number between 1 and {len(ports)}.")
        except ValueError:
            print("Please enter a valid number.")

    print(f"\nSelected port: {selected_port}")

    # Configure serial connection (baud rate matches Arduino's 9600)
    try:
        ser = serial.Serial(selected_port, baudrate=9600, timeout=1)
        print(f"Connected to {selected_port} at 9600 baud.")
        print("Type 'exit' to quit.\n")

        # Send a sample moveto command
        sample_yaw = 45.00
        sample_pitch = 30.00
        command = f"moveto {sample_yaw:.2f} {sample_pitch:.2f}\n"
        
        print(f"Sending command: {command.strip()}")
        ser.write(command.encode())
        
        # Wait for response
        try:
            response = ser.readline().decode().strip()
            if response:
                print(f"Response: {response}")
        except:
            pass

        # Interactive mode
        while True:
            user_input = input("Enter command (or 'exit'): ").strip()
            
            if user_input.lower() == 'exit':
                break
            
            if user_input:
                # Add newline to the command
                full_command = user_input + '\n'
                print(f"Sending: {user_input}")
                ser.write(full_command.encode())
                
                # Read response
                try:
                    response = ser.readline().decode().strip()
                    if response:
                        print(f"Response: {response}")
                except:
                    pass

        ser.close()
        print("Connection closed.")

    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")


if __name__ == "__main__":
    main()
