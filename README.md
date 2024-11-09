# SimpleSerial Library

Simple api class for UART comunication protocol between two ESP32's.

Designed to work with ESP-IDF with Arduino as Component and uses a FreeRTOS task and queues to archieve asynchronous functionality.

This protocol is an extension of the default UART, by adding two more pins that direct the sending / receiving only for 1 ESP32 at a time.
