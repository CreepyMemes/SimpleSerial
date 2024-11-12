# SimpleSerial Library

SimpleSerial is a library that enables asynchronous communication between two ESP32's, through the UART protocol, with handshakes and checksums for reliability. Either ESP32 can be either sending ore receiving at a time

---

Designed to work with ESP-IDF with Arduino as Component and uses a FreeRTOS task and queues to archieve asynchronous functionality.

### Note

On platformio.ini to set Simple Serial's logging level just add this build tag

```
build_flags = -DSIMPLE_SERIAL_LOG_LEVEL=4 ; To enable logging on Simple Serial Library
```
