# weather-boondoggle
waste-of-time-wireless-weather-station


    # add board - using nodemcu for adafruit feather huzzah - think this is correct???
    platformio init -b nodemcu
    # Adafruit-SSD1306
    platformio lib install 135
    # build it....
    platformio run
    # burn it... if it doesn't work, try reseting the board right before or even during the command
    platformio run --target upload