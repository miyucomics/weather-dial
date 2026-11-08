from datetime import datetime
from forecast import URL, Forecast
import json
import sys
import time
import urllib.request
import serial
from serial.tools import list_ports

ESPRESSIF_VID = 0x303A
BAUD = 115200

forecast = None
fetched_at = 0.0
REFRESH_SECONDS = 30 * 60


def get_forecast():
    global forecast, fetched_at
    if forecast is None or time.time() - fetched_at > REFRESH_SECONDS:
        try:
            with urllib.request.urlopen(URL) as response:
                forecast = Forecast.from_dict(json.load(response))
                fetched_at = time.time()
            print("forecast refreshed")
        except Exception as e:
            print(f"fetch failed: {e}")
    return forecast


def find_dial():
    if len(sys.argv) > 1:
        return sys.argv[1]
    for p in list_ports.comports():
        if p.vid == ESPRESSIF_VID:
            return p.device
    return None


def count_mins(dt):
    return dt.hour * 60 + dt.minute


def build_reply(f, offset):
    target, reading = f.at_offset(offset)
    profile = f.rain_profile(target.date())
    now = datetime.now()

    fields = [
        "DATA",
        str(offset),
        f"{target:%m-%dT%H:%M}",
        f"{reading.temperature:.1f}",
        str(reading.precip_chance),
        f"{reading.precip_mm:.1f}",
        str(count_mins(reading.sunrise)),
        str(count_mins(reading.sunset)),
        str(count_mins(now) if target.date() == now.date() else -1),
        ";".join(str(p) for p in profile),
    ]
    return ",".join(fields)


def handle_request(line):
    parts = line.split(",")
    if len(parts) != 2 or parts[0] != "GET":
        return "ERR,bad request"

    try:
        offset = int(parts[1])
    except ValueError:
        return "ERR,bad offset"

    f = get_forecast()
    if f is None:
        return "ERR,no forecast"

    try:
        return build_reply(f, offset)
    except KeyError:
        return "ERR,out of range"


def serve(port):
    with serial.Serial(port, BAUD, timeout=1) as channel:
        print(f"listening on {port}")
        while True:
            raw = channel.readline()
            if not raw:
                continue
            request = raw.decode(errors="ignore").strip()
            if not request:
                continue
            response = handle_request(request)
            channel.write((response + "\n").encode())
            print(f"{request} -> {response}")


while True:
    dial = find_dial()
    if dial is None:
        print("no dial found, retrying...")
        time.sleep(1)
        continue

    try:
        serve(dial)
    except serial.SerialException as e:
        print(f"serial error: {e}")
        time.sleep(2)
