 #include <Servo.h>
#include "MQ135.h"
#include <SoftwareSerial.h>---+
#include <LiquidCrystal.h> 
LiquidCrystal lcd(12,11,6, 5, 4, 3);
int air_quality;
#define DEBUG true
Servo myservo;  // 定义Servo对象来控制
int pos = 0;
SoftwareSerial wifi_module(9,10); // Connect TX pin of esp to the pin 2 of Arduino and RX pin of esp to the pin 3 of Arduino
int buzzer = 7;
int smoke_sensor_pin = A0;
void setup()
{
 myservo.attach(8);
pinMode( buzzer, OUTPUT);
lcd.begin(16,2);
lcd.setCursor (0,0);
lcd.print ("air  ");
lcd.setCursor (0,1);
lcd.print ("suxian ");
delay(100);
  Serial.begin(115200);
  wifi_module.begin(115200); // Set the baudrate according to your esp8266
  pinMode(smoke_sensor_pin, INPUT);
  esp8266_command("AT+RST\r\n",2000,DEBUG); // reset module
  esp8266_command("AT+CWMODE=2\r\n",1000,DEBUG); // configure as access point
  esp8266_command("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  esp8266_command("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  esp8266_command("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
}
void loop()
{ 
MQ135 gasSensor = MQ135(A0);
float air_quality = gasSensor.getPPM();
lcd.setCursor (0, 0);
lcd.print ("Air Quality is ");
lcd.print (air_quality);
lcd.print (" PPM ");
lcd.setCursor (0,1);
if (air_quality<=600)
{
lcd.print("Fresh Air");
digitalWrite(buzzer,LOW);
myservo.write(0);              // 舵机角度写入
delay(10);
}
else if(air_quality>=600 && air_quality<=1000 )
{
lcd.print("Poor Air");
digitalWrite(buzzer, HIGH );
//myservo.write(90);              // 舵机角度写入
//delay(10);
}
else if( air_quality>1000 )
{
myservo.write(90);              // 舵机角度写入
delay(10);                       // 等待转动到指定角度 
}
lcd.scrollDisplayLeft();
delay(100);
int analogSensor = analogRead(smoke_sensor_pin);
if (analogSensor > 1000)
  {
digitalWrite(8,HIGH);
delay(1000);
  }
  else
  {
digitalWrite(8,HIGH);
delay(1000);
  }
  if(wifi_module.available()) 
  {
    if(wifi_module.find("+IPD,"))
    {
     delay(1000);
     int connectionId = wifi_module.read()-48;    
     String webpage = "<h1>IOT Air Pollution System</h1>";
      webpage +="<p>Air Quality is ";
      webpage+= air_quality;
      webpage+=" PPM";
      //webpage += analogSensor;
      webpage +="</p>";
      if (analogSensor > 600)
  {
    webpage +="<h5>DANGER! Move Somewhere Else</h5>";
  }
  else
  {
    webpage +="<h4>Everything Normal</h4>";
  }
     String cipSend = "AT+CIPSEND=";
     cipSend += connectionId;
     cipSend += ",";
     cipSend +=webpage.length();
     cipSend +="\r\n";
     esp8266_command(cipSend,1000,DEBUG);
     esp8266_command(webpage,1000,DEBUG);
     String closeCommand = "AT+CIPCLOSE="; 
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";
     esp8266_command(closeCommand,3000,DEBUG);
    }
  }
}
String esp8266_command(String command, const int timeout, boolean debug)
{
    String response = "";
    wifi_module.print(command); 
    long int time = millis();
    while( (time+timeout) > millis())
    {
      while(wifi_module.available())
      {    
        char c = wifi_module.read(); 
        response+=c;
      }  
    }    
    if(debug)
    {
     Serial.print(response);
    }
    return response;
}
