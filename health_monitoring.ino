#include<OneWire.h>
#include<DallasTemperature.h>
#include<TinyGPS++.h>
#include<LiquidCrystal.h>
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);
#include<SoftwareSerial.h>
SoftwareSerial espSerial =  SoftwareSerial(2, 3);
String apiKey = "BYI9P411G3E9ECHI";     // replace with your channel's thingspeak WRITE API key
String ssid = "ROBOT1";  // Wifi network SSID
String password = "12345678"; // Wifi network password
boolean DEBUG = true;
int f3, f4, f5, f6, f7, f8, f9, f10;
int t5, t6;

int m=0;
int j1 = 0;
int i = 0, j = 0, temp = 0;
int l1 = 0;
int W = 0;
int l2 = 0;
int cm;
charstr[25];
int t;
constint sw = 13;
int sws = 0;
TinyGPSPlus gps; // GPS object

constint ldrPin = A2 ;
constint gas = A1;
int ldrStatus = 0;
int value1 = 0;
int buzzer = 12;
constint relay = 10;

#defineONE_WIRE_BUS11
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

int gasvalue = 0;
float tempC;

// BPM variables
boolean countStatus;
int beat, bpm;
unsignedlong millisBefore;

unsignedlong lastBeatMillis = 0;     // time of last counted beat
constunsignedlong beatRefractory = 250; // ignore new peaks for 250 ms after a beat (tune this)

// Non-blocking temperature globals
unsignedlong lastTempRequest = 0;
constunsignedlong tempConversionTime = 750; // typical conversion time in ms
bool tempWaiting = false;
constunsignedlong tempRequestInterval = 2000; // request conversion every 2s (tune if needed)

// Upload interval (increased to reduce interference)
constunsignedlong uploadInterval = 30000; // 30 seconds

voidsetup()
{
  lcd.begin(20, 4);
  Serial.begin(9600); // This opens up communications to the Serial monitor in the Arduino IDE
  lcd.setCursor(0, 0);
  lcd.print("      WEL-COME       ");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GSM initialisation");
  lcd.setCursor(0, 1);
  lcd.print("wait for 2 second");
  Serial.println("AT+CNMI=2,2,0,0,0");
  delay(500);
  Serial.println("AT+CMGF=1");
  delay(1000);
  lcd.clear();
  pinMode(sw, INPUT);
  t5 = millis();

  sensors.begin();
  // Optionally set wait-for-conversion false if supported by your library version:
  // sensors.setWaitForConversion(false);

  pinMode(gas, INPUT);
  pinMode(ldrPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(buzzer, LOW);
  digitalWrite(relay, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONNECTING WI-FI...");

  espSerial.begin(9600);  // enable software serial

  espSerial.println("AT+CWMODE=1");   // set esp8266 as client
  showResponse(3000);

  espSerial.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\""); // set your home router SSID and password
  showResponse(8000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WI-FI CONNECTED");
  delay(1500);

  if (DEBUG)  Serial.println("Setup completed");
  lcd.clear();
  t5 = millis();

  // initialize BPM-related state
  millisBefore = millis();   // start the 15s BPM window from now
  countStatus = false;
  beat = 0;
  bpm = 0;
  lastBeatMillis = millis();

  // Initialize temp conversion timing so first reading occurs shortly
  lastTempRequest = millis() - tempRequestInterval; // request immediately on first loop

  lcd.setCursor(0, 1);
  lcd.print("BPM:");
  // create a simple custom "heart" character (index 0)
  byte heart[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000
  };
  lcd.createChar(0, heart);
}

//========================================================== showResponce
voidshowResponse(int waitTime)
{
  long t = millis();
  char c;
  while (t + waitTime >millis())
  {
    if (espSerial.available())
    {
      c = espSerial.read();
      if (DEBUG) Serial.print(c);
    }
  }
}

//========================================================================
boolean thingSpeakWrite(float value3, float value4, float value5, float value6, float value7, float value8)
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";                  // TCP connection
  cmd += "184.106.153.149";                               // api.thingspeak.com
  cmd += "\",80";
  espSerial.println(cmd);
  if (DEBUG) //Serial.println(cmd);
    if (espSerial.find("Error")) {
      if (DEBUG) //Serial.println("AT+CIPSTART error");
        returnfalse;
    }
  String getStr = "GET /update?api_key=";   // prepare GET string
  getStr += apiKey;

  getStr += "&field3=";
  getStr += String(value3);
  getStr += "&field4=";
  getStr += String(value4);
  getStr += "&field5=";
  getStr += String(value5);
  getStr += "&field6=";
  getStr += String(value6);
  getStr += "&field7=";
  getStr += String(value7);
  getStr += "&field8=";
  getStr += String(value8);

  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  espSerial.println(cmd);

  if (DEBUG)  //Serial.println(cmd);

    delay(100);
  if (espSerial.find(">")) {
    espSerial.print(getStr);
    // if (DEBUG)  //Serial.print(getStr);
  }
  else {
    espSerial.println("AT+CIPCLOSE");
    // alert user
    if (DEBUG)   //Serial.println("AT+CIPCLOSE");
      returnfalse;
  }
  returntrue;
}




voiddata()
{
  thingSpeakWrite(f3, f4, f5, f6, f7, f8); // Write values to thingspeak
  //delay(1000);
}


voidsenddata()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for GPS");
  lcd.setCursor(0, 1);
  lcd.print("char come ");
  while (Serial.available()) //While there are characters to come from the GPS
  {
    gps.encode(Serial.read());//This feeds the serial NMEA data into the library one char at a time
  }
  if (gps.location.isUpdated()) //This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LAT:");
    lcd.setCursor(4, 0);
    lcd.print(gps.location.lat(), 6);
    lcd.setCursor(0, 1);
    lcd.print("LOG:");
    lcd.setCursor(4, 1);
    lcd.print(gps.location.lng(), 6);

    Serial.print("AT\r");
    delay(500);
    Serial.print("AT+CMGF=1\r");
    delay(500);
    Serial.print("AT+CNMI=2,2,0,0,0\r");
    delay(500);
    Serial.print("AT+CMGS=");
    delay(500);
    Serial.print('"');
    delay(500);
    Serial.print("+919483175986");
    delay(500);
    Serial.print('"');
    delay(500);
    Serial.write(0x0D);
    delay(500);

   if(m==1)
   {
     Serial.print("accelerometer detected ");
   }
   else  if(m==2)
   {
     Serial.print("Switch pressed ");
   }
    Serial.print("LDR =");
    Serial.print(ldrStatus);
    Serial.print(", GAS=");
    Serial.print(gasvalue);
    Serial.print(", Temp");
    Serial.print(tempC);
    Serial.print(", BPM:");
    Serial.print(bpm);
    Serial.print(", LOCATION ");
    Serial.print("https://maps.google.com/maps?q=loc:");
    //Serial.print("*Lattitude=");   
    Serial.print(gps.location.lat(),6);
    Serial.print(",");
    // Serial.print("Longitude=");
    Serial.print(gps.location.lng(), 6);
    //Serial.print(",");
    delay(500);
    Serial.write(0x1A);
    delay(200);

    j = 1;
    // delay(5000);
    delay(1000);

  }
}

