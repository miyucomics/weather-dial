My submission for HackRPI 2026.

## Inspiration
It rains a lot in Troy and I often forget to check the weather before going out, so I have to go back up for an umbrella or worse, I try to brave through a light drizzle and then it turns into heavy rain spontaneously. I thought it would be convenient if I could have a device that would look up the weather for me and display all the important information at a glance.

## What it does
After it finds its matching daemon, it is able to request weather information (provided by Open Meteo) and render it on the circular display. It shows a 24-hour clock, with sunrise and sunset marked with the yellow suns. It also shows the current date and time being viewed, temperature, the precipitation percentage, and the amount of precipitation expected. By rotating the dial on the device, you can scroll hour-by-hour into the future or past to see old weather information and forecasts. Pressing the hidden button jumps back to the current day.

The hour markings also change color depending on the percentage of rain expected so you can see the entire day's rain schedule at a glance without having to scroll hour-by-hour. If it is noticeably blue, the chance of rain is likely high enough that you should bring an umbrella!

## How we built it
It uses M5Stack's M5Dial as the hardware component. I programmed the firmware in C. It communicates over serial with a daemon running on the computer. The daemon is programmed in Python (although it can really be written in any language) and makes web requests to [Open Meteo](https://open-meteo.com) in order to get historical weather information and future forecasts. The dial periodically asks the daemon for information (or when the user scrolls/presses the button) and the daemon responds with the weather information which it has cached. Every thirty minutes, the daemon fetches new up-to-date weather information but otherwise, the setup does not spam Open Meteo with requests.

## Challenges we ran into
Because the serial was being used for process communication, I could not print to the serial in order to debug the firmware. It also took around eight seconds each time to flash the firmware, which was very irritating since I could not iterate quickly.

## Accomplishments that we're proud of
I had another idea for the first twelve hours of the hackathon that did not end up panning out, and I managed to do this new idea with over half of my time gone already, and without having to resort to generative AI.

The dial was also able to predict me being rained on as I went to pick up boba from the DCC.

## What we learned
I did not know it was possible to communicate back and forth over serial. I thought I would have to set up a web server or somehow get the dial connected to RPI's WiFi but fortunately, it was really simple to receive and send messages on both sides.

## What's next for Weather Dial
The ESP-32 is capable of connecting to WiFi and making web requests on its own so the daemon on the host machine might not even be needed. The device is also low-power enough that a coin battery can likely power it for days on end. These are both potential improvements to the weather dial.
