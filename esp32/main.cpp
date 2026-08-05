#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
#include "esp_sntp.h"
#include <ArduinoJson.h>


#include "espWeb.h";


//DO NOT CHANGE INPUT PIN
const uint8_t dht_pin = 2;
const uint8_t LCD_sda = 4;
const uint8_t LCD_scl = 5;
const uint8_t LCD_ADDR = 0x27;
const uint8_t buzzPin = 7;
const uint8_t buttonPin = 10;
const uint8_t relaySignalPin = 18;

const char *ssid = "SpectrumSetup-C5";
const char *pass = "smallpoodle907";
const char *ntpServer = "pool.ntp.org"; 

const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 3600;

//declare display default state(s)
uint8_t buttonState = 0;
uint8_t prevButtonState = 0;
uint8_t menuState = 0; // 0 default
bool menuStateChanged = false;

volatile bool relayActivate = false;

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
uint16_t yrs = 0;


//Webserver object setup
WebServer server(80);


//define DHT sensor object (assume 6 count)
DHT dht(dht_pin, DHT11);

//define LCD obj
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);


//----------------------------------Define Functions--------------------------------------//

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
    lcd.print("Connected to WiFi!");
    lcd.print(WiFi.localIP());
    delay(750);
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

  mon = timeinfo.tm_mon + 1;
  day = timeinfo.tm_mday;
  yrs = timeinfo.tm_year + 1900;
  
}


void sendWebsite(){

  /*server.send(code, *content type, actual code (char)) 
  
    - 200: HTTP code (indicates "ok")
    - "text/HTML"
    - PAGE_MAIN : a very long const char indented to be inside dedicated .h file
  
  */
  
  server.send(200, "text/HTML", PAGE_MAIN);
}

void sendJSON(){
  /*
    Creates a temporary JSON document that stores temp. & humidity values and gets
    serialized into a serial string. This string then gets send to the HTML doc on
    the client device.
  */

  JsonDocument doc;

  doc["temp"] = tempF;
  doc["humidity"] = humidity;
  doc["relayActivate"] = relayActivate;
  //doc["garageState"] = 0 || 1;

  //Create raw text string
  String jsonSend;
  serializeJson(doc, jsonSend);

  server.send(200, "application/json", jsonSend);


}

void processButtonOpen(){ 
  relayActivate = true;
  server.send(200, "text/plain", "OK");
}


void processButtonClose(){ 
  relayActivate = true;
  server.send(200, "text/plain", "OK");
  
  } 
/* Note: Future improvements should be able to discern opened & closed garage states*/



//----------------------------------Define Task Functions---------------------------------//

void retrieveLocalTime(void * params){
  /*
    Repeatedly calls the print local time function to reassign time & date values
  
  */

  for(;;){
    printLocalTime();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

  }
}




void printToLCD(void * params){

  /*
    Continuously runs to always update the print

    ->retrieveLocalTime always reassigns the time & date values
    ->buttonTask always changes menuState based on the button press
  
  */

  
  for(;;){

    if(menuStateChanged == true){
      //Clears LCD to display new information
      lcd.clear();

      menuStateChanged = false;
    }

    if(menuState == 0){
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
    else{
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

    vTaskDelay( 100 / portTICK_PERIOD_MS );
  }

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
        menuStateChanged = true;
        menuState = 1;
        
      }
      else{ /* State 1 -> 0 */
        menuStateChanged = true;
        menuState = 0;

      }

    }

    //updates button state
    prevButtonState = buttonState;

    //Blocks for 50 ms
    vTaskDelay( 50 / portTICK_PERIOD_MS);

  }
}



void webServerTask(void * params){
  /*
    Continuously loops to retrieve any inputs 
    via the webpage
  
  */

  for(;;){


    server.handleClient(); 

    vTaskDelay( 50 / portTICK_PERIOD_MS);
  }

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

void relayTask(void * params){

  /*
    Detects signal from WiFiTask to simulate a garage door 
    activation. This is achieved by shorting the relay's output
    for 1 second.

    HIGH = open relay
    LOW = closed relay

  */


  for(;;){
    //keep in default state unless otherwise
    digitalWrite(relaySignalPin, HIGH);

    if(relayActivate){
      digitalWrite(relaySignalPin, LOW);
      vTaskDelay ( 1000 / portTICK_PERIOD_MS);
      digitalWrite(relaySignalPin, HIGH); //returns back to default state

      relayActivate = false; 
    }

    vTaskDelay( 50 / portTICK_PERIOD_MS);

  }
}

//--------------------------------------------Setup-----------------------------------------------//


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
  
  //instantiate  button & relay 
  pinMode(buttonPin, INPUT_PULLUP); //1 (not pressed), 0 (pressed)
  pinMode(relaySignalPin, OUTPUT); //1 (not activated), 0 (activated)
  digitalWrite(relaySignalPin, HIGH); //immidiately sends a HIGH to prevent activation
  

  connectToWifi();

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

  printLocalTime();


  //defines route handler callback function
  server.on("/", sendWebsite);
  server.on("/JSON", sendJSON);
  server.on("/Button_Open", processButtonOpen);
  server.on("/Button_Close", processButtonClose);
  //server.on("/Button_2", processButton2);

  server.begin();
  
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

  xTaskCreate(
    printToLCD, // Function Name
    "printToLCD", //task Name
    2048, // stack size
    NULL, // task parameters,
    3, // task priority
    NULL // task handle
  );

  xTaskCreate(
    retrieveLocalTime, // Function Name
    "retrieveLocalTime", //task Name
    2048, // stack size
    NULL, // task parameters,
    3, // task priority
    NULL // task handle
  );

  xTaskCreate(
    relayTask, // Function Name
    "relayTask", //task Name
    2048, // stack size
    NULL, // task parameters,
    3, // task priority
    NULL // task handle
  );


  xTaskCreate(
    webServerTask, // Function Name
    "webServerTask", //task Name
    4096, // stack size
    NULL, // task parameters,
    1, // task priority
    NULL // task handle
  );
}

void loop() {
  

  vTaskDelete(NULL);


  

}
