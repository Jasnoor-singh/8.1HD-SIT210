#include <ArduinoBLE.h>
#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter;

BLEService ledService("180A"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLEWrite | BLENotify);

// List of allowed central MAC addresses
const String allowedMACs[] = {"d8:3a:dd:15:28:db","D8:3A:DD:15:28:DB"}; // Replace with actual MAC addresses


void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth Low Energy failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Nano 33 IoT");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characteristic:
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE Peripheral");

  Wire.begin();

  lightMeter.begin();

  Serial.println(F("BH1750 initialised"));
}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    String centralMAC = central.address();
    Serial.print("Trying to connect central: ");
    Serial.println(centralMAC);

    // Check if the connecting central is in the allowed list
    bool isAllowed = false;
    for (int i = 0; i < sizeof(allowedMACs) / sizeof(allowedMACs[0]); i++) {
      if (centralMAC.equalsIgnoreCase(allowedMACs[i])) {
        isAllowed = true;
        break;
      }
    }

    // If the central is allowed, proceed with connection
    if (isAllowed) {
      Serial.println("Allowed central is detected.");

      // while the central is still connected to peripheral:
      while (central.connected()) {
        Serial.print(">>");
          float lux = lightMeter.readLightLevel();
          Serial.print("Light: ");
          Serial.print(lux);
          Serial.println(" lx");
          delay(5000);
          switchCharacteristic.writeValue(lux);
      }

      // when the central disconnects, print it out:
      Serial.print(F("Disconnected from central: "));
      Serial.println(central.address());
    } else {
      Serial.println("Connection rejected. Central not allowed.");
      BLE.disconnect(); // Disconnect the unauthorized central
    }
  }
}