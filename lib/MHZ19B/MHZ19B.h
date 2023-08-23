#ifndef MHZ19B_h
#define MHZ19B_h

#include "SoftwareSerial.h"

const char message_header[2] = {0xFF, 0x01};

enum class Command {
    READ_CO2_CONCENTRATION  = 0x86,
    ZERO_POINT_CALIBRATION  = 0x86,
    SPAN_POINT_CALIBRATION  = 0x86,
    ONOFF_AUTO_CALIBRATION  = 0x86,
    DETECTION_RANGE_SETTING = 0x86,
};

enum class MHZ19B_STATUS {
    GENERIC_FAILURE,
    OK,
    ALLOCATION_FAULT,
    WRITE_FAULT,
    CHECKSUM_ERROR,
    OVER_MEASUREING_RANGE,
};

class MHZ19B
{
private:
    char calculate_checksum(char* packet);
    MHZ19B_STATUS send_command(Command cmd, uint8_t* data, uint8_t data_len);
    MHZ19B_STATUS receive_message(char* message);
    SoftwareSerial* CO2_Serial;

    // Data
    uint16_t CO2_concentration;
    uint8_t  Temperature;
    

public:
    MHZ19B(size_t rx_pin, size_t tx_pin);
    ~MHZ19B();

    // Getters
    uint16_t get_CO2();
    uint8_t  get_Temperature();

    // Uppdaters
    MHZ19B_STATUS update_CO2_Tmp();
    MHZ19B_STATUS calibrate_zero_point_auto(bool toggle);
    MHZ19B_STATUS calibrate_zero_point();
    MHZ19B_STATUS calibrate_span_point(uint8_t* data, uint8_t data_len);
    MHZ19B_STATUS set_detection_range(uint8_t* data, uint8_t data_len);
};


#endif