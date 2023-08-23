#include <Arduino.h>
#include "MHZ19B.h"

MHZ19B::MHZ19B(size_t rx_pin, size_t tx_pin) {
    this->CO2_Serial = new SoftwareSerial(rx_pin, tx_pin, false);
    this->CO2_Serial->setTimeout(5);
    this->CO2_Serial->begin(9600);
    this->CO2_concentration = 0;
    this->Temperature = 0;
}

MHZ19B::~MHZ19B() {
    free(this->CO2_Serial);
}

/**
 * @brief Send the specified command to the MH-Z19B software serial port. 
 * Byte 3 and 4 are only used for detection range setting and span point callibration commands.
 * @param cmd The command to be sent to MH-Z19B.
 * @param data The data to be sent be added after the second byte. Max of 5 bytes, use 0x00 when not used.
 * @param data_len The length of the data provided
 * @returns A failure code specified in MHZ19B.h.
*/
MHZ19B_STATUS MHZ19B::send_command(Command cmd, uint8_t* data, uint8_t data_len) {
    if (data_len > 5)
        return MHZ19B_STATUS::GENERIC_FAILURE;

    // Allocate memmory for packet to be sent
    char* packet = (char*) calloc(9, 1);

    if (packet == NULL) {
        return MHZ19B_STATUS::ALLOCATION_FAULT;
    }

    // Set message header
    packet[0] = message_header[0];
    packet[1] = message_header[1];

    // Insert command
    packet[2] = (char) cmd;

    // Insert data bytes
    for (int i = 0; i < data_len; i++) {
        packet[3+i] = data[i];
    }

    // Calculate and set the checksum byte
    packet[8] = this->calculate_checksum(packet);

    while (!this->CO2_Serial->availableForWrite()) {
        sleep(0.1);
    }

    size_t bytes_writte = this->CO2_Serial->write(packet, 9);
    this->CO2_Serial->flush();

    if (bytes_writte < 9) {
        return MHZ19B_STATUS::WRITE_FAULT;
    }

    free(packet);
    return MHZ19B_STATUS::OK;
}

/**
 * @brief Receives a 9 byte long message from the MH-Z19B. This blocks forever.
 * @param message A message buffer where the message shall be read into, length must be 9 bytes.
 * @returns A failure code specified in MHZ19B.h.
*/
MHZ19B_STATUS MHZ19B::receive_message(char* message) {
    
    // Block until we have received 9 bytes
    while (this->CO2_Serial->available() < 9) {}
    this->CO2_Serial->readBytes(message, 9);


    if (message[8] != this->calculate_checksum(message)) {
        return MHZ19B_STATUS::CHECKSUM_ERROR;
    }

    return MHZ19B_STATUS::OK;
}

/**
 * @brief Get a CO2 concentration and temperature reading from the sensor.
 * @returns A failure code specified in MHZ19B.h.
*/
MHZ19B_STATUS MHZ19B::update_CO2_Tmp() {
    MHZ19B_STATUS status = MHZ19B_STATUS::OK;
    char* message = (char*) malloc(9);


    if (message == NULL) {
        return MHZ19B_STATUS::ALLOCATION_FAULT;
    }

    // Send command to get CO2 Concentration
    status = this->send_command(Command::READ_CO2_CONCENTRATION, 0, 0);
    if (status != MHZ19B_STATUS::OK) {
        return status;
    }

    // Receive message from MH-Z19B
    status = this->receive_message(message);
    if (status != MHZ19B_STATUS::OK) {
        return status;
    }

    Serial.print("Message: ");
    for(size_t i = 0; i < 9; i++) {
        Serial.print(String(message[i], HEX));
        Serial.print(" ");
    }
    Serial.println();


    // Extract CO2 concentration from message and store in obj.
    this->CO2_concentration = (uint16_t) message[2] << 8 | message[3];
    this->Temperature       = (uint8_t) message[4]-40;
    if (this->CO2_concentration > 5000) {
        return MHZ19B_STATUS::OVER_MEASUREING_RANGE;
    }

    return MHZ19B_STATUS::OK;
}

 uint16_t MHZ19B::get_CO2() {
    return this->CO2_concentration;
}

uint8_t  MHZ19B::get_Temperature() {
    return this->Temperature;
}

MHZ19B_STATUS MHZ19B::calibrate_zero_point_auto(bool toggle) {
    uint8_t data = toggle ? 0xA0 : 0x00;
    return this->send_command(Command::ONOFF_AUTO_CALIBRATION, &data, 1);
}

MHZ19B_STATUS MHZ19B::calibrate_zero_point() {
    return this->send_command(Command::ONOFF_AUTO_CALIBRATION, NULL, 0);
}

MHZ19B_STATUS MHZ19B::calibrate_span_point(uint8_t* data, uint8_t data_len) {
    return this->send_command(Command::SPAN_POINT_CALIBRATION, data, data_len);
}

MHZ19B_STATUS MHZ19B::set_detection_range(uint8_t* data, uint8_t data_len) {
    return this->send_command(Command::SPAN_POINT_CALIBRATION, data, data_len);
}

/**
 * @brief Calculates the checksum of the packet according to the MHZ19B datasheet.
 * This function does not modify the packet variable passed into the method.
 * @param packet An 8 BYTE character array
 * @returns      The checksum of packet.
*/
char MHZ19B::calculate_checksum(char* packet) {
    char i, checksum;

    for( i = 1; i < 8; i++) {
        checksum += packet[i];
    }

    checksum = 0xff - checksum;
    checksum += 1;
    return checksum;
}