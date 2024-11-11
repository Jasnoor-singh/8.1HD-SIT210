from bluepy import btle
import time
from gpiozero import PWMLED
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BOARD)

blue_led = PWMLED(17)

def map_lux_to_analog_value(lux, max_lux=1000):
    # Map lux (0 to max_lux) to (0 to 255)
    analog_value = int((lux / max_lux) * 255)
    # Ensure the value is between 0 and 255
    return max(0, min(255, analog_value))

def connect_and_read_ble(device_mac, characteristic_uuid):
    device = None
    try:
        print(f"Connecting to {device_mac}...")
        device = btle.Peripheral(device_mac, btle.ADDR_TYPE_PUBLIC)
        print(f"Reading characteristic {characteristic_uuid}...")
        
        while True:
            characteristic = device.getCharacteristics(uuid=characteristic_uuid)[0]
            value = characteristic.read()
            number = int.from_bytes(value, byteorder='big')  # Convert to integer
            print(f"Value: {number}")
            
            # Mapping the value of lux into analog value
            analog_value = map_lux_to_analog_value(number, 100)
            print(f"Analog Value: {analog_value}")
            blue_led.value = 1 - (analog_value / 255)
            time.sleep(1)  # Adjust delay as needed
            
    except Exception as e:
        print(f"Failed to connect or read from {device_mac}: {str(e)}")
    finally:
        if device:
            print("Disconnecting...")
            device.disconnect()
            print("Disconnected")

if __name__ == "__main__":
    device_mac_address = "30:C6:F7:03:5A:D2"
    characteristic_uuid = "2A57" 

    connect_and_read_ble(device_mac_address, characteristic_uuid)