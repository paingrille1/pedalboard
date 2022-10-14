ARDUINO_DIR   = /usr/share/arduino/
#ARDUINO_VERSION = 10819
ARDMK_DIR     = $(realpath ./lib/arduinomk)
AVR_TOOLS_DIR = /usr
MONITOR_PORT  = /dev/ttyACM0
BOARD_TAG   = nano
BOARD_SUB   = atmega328
AVRDUDE      = /usr/bin/avrdude
AVRDUDE_CONF = /etc/avrdude.conf
USER_LIB_PATH = $(realpath ./lib)

ARDUINO_LIBS = SPI Control-Surface Wire LiquidCrystal_PCF8574 uclock

include $(ARDMK_DIR)/Arduino.mk
