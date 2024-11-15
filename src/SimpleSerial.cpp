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
    _freertos_message_queue(NULL) {
}

// Destructor
SimpleSerial::~SimpleSerial() {
    end();
}

// Initiates the uart protocol and the the async main task
void SimpleSerial::begin(const unsigned long baud_rate, const SerialConfig mode) {
    _serial->begin(baud_rate, mode, _rx_pin, _tx_pin);
    _createMessageQueue();
    _createMainTask();
}

// Destroy main task and messages queue
void SimpleSerial::end() {
    _destroyMainTask();
    _destroyMessageQueue();
}

// Method used to send new messages through the Simple Serial protocol
void SimpleSerial::send(const std::vector<uint8_t> &message) {
    _pushQueue(message);
    xQueueSend(_freertos_message_queue, &_message_queue.back(), (TickType_t)10); // And sends it to the main task through the definedfreertos message queue
}

// -------------------------------------- PRIVATE METHODS -----------------------------------------------------
void SimpleSerial::_pushQueue(const std::vector<uint8_t> &message) {
    std::vector<uint8_t> *message_to_send = new std::vector<uint8_t>(message); // Allocates a new message vector dynamically
    _message_queue.push(message_to_send);                                      // Pushes the dynamic vector's pointer to the object's message queue handler
}

void SimpleSerial::_popQueue(std::vector<uint8_t> *message) {
    _message_queue.pop(); // Remove the message to send from the queue
    delete message;       // Free the dynamically allocated vector
}

// TODO
bool SimpleSerial::_isAvailableToSend(Message &message) {
    std::vector<uint8_t> *message_to_send = NULL;

    if (xQueueReceive(_freertos_message_queue, &message_to_send, (TickType_t)10) == pdTRUE) {


        SS_LOG_I("New message to send: {%s}", vectorToHexString(*message_to_send).c_str());
        // vectorToHexString(*message_to_send).c_str();

        try {
            // initiate the message handler with message as SENDER mode
            message.create(*message_to_send);
        }
        // Error handling if message initiation fails
        catch (const std::exception &e) {
            SS_LOG_E("%s", e.what());
            _popQueue(message_to_send);
        }

        _popQueue(message_to_send);

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
void SimpleSerial::_createMessageQueue() {
    if (_freertos_message_queue == NULL) {
        _freertos_message_queue = xQueueCreate(SIMPLE_SERIAL_QUEUE_SIZE, sizeof(std::vector<uint8_t> *));

        if (_freertos_message_queue == NULL) {
            SS_LOG_E("Queue for outgoing messages failed to create!");
        }
    }
}

// Destroy the outgoing messages queue
void SimpleSerial::_destroyMessageQueue() {
    if (_freertos_message_queue != NULL) {
        vQueueDelete(_freertos_message_queue);
        _freertos_message_queue = NULL;
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
            SS_LOG_I("Payload: {%s}", arrayToHexString(message.getPayload(), message.getSize()).c_str());
        }

        if (self->_isAvailableToReceive()) {
            // receive msg protocol here
        }

        delay(10); // Give other tasks a chance to run
    }
}
