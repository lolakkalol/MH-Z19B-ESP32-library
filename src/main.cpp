#include <Arduino.h>
#include "MHZ19B.h"
#include "SoftwareSerial.h"


MHZ19B* mhz19b;

void setup() {
    Serial.begin(115200);
    while (!Serial){}

    mhz19b = new MHZ19B(13, 15);    
    mhz19b->calibrate_zero_point_auto(true);
}

void loop() {
    mhz19b->update_CO2_Tmp();

    Serial.print("CO2: ");
    Serial.println(mhz19b->get_CO2());
    Serial.print("Temperature: ");
    Serial.println(mhz19b->get_Temperature());

    sleep(2);
  
}