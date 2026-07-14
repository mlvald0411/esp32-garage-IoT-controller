#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//DO NOT CHANGE INPUT PIN
const uint8_t dht_pin = 2;
const uint8_t LCD_sda = 4;
const uint8_t LCD_scl = 5;
const uint8_t LCD_ADDR = 0x27;
const uint8_t buzzPin = 7;


//define DHT sensor object (assume 6 count)
DHT dht(dht_pin, DHT11);

//define LCD obj
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

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
  
  //Audio Response
  pinMode(buzzPin, OUTPUT);
  tone(buzzPin, 3500);
  delay(250);
  tone(buzzPin,0);

  /* Declare display state
    1. Temperature (F°)
    2. Humidity (RH%)
    3. Time & Date (XX:XX MM/DD/YYYY)
    4. Real Time Distance

  */ 

}

void loop() {
  // put your main code here, to run repeatedly:
  float temp = dht.readTemperature();
  float tempF = dht.convertCtoF(temp);
  
  //calculates offset
  tempF = tempF - 2;
  float tempC = dht.convertFtoC(tempF);


  float humidity = dht.readHumidity();

  

  Serial.print("Humidity (RH)%: "); 
  Serial.println(humidity);

  
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" / ");
  Serial.print("Temp F: ");
  Serial.println(tempF);
  Serial.println("-------------");

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

  delay(2000);

}