voidprintText() {
  // keep the top row for status; place beat count near the right of bottom row
  lcd.setCursor(8, 1);         // column 10, row 1 (second row)
  lcd.print("    ");        // clear bottom row
  lcd.setCursor(8, 1);         // column 10, row 1 (second row)
  lcd.print(beat);
}

// Print BPM on LCD
voidprintBPM() {

  lcd.setCursor(0, 1);
  lcd.print("       ");
  lcd.setCursor(0, 1);
  lcd.print("BPM:");
  lcd.print(bpm);
}


voidloop()
{
  // sensor value reading
  int sensorValue = analogRead(A0);
  ldrStatus = analogRead(ldrPin);
  gasvalue = analogRead(gas);

  // --- Non-blocking temperature handling ---
  // Request a temperature conversion on an interval (does not block loop continuously)
  if (!tempWaiting && (millis() - lastTempRequest >= tempRequestInterval)) {
    sensors.requestTemperatures();    // start conversion (may block briefly for some library versions)
    lastTempRequest = millis();
    tempWaiting = true;
  }

  // Read temperature after conversion time elapsed
  if (tempWaiting && (millis() - lastTempRequest >= tempConversionTime)) {
    tempC = sensors.getTempCByIndex(0);
    tempWaiting = false;
  }
  // --- end temperature handling ---

  //display all data here
  if (!countStatus) {
    // only count if above threshold AND enough time has passed since last beat
    if (sensorValue >600&& (millis() - lastBeatMillis) > beatRefractory) {
      countStatus = true;
      beat++;
      lastBeatMillis = millis();   // record beat time
      Serial.println("Beat Detected!");
      Serial.print("Beat : ");
      Serial.println(beat);
      printText();
    }
  } else {
    if (sensorValue <500) {
      countStatus = false;
      printText();
    }
  }

  if (millis() - millisBefore >15000) {
    bpm = beat * 4;
    beat = 0;
    Serial.print("BPM : ");
    Serial.println(bpm);
    printBPM();
    millisBefore = millis();
  }

  delay(1);        // delay in between reads for stab

   
  lcd.setCursor(5, 0);
  lcd.print("     ");
  lcd.setCursor(0, 0);
  lcd.print("Temp=");
  lcd.print(tempC);
  lcd.setCursor(19, 0);
  lcd.print(" ");
  lcd.setCursor(10, 0);
  lcd.print(" Air.Q=");
  lcd.print(gasvalue );
  lcd.setCursor(16, 1);
  lcd.print("    ");
  lcd.setCursor(12, 1);
  lcd.print("Ldr=");
  lcd.print(ldrStatus);
  lcd.setCursor(0, 2);
  lcd.print("                  ");

  if (ldrStatus >900)
  {
    digitalWrite(relay, HIGH);
    lcd.setCursor(0, 2);
    lcd.print("LDR-LOW LIGHT-ON");
  }
  else{
    digitalWrite(relay, LOW);

  }

  if (gasvalue >200)
  {
    lcd.setCursor(0, 2);
    lcd.print("AIR QUALITY-LOW");
  }
  else{
  }

  if (tempC >40)
  {
    lcd.setCursor(0, 2);
    lcd.print("OVER-TEMPERATURE...");
    digitalWrite(buzzer, HIGH);
  }
  else{
    digitalWrite(buzzer, LOW);
  }

  //==============================================================
  sws = digitalRead(sw);

  if (sws == 0)
  {

    m=2;
    lcd.setCursor(5, 0);
    lcd.print("Reading location ");
    lcd.setCursor(0, 1);
    lcd.print("Sending Location    ");
    delay(2000);
    while (j == 0)
    {
      // senddataac();
      senddata();
      lcd.clear();
      W = 0;
    }
    j = 0;

    for (j1 = 0; j1 <10; j1++)
    {
      str[j1] = 's';
    }

  }

  t6 = millis() - t5;

  f3 = ldrStatus;
  f4 = gasvalue;
  f5 = bpm;
  f6 = tempC;
  f7 = 0;
  f8 = 0;

  if (t6 > uploadInterval)
  {

    lcd.setCursor(0, 3);
    lcd.print("Uploading.....");
    data();
    t5 = millis();
    lcd.setCursor(0, 3);
    lcd.print("              ");
    t6 = 0;
  }

}
