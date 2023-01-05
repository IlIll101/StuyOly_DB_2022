#include "SPI.h"
#include "LiquidCrystal.h"

//LEDS use analog pins
#define RED A3
#define GRN A4
#define BLU A5 

const int DRDY = 9, ADCRST = 2, SPICLOCK = 13; //MAX1416 PINS
const int RS = 2, E = 3, D4 = 5, D5 = 6, D6 = 7, D7 = 8; //LCD PINS
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
const int ss=8;
unsigned int adcValue;

 
void setup()
{
  pinMode(RED, OUTPUT);
  pinMode(GRN, OUTPUT);
  pinMode(BLU, OUTPUT);
  lcd.begin(16, 2);
  lcd.print("INIT...");
  delay(100);
  pinMode(DRDY, INPUT);
  pinMode(ADCRST, OUTPUT);
  digitalWrite(ss,HIGH);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  digitalWrite(SPICLOCK,HIGH);
  digitalWrite(ADCRST,HIGH);
  delay(1000);
  digitalWrite(ADCRST,LOW);
  delay(1000);
  digitalWrite(ADCRST,HIGH);
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("READY!");
  delay(300);
}

void MAX1416_SerialInit()//You can modify it for handle channels
{
  //series of commandbit    
  digitalWrite(ss,LOW); // Enable ADC SPI
  
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);  
  SPI.transfer(0xFF); 
  SPI.transfer(0xFF);

  digitalWrite(ss,HIGH); // Disable ADC SPI
}
 
void MAX1416_Config()//You can modify it for handle channels
{
  //series of commandbit    
  digitalWrite(ss,LOW); // Enable ADC SPI
  
  //Write OP
  SPI.transfer(0x20);//command for comm reg to select ch1 and write to clock register 
  delay(100);
  SPI.transfer(0xA7);//command for clock reg to set 2,4576Mhz                                  
  //End Write OP
  delay(100);
  //Write OP
  SPI.transfer(0x10);//command for comm reg to write setup register 
  delay(100);
  SPI.transfer(0x44);//command for setup reg to self calibration,unipolar,unbuffered,     
  //End Write OP

  digitalWrite(ss,HIGH); // Disable ADC SPI
}
 
void MAX1416_WaitForData_Soft() 
{
      char DataNotReady = 0x80;
      
      digitalWrite(ss,LOW); // Enable ADC SPI
      
      while(DataNotReady) // wait for end of conversion 
      {
          // Read OP
          SPI.transfer(0x08);//command for comm reg to read  (dec   8)
          DataNotReady =SPI.transfer(0x00); // Read comm register
          // End Read OP
          Serial.println(DataNotReady,BIN);
          DataNotReady &= 0x80;
      }
      
      digitalWrite(ss,HIGH); // Disable ADC SPI
}

void MAX1416_WaitForData_Hard() 
{
      char DataNotReady = 1;
      char value;

      while(DataNotReady) // wait for end of conversion 
      {
          // Read OP
          value = digitalRead(DRDY); // Read comm register
           if (value == LOW)
             DataNotReady = 0;
           else
             DataNotReady = 1;
          // End Read OP
          //Serial.println("NOT READY");
      }
}

byte MAX1416_ReadSetupReg() //You can modify it to read other channels
{

      byte myByte;
      
      digitalWrite(ss,LOW); // Enable ADC SPI
      
      // READ Data OPERATION
      SPI.transfer(0x18);//command for the comm to read register register 00011000
      //read 8bit of data
      myByte = SPI.transfer(0x00);
      // End Read Data
      Serial.print(myByte,BIN);
      //delay(2000);
      digitalWrite(ss,HIGH); // Disable ADC SPI
    
      
      return myByte;
}

 
unsigned int MAX1416_ReadCH0Data() //You can modify it to read other channels
{
      unsigned int uiData;
      byte highByte;
      byte lowByte;
      
      digitalWrite(ss,LOW); // Enable ADC SPI
      
      // READ Data OPERATION
      SPI.transfer(0x38);//command for the comm to read data register for channel 1 (dec  56)
      //read 16bit of data ADC
      highByte = SPI.transfer(0x00);
      lowByte = SPI.transfer(0x00);
      // End Read Data
      
      digitalWrite(ss,HIGH); // Disable ADC SPI
    
      
      uiData = highByte;
      uiData <<= 8;
      uiData |= lowByte;
      
      return uiData;
}
 
void loop()
{
  //digitalWrite(ss,LOW); // Enable ADC SPI
  delay(100); 
  MAX1416_Config();
  delay(100);
  double volt;
  MAX1416_ReadSetupReg();
  while(1)
  {
      
      //MAX1416_WaitForData_Soft() ;
      MAX1416_WaitForData_Hard() ;
      delay(10);
      lcd.clear();
      lcd.setCursor(0, 0);
      adcValue = MAX1416_ReadCH0Data();
      lcd.print("analog: ");
      lcd.print(adcValue);
      lcd.setCursor(0, 1);
      volt=double(adcValue)*5/65535;
      lcd.print("volt =");
      lcd.print(volt,4);
      
  }
  digitalWrite(ss,HIGH); // Disable ADC SPI

}
