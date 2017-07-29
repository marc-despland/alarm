#include <Wire.h>


//Libraries
#include <DHT.h>

//Constants
#define DHTPIN 5     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino


//Variables
int hum;  //Stores humidity value
int temp; //Stores temperature value
int mq2;
int mq6;
int mq5;
int mq4;
int mq7;
int mq135;
#define SLAVE_ADDRESS 0x04
int number = 0;
byte data[16];
  
void setup() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);

  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);

  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
  dht.begin();
}
 
void loop() {
 delay(200);
 hum = (int) (10*dht.readHumidity());
 temp= (int) (10*dht.readTemperature());
 mq2= (int) (10*analogRead(A0));
 mq4= (int) (10*analogRead(A1));
 mq5= (int) (10*analogRead(A2));
 mq6= (int) (10*analogRead(A3));
 mq7= (int) (10*analogRead(A6));
 mq135= (int) (10*analogRead(A7));
 data[0]=hum;
 data[1]=hum >> 8;
 data[2]=temp;
 data[3]=temp >> 8; 
 data[4]=mq2;
 data[5]=mq2 >> 8; 
 data[6]=mq4;
 data[7]=mq4 >> 8; 
 data[8]=mq5;
 data[9]=mq5 >> 8; 
 data[10]=mq6;
 data[11]=mq6 >> 8; 
 data[12]=mq7;
 data[13]=mq7 >> 8; 
 data[14]=mq135;
 data[15]=mq135 >> 8; 
}
 
// callback for received data
void receiveData(int byteCount){
  while(Wire.available()) {
    number = Wire.read();
  }
}
 
// callback for sending data
void sendData(){
  Wire.write(data,16);
}
