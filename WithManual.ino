#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


String  i = "OFF";

ESP8266WebServer server(80);

const int sensorIn = A0;
int mVperAmp = 100; // use 100 for 20A Module and 66 for 30A Module


double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;
double kwh = 0;
String data = "";

const byte interruptPin = D8;
void setup()
{

  Serial.begin(9600);

  pinMode(D2, OUTPUT);
  // pinMode(D8, INPUT); //button
  pinMode(D7, OUTPUT); //puls
  pinMode(D6, OUTPUT); //bulb

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING);// CHANGE, RISING or FALLING

  WiFi.disconnect();
  delay(3000);
  Serial.println("Start");
  WiFi.begin("nishara", "nish1994");
  while ((!(WiFi.status() == WL_CONNECTED))) {
    delay(300);
    Serial.print("...");

  }
  Serial.println("Connected");
  Serial.println((WiFi.localIP()));
  server.begin();

  server.on("/ON", bulbOn);
  server.on("/OFF", bulbOff);
  server.on("/POWER", getPower);
  server.on("/REFRESH", refresh);
  server.on("/", root);

}

void root() {
  Serial.println("Root REQ");
  server.send(200, "text/html", "<!DOCTYPE HTML> <html> Welcome </html>");
}
void bulbOn() {
  i = "ON";
  digitalWrite(D6, HIGH);
  server.send(200, "text/html", "<!DOCTYPE HTML> <html> Switch-ON </html>");
}

void bulbOff() {
  i = "OFF";
  digitalWrite(D6, LOW);
  server.send(200, "text/html", "<!DOCTYPE HTML> <html> Switch-OFF </html>");
}

void getPower() { 
  server.send(200, "text/html", "<!DOCTYPE HTML> <html>"+data+"  kWh</html>");
}

void refresh(){
  if(i=="ON"){
    server.send(200, "text/html", "<!DOCTYPE HTML> <html> Switch-ON </html>");
    }
    else{
     server.send(200, "text/html", "<!DOCTYPE HTML> <html> Switch-OFF </html>"); 
      }
  }


void loop() {
  server.handleClient();

  if (i == "ON") {
    digitalWrite(D7, LOW);
    delay(1);
    digitalWrite(D7, HIGH);
    delay(1);
    Serial.println("Light ON");
  } else if (i == "OFF") {

    digitalWrite(D7, LOW);
    Serial.println("Light OFF");
  }
  //Calculate the Power Consumption
  Voltage = getVPP();
  VRMS = (Voltage / 2.0) * 0.707;
  AmpsRMS = (VRMS * 1000) / mVperAmp;

  double kwhPerSecond = (AmpsRMS * 230 ) / (1000*100);
  kwh = kwh + (kwhPerSecond / 3600);
  //String data = "";
  data = String(kwh, 6);

}

void handleInterrupt() {
  if (i == "ON") {
    i = "OFF";
    digitalWrite(D6, LOW);
  } else if (i == "OFF") {
    i = "ON";
    digitalWrite(D6, HIGH);
  }
}

float getVPP()
{
  float result;

  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  int j=0;
  uint32_t start_time = millis();
  //while ((millis() - start_time) < 1000) //sample for 1 Sec
  while(j<10)
  {
    readValue = analogRead(sensorIn);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the maximum sensor value*/
      minValue = readValue;
    }
    j=j+1;
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 5.0) / 1024.0;

  return result;
}


