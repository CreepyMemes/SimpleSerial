#include "SimpleSerial.h"

// -------------------------------------- PUBLIC METHODS -----------------------------------------------------

// SimpleSerial Constructor
SimpleSerial::SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, uint8_t max_retries, const UBaseType_t task_priority) :
    _serial(serial),
    _rx_pin(rx_pin),
    _tx_pin(tx_pin),
    _max_retries(max_retries),
    _task_priority(task_priority),
    _task_handle(NULL),
    _message_queue(),
    _message_available_event(NULL) {
}

// Destructor
SimpleSerial::~SimpleSerial() {
    end();
}

// Initiates the uart protocol and the the async main task
void SimpleSerial::begin(const unsigned long baud_rate, const SerialConfig mode) {
    _serial->begin(baud_rate, mode, _rx_pin, _tx_pin);
    _createMessageAvailableEvent();
    _createMainTask();
}

// Destroy main task and messages queue
void SimpleSerial::end() {
    _destroyMainTask();
    _destroyMessageAvailableEvent();
}

// Method used to send new messages through the Simple Serial protocol
bool SimpleSerial::send(const std::vector<uint8_t> &message) {
    SS_LOG_I("New message to send: {%s}", vectorToHexString(message).c_str());

    if (_pushQueue(message) == true) {
        if (xSemaphoreGive(_message_available_event) == pdTRUE) {
            SS_LOG_D("Semaphore event successfully released!");
            return true;
        }
        SS_LOG_W("Semaphore event failed to release!");
    }

    return false;
}

// -------------------------------------- PRIVATE METHODS -----------------------------------------------------
bool SimpleSerial::_pushQueue(const std::vector<uint8_t> &message) {
    if (_message_queue.size() < SIMPLE_SERIAL_QUEUE_SIZE) {
        std::vector<uint8_t> *message_to_send = new std::vector<uint8_t>(message); // Allocates a new message vector dynamically
        _message_queue.push(message_to_send);                                      // Pushes the dynamic vector's pointer to the object's message queue handler

        SS_LOG_D("Queue successfully pushed!");
        return true;
    }

    SS_LOG_W("Queue Full, Failed to push message!");
    return false;
}

void SimpleSerial::_popQueue() {
    delete _message_queue.front(); // Free the dynamically allocated vector
    _message_queue.pop();          // Remove the message to send from the queue
}

// TODO: no need to transfer vectors in freertos queue at this point...

// Check if a new message to send is available
bool SimpleSerial::_isAvailableToSend(Message &message) {
    if (xSemaphoreTake(_message_available_event, (TickType_t)10) == pdTRUE) {
        SS_LOG_D("Semaphore event was taken successfully!");

        // Try to initiate the message handler as SENDER mode with new message to send
        try {
            message.create(*_message_queue.front());
        }
        // Error handling if message initiation fails
        catch (const std::exception &e) {
            SS_LOG_E("%s", e.what());
            _popQueue();
        }

        _popQueue();
        return true;
    }

    return false;
}

// TODO
bool SimpleSerial::_isAvailableToReceive() {
    if (_serial->available() > 0) {

        return true;
    }

    return false;
}

// Create the queue that will hold the outoging messages
void SimpleSerial::_createMessageAvailableEvent() {
    if (_message_available_event == NULL) {
        _message_available_event = xSemaphoreCreateBinary();

        if (_message_available_event == NULL) {
            SS_LOG_E("Freertos semaphore for signaling outgoing message events failed to create!");
        }
    }
}

// Destroy the outgoing messages queue
void SimpleSerial::_destroyMessageAvailableEvent() {
    if (_message_available_event != NULL) {
        vSemaphoreDelete(_message_available_event);
        _message_available_event = NULL;
    }
}

// Create the main task for asynchronous operation
void SimpleSerial::_createMainTask() {
    if (_task_handle == NULL) {
        xTaskCreatePinnedToCore(_mainTask, "simple_serial", SIMPLE_SERIAL_STACK_SIZE, this, _task_priority, &_task_handle, SIMPLE_SERIAL_CORE);

        if (_task_handle == NULL) {
            SS_LOG_E("Main task failed to create!");
        }
    }
}

// Destroy the main task
void SimpleSerial::_destroyMainTask() {
    if (_task_handle != NULL) {
        vTaskDelete(_task_handle);
        _task_handle = NULL;
    }
}

// TODO
void SimpleSerial::_mainTask(void *arg) {

    // Cast arg to SimpleSerial pointer to access this object's instance
    SimpleSerial *self = (SimpleSerial *)arg;

    // Create a Message object to manage incoming/outcoming messages
    Message message;

    // The message payload vector handler
    std::vector<uint8_t> payload;

    while (true) {
        if (self->_isAvailableToSend(message)) {
            SS_LOG_I("Payload: {%s}\n", arrayToHexString(message.getPayload(), message.getSize()).c_str());
        }

        if (self->_isAvailableToReceive()) {
            // receive msg protocol here
        }

        delay(1); // Give other tasks a chance to run
    }
}
