int A1 = 34;
#define SoundSensorPin A1  //this pin read the analog voltage from the sound level meter
#define VREF  3.7  //voltage on AREF pin,default:operating voltage

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 2 /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

//Network info
#include "WiFi.h"
char* ssid = "Tenda_359A90";
const char* password =  "26069951207999";

//HTTP  
#include <HTTPClient.h>
#include <math.h>

float av;
float noise;
int n=900;



      /* Method to print the reason by which ESP32 has been awaken from sleep */
      void print_wakeup_reason()
      {
          esp_sleep_wakeup_cause_t wakeup_reason;

          wakeup_reason = esp_sleep_get_wakeup_cause();

            switch(wakeup_reason)
            {
                case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); 
                break;
                case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL");
                break;
                case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); 
                break;
                case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad");
                break;
                case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); 
                break;
                default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); 
                break;
            }
      }


      void setup()
      {
  
          Serial.begin(115200);
          delay(1000); //Take some time to open up the Serial Monitor

          pinMode(SoundSensorPin,INPUT);
          connectWiFi();

          float DB;
          // In each loop, make sure there is an Internet connection.
           
                  if (WiFi.status() != WL_CONNECTED)
                  { 
                        connectWiFi();
                  }

          int j=1;
          float sum=0;
          float max1 = -9999999.99;
int s=0;
                  while(j<=n)
                  {
                        DB=measure_sound();
                        Serial.print(DB,1); //noise value in dBA (*50)
                        Serial.println(" dBA");
                        float D = DB;
                        
                                if ( D > max1)
                                {
                                      max1 = D;
                                }
           
                        sum= sum + pow(10,DB/10);         // it is better if the noise is not in dB
                        j++;
                        delay(1000);                      //new value every second
                  }

           Serial.println("The maximum is: ");
           Serial.print(max1,1);
           Serial.println(" dBA");
       s=sum/j; 
           av = 10 * log10(s);                          //only the average value is multiplied by 50 (value in dBA)
           httpRequest(av, max1);
           
           Serial.print("The sum is: ");
           Serial.println(sum,1);
           Serial.println("The average is:");
           Serial.print(av,1);
           Serial.println(" dBA");
           Serial.println("New Values!!!");



    
           ++bootCount;                                   //Increment boot number and print it every reboot
           Serial.println("Boot number: " + String(bootCount));

           print_wakeup_reason();                         //Print the wakeup reason for ESP32

          /* First we configure the wake up source. We set our ESP32 to wake up every 5 seconds */
          esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
          Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

          Serial.println("Going to sleep now");
          delay(1000);
          Serial.flush();
          esp_deep_sleep_start();
          Serial.println("This will never be printed");    
      }



      void loop ()
      {
      //This is not going to be called.
      }
    
    
      void connectWiFi()
      {

          while (WiFi.status() != WL_CONNECTED )
          {
          WiFi.begin(ssid, password);
          delay(3000);
          }

           // Display a notification that the connection is successful. 
           Serial.println("Connected");
      }

      void httpRequest(float field1Data, float field1Data2) 
      {

          if ((WiFi.status() == WL_CONNECTED))
          { 
            //Check the current connection status
            HTTPClient http;

            String api = "https://api.thingspeak.com/update?api_key=UQKIGVLMHH15JVH1&field1=" +String(field1Data, 2)+"&field2="+String(field1Data2, 2);             //Specify the URL
            http.begin(api);
   
            int httpCode = http.GET();                                                    //Make the request
 
                if (httpCode > 0) 
                {
                  
                  //Check for the returning code
 
                  String payload = http.getString();
                  Serial.println(httpCode);
                  Serial.println(payload);
                }
 
                else 
                {
                Serial.println("Error on HTTP request");
                }
 
                http.end(); //Free the resources
          }
 
          delay(1000);
      }


      float measure_sound () 
      {
        
           float voltageValue, dbValue;
           voltageValue = analogRead(SoundSensorPin) / 4096.0 * VREF;
           dbValue = voltageValue*50;
           return dbValue;
      }
