#include <stdlib.h>
#include "DHT.h"

#include "WizFi360.h"

/* Baudrate */
#define SERIAL_BAUDRATE   115200
#define SERIAL3_BAUDRATE  115200

/* Sensor */
#define DHTTYPE DHT11
#define DHTPIN 2
int tempValue, humiValue, hicValue;

/* LED and Buzzer */
int buzzer = 3;
int blue_hum = 4, green_hum = 5, red_hum = 6;
int blue_temp = 7, green_temp = 8, red_temp = 9;
int heater = 10, blower = 11;

/* Wifi */
char ssid[] = "Sekolah Robot Indonesia";       // your network SSID (name)
char pass[] = "sadapsadap";   // your network password
int status = WL_IDLE_STATUS;  // the Wifi radio's status

/* Thinkspeak */
char server[] = "api.thingspeak.com"; // server address
String apiKey ="7HYKF84XX6QS021W";    // apki key

// sensor buffer
char temp_buf[10];
char humi_buf[10];
char hic_buf[10];

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 30000L; // delay between updates, in milliseconds

// Initialize the Ethernet client object
WiFiClient client;
// Initialize the DHT object
DHT dht(DHTPIN, DHTTYPE);

void setup() {              
  //initialize sensor
  dht.begin();
  
  // initialize serial for debugging
  Serial.begin(SERIAL_BAUDRATE);
  // initialize serial for WizFi360 module
  Serial3.begin(SERIAL3_BAUDRATE);
  // initialize WizFi360 module
  WiFi.init(&Serial3);
  
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // Pin LED and Buzzer
  pinMode(buzzer, OUTPUT);
  pinMode(blue_hum, OUTPUT);  pinMode(green_hum, OUTPUT);   pinMode(red_hum, OUTPUT);
  pinMode(blue_temp, OUTPUT); pinMode(green_temp, OUTPUT);  pinMode(red_temp, OUTPUT);
  pinMode(heater, OUTPUT);    pinMode(blower, OUTPUT);
  

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  Serial.println("You're connected to the network");
  
  printWifiStatus();

  //First transmitting
  sensorRead();
  thingspeakTrans();
}

void loop()
{
  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only   
  while (client.available()) {
    char c = client.read();
    Serial.print("recv data: ");
    Serial.write(c);
    Serial.println();
  }
  
  // if 30 seconds have passed since your last connection,
  // then connect again and send data
  if (millis() - lastConnectionTime > postingInterval) {
    sensorRead();
    thingspeakTrans();
  }

  if (tempValue>0 and tempValue<=35){
    digitalWrite(red_temp, LOW);
    digitalWrite(green_temp, LOW);
    digitalWrite(blue_temp, HIGH);
    digitalWrite(buzzer, LOW);
    digitalWrite(heater, LOW);
  }else if(tempValue>35 and tempValue<=42){
    digitalWrite(red_temp, LOW);
    digitalWrite(green_temp, HIGH);
    digitalWrite(blue_temp, LOW);
    digitalWrite(buzzer, LOW);
    digitalWrite(heater, LOW);
  }else if(tempValue>42 and tempValue<=100){
    digitalWrite(red_temp, HIGH);
    digitalWrite(green_temp, LOW);
    digitalWrite(blue_temp, LOW);
    digitalWrite(buzzer, HIGH);
    digitalWrite(heater, HIGH);
  }

  if(humiValue>0 and humiValue<=40){
    digitalWrite(red_hum, LOW);
    digitalWrite(green_hum, LOW);
    digitalWrite(blue_hum, HIGH);
    digitalWrite(buzzer, LOW);
    digitalWrite(blower, HIGH);
  }else if(humiValue>40 and humiValue<=70){
    digitalWrite(red_hum, LOW);
    digitalWrite(green_hum, HIGH);
    digitalWrite(blue_hum, LOW);
    digitalWrite(buzzer, LOW);
    digitalWrite(blower, LOW);
  }else if(humiValue>70 and humiValue<=100){
    digitalWrite(red_hum, HIGH);
    digitalWrite(green_hum, LOW);
    digitalWrite(blue_hum, LOW);
    digitalWrite(buzzer, HIGH);
    digitalWrite(blower, LOW);
  }
}

// Read sendsor value
void sensorRead(){
  tempValue=dht.readTemperature();
  humiValue=dht.readHumidity();
  hicValue = dht.computeHeatIndex(tempValue, humiValue, false);
  
  String strTemp = dtostrf(tempValue, 4, 1, temp_buf);
  String strHumi = dtostrf(humiValue, 4, 1, humi_buf);
  String strHic = dtostrf(hicValue, 4, 1, hic_buf);
  
  Serial.print("Temperature: ");
  Serial.println(tempValue);
  Serial.print("Humidity: ");
  Serial.println(humiValue);
  Serial.print("Cds: ");
  Serial.println(hicValue);
}

//Transmitting sensor value to thingspeak
void thingspeakTrans()
{
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    
    // send the Get request
    client.print(F("GET /update?api_key="));
    client.print(apiKey);
    client.print(F("&field1="));
    client.print(temp_buf);
    client.print(F("&field2="));
    client.print(humi_buf);
    client.print(F("&field3="));
    client.print(hic_buf);
    client.println();
    // note the time that the connection was made
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
