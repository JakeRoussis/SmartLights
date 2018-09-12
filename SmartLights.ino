//Included Libraries
#include <Wire.h>
#include "RTClib.h"
//

//Pin Variables
int light_red_pin = 13;
int light_green_pin = 12;
int light_blue_pin = 11;
int motion_pin = 6;
int microphone_pin_in = A0;
int microphone_pin_out = 7;
//

//Global Variables
long long int microphone_last_state = 0;
int threshold;
bool light_on = false;
bool trigger_by_clap = false;

int start_hour = 19;
int start_min = 0;
int end_hour = 7;
int end_min = 0;

RTC_PCF8523 rtc;
//

//Set the colour of the LED
void setColour(int red_value, int green_value, int blue_value)
{
    analogWrite(light_red_pin, red_value);
    analogWrite(light_green_pin, green_value);
    analogWrite(light_blue_pin, blue_value);
}

//Serial print without disturbing other functions
void mySPL(String p1, unsigned long p2)
{
    Serial.print(p1);
    Serial.println(p2);
}

//Calculate background noise level
void calculate_background_noise()
{
    int i;
    unsigned long sum = 0;
    Serial.print("### Calculating background noise ###");
    for (i = 0; i < 1000; i++) //1000 samples
    {
        sum += analogRead(microphone_pin_in);
        delay(1);
    }
    threshold = int((sum / 1000) * 1.1); //Average samples plus clap level
    mySPL("Threshold set at ", threshold);
}

//Initialize
void setup()
{
    Serial.begin(9600);
    pinMode(motion_pin, INPUT);
    pinMode(microphone_pin_in, INPUT);
    pinMode(microphone_pin_out, OUTPUT);
    pinMode(light_red_pin, OUTPUT);
    pinMode(light_green_pin, OUTPUT);
    pinMode(light_blue_pin, OUTPUT);
    calculate_background_noise();
}

void loop()
{
    DateTime now = rtc.now();

    //Check if the light is already on from a clap
    if (trigger_by_clap == false)
    {
        //Check if its within the motion sensor's active hours
        if ((now.hour() >= start_hour && now.minute() >= start_min) || (now.hour() <= end_hour && now.minute() <= end_min))
        {
            if (digitalRead(6) == HIGH)
            {
                Serial.println("Active");
                setColour(255, 255, 255);
                delay(100);
            }
            else
            {
                //Serial.println("Inactive");
                setColour(0, 0, 0);
            }
        }
    }

    //Set clap counter
    int clap_count = 0;
    int level;

    //Check for initial clap
    level = analogRead(microphone_pin_in);
    if (level > threshold)
    {
        clap_count = 1;

        //Check for second clap
        for (int i = 0; i < 3000; i++)
        {
            level = analogRead(microphone_pin_in);
            if (level > threshold)
            {
                clap_count += 1;
                delay(40);
            }
        }
    }

    //Check if a double clap was found and turn the light on/off
    if (clap_count >= 2)
    {
        if (light_on == false)
        {
            setColour(255, 255, 255);
            Serial.println("LIGHT ON");
            light_on = true;
            trigger_by_clap = true;
        }
        else
        {
            setColour(0, 0, 0);
            light_on = false;
            trigger_by_clap = false;
            Serial.println("LIGHT OFF");
        }
    }
}
