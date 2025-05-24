import asyncio
from bleak import BleakScanner, BleakClient
import os

SERVICE_UUID = "edddc3d6-5d4a-4677-87c5-f4f7d40b6111"
CHARACTERISTIC_UUID = "b4389ca3-0414-43a6-87d2-146c704e8353"

async def find_esp32():
    print("Hľadám ESP32 s UUID služby...")
    devices = await BleakScanner.discover()
    for device in devices:
        if SERVICE_UUID.lower() in [s.lower() for s in device.metadata.get("uuids", [])]:
            print(f" Nájdené ESP32: {device.name} ({device.address})")
            return device.address  # Vráti MAC adresu ESP32
        
    print(" ESP32 s danou službou nebolo nájdené.")
    return None

async def connect_ble():
    esp32_address = await find_esp32()
    if not esp32_address:
        return

    async with BleakClient(esp32_address) as client:
        print(" Pripojené k ESP32!")
        # Čítanie hodnoty z ESP32
        while True:
            value = await client.read_gatt_char(CHARACTERISTIC_UUID)
            value_hex = value.hex()
            print(f"Prijaté z ESP32: {value_hex}")
            if value_hex == "04000000":
                os.startfile("C:/Users/marti/Desktop/songa.MP3")
                break
                
asyncio.run(connect_ble())