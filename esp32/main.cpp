#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <time.h>
#include "esp_sntp.h"


//DO NOT CHANGE INPUT PIN
const uint8_t dht_pin = 2;
const uint8_t LCD_sda = 4;
const uint8_t LCD_scl = 5;
const uint8_t LCD_ADDR = 0x27;
const uint8_t buzzPin = 7;
const uint8_t buttonPin = 10;

const char *ssid = "WiFi goes here";
const char *pass = "Passwords goes here";
const char *ntpServer = "pool.ntp.org"; 

const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 3600;

//declare display default state(s)
uint8_t buttonState = 0;
uint8_t prevButtonState = 0;
uint8_t menuState = 0; // 0 default

//delcare collected values
float tempF = 0;
float temp = 0;
float tempC = 0;
float humidity = 0;

uint8_t h = 0;
uint8_t m = 0;
uint8_t s = 0;
uint8_t mon = 0;
uint8_t day = 0;
uint8_t yrs = 0;


//define DHT sensor object (assume 6 count)
DHT dht(dht_pin, DHT11);

//define LCD obj
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);


//---------------------Define Task Functions-----------------------//

void connectToWifi(){
    Serial.print("Connecting to ");
    Serial.print(ssid);

    WiFi.begin(ssid, pass);

    //outputs wifi status to Serial Monitor & LCD Display.
    while (WiFi.status() != WL_CONNECTED){
      Serial.println("...attempting to connect..");
      lcd.print("...connecting..");
      delay(500);
    }

    Serial.println("Connected to WiFi.");
    Serial.print(WiFi.localIP()); 

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Connected!");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());
    delay(1000);
    lcd.clear();
    
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  /* Stores to Vars for Access*/
  h = timeinfo.tm_hour;
  m = timeinfo.tm_min;
  s = timeinfo.tm_sec;

  mon = timeinfo.tm_mday + 1;
  day = timeinfo.tm_mday;
  yrs = timeinfo.tm_year + 1900;


  delay(1000);
}


void buttonTask(void * params){
  /*
    Detects changes in button state to change LCD menu.
    Prompts change to LCD Menu
  */
 
  /* Declare display state
    0. Temperature (F°)
       Humidity (RH%)
    1. Time & Date (XX:XX MM/DD/YYYY)
       Real Time Distance (m)

  */ 

  for(;;){

    buttonState = digitalRead(buttonPin);

    //Detects change in button state & changes menu
    if(prevButtonState == 1 && buttonState == 0){
      

      if(menuState == 0){ /* State 0 -> 1*/

        menuState = 1;
        lcd.setCursor(0,0);
        lcd.print("Temp (F");
        lcd.print((char)223);
        lcd.print(")");
        lcd.print(": ");
        lcd.print(tempF);
        
        lcd.setCursor(0,1);
        lcd.print("Humidity ");
        lcd.print(humidity);
        lcd.print(" %");
      }
      else{ /* State 1 -> 0 */

        menuState = 0;
        
        lcd.setCursor(0,0);
        lcd.print("Time: ");
        lcd.print(h);
        lcd.print(":");
        lcd.print(m);
        lcd.print(":");
        lcd.print(s); 

        lcd.setCursor(0,1);
        lcd.print("Date: ");
        lcd.print(mon);
        lcd.print("/");
        lcd.print(day);
        lcd.print("/");
        lcd.print(yrs);

      }

    }

    //updates button state
    prevButtonState = buttonState;

    //Blocks for 5 ms
    vTaskDelay( 5 / portTICK_PERIOD_MS);

  }
}



void WifiTask(void * params){
  /*
    Connects to WiFi & communicates through 
    the HTML to allow for local remote control.
  
  */


}



void DHTread(void * params){

    /*
        Reads input data from DHT sensor & assigns to respective 
        temperature & humidity vars every 2 seconds.

        Prints collected information to serial.
    */
  
  for(;;){
    temp = dht.readTemperature();
    tempF = dht.convertCtoF(temp);
    
    //calculates offset caused by inaccuracies using DHT11 sensor
    tempF = tempF - 2; 
    tempC = dht.convertFtoC(tempF);

    humidity = dht.readHumidity();

    Serial.print("Humidity (RH)%: "); 
    Serial.println(humidity);
    Serial.print("Temp C: ");
    Serial.print(tempC);
    Serial.print(" / ");
    Serial.print("Temp F: ");
    Serial.println(tempF);

    Serial.println("-------------");

    //Blocks for 2000 ms
    vTaskDelay( 2000 / portTICK_PERIOD_MS);

  }
}




//-----------------------Setup---------------------------//


void setup() {
  // put your setup code here, to run once:


  Serial.begin(115200);
  delay(500); // Give serial a moment to stabilize

  Serial.println("Initializing..");
  dht.begin();

  Wire.begin(LCD_sda, LCD_scl);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  //instantiate  button
  pinMode(buttonPin, INPUT_PULLUP); //1 (not pressed), 0 (pressed)
  
  

  connectToWifi();

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

  printLocalTime();

  
  /*
    Order of Setup:
    0. Initialize Serial & Task Priority
    01. Initializes sensor reading & Configs LCD
    1. Connect to WiFi
    2. Configures Time & Date
    3. Connects to Website
    4. Setup is Complete (Audio Response)
  
  
  */

  //Audio Response
  pinMode(buzzPin, OUTPUT);
  tone(buzzPin,  5000);
  delay(250);
  tone(buzzPin,0);

  xTaskCreate(
    DHTread, // Function Name
    "DHTread", //task Name
    2048, // stack size
    NULL, // task parameters,
    1, // task priority
    NULL // task handle
  );

  xTaskCreate(
    buttonTask, // Function Name
    "buttonTask", //task Name
    2048, // stack size
    NULL, // task parameters,
    3, // task priority
    NULL // task handle
  );
}

void loop() {
  




  delay(200);

}
