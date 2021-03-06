/*------------------------------------------------------------------------------
  LIDARLite Arduino Library
  v3/GetDistanceI2c
  This example shows how to initialize, configure, and read distance from a
  LIDAR-Lite connected over the I2C interface.
  Connections:
  LIDAR-Lite 5 Vdc (red) to Arduino 5v
  LIDAR-Lite I2C SCL (green) to Arduino SCL
  LIDAR-Lite I2C SDA (blue) to Arduino SDA
  LIDAR-Lite Ground (black) to Arduino GND
  (Capacitor recommended to mitigate inrush current when device is enabled)
  680uF capacitor (+) to Arduino 5v
  680uF capacitor (-) to Arduino GND
  See the Operation Manual for wiring diagrams and more information:
  http://static.garmin.com/pumac/LIDAR_Lite_v3_Operation_Manual_and_Technical_Specifications.pdf
------------------------------------------------------------------------------*/

#include <Wire.h>
#include <avr/wdt.h>
#include <LIDARLite.h>

LIDARLite myLidarLite;

int Switch = 9; //Use pin 12 to switch POWER ENABLE on and off
unsigned long UpdatePeriod = 300; //Update only once every 1 second
unsigned int Data = 0; //Global handler used to move data
volatile bool Update = true; //Update flag, true on startup for initial sample
volatile int Count = 0; 
void setup()
{
  wdt_enable(WDTO_8S);
  PRR = 0x4D; //Shut down Timer1, Timer2, SPI, ADC
  pinMode(2, INPUT); //Switch all unused control pins to be inputs
  pinMode(15, INPUT);
  pinMode(3, INPUT); 
  
  pinMode(Switch, OUTPUT);
  digitalWrite(Switch, LOW); //Enable power to device for startup
  Serial.begin(9600); // Initialize serial connection to display distance readings

  /*
    begin(int configuration, bool fasti2c, char lidarliteAddress)
    Starts the sensor and I2C.
    Parameters
    ----------------------------------------------------------------------------
    configuration: Default 0. Selects one of several preset configurations.
    fasti2c: Default 100 kHz. I2C base frequency.
      If true I2C frequency is set to 400kHz.
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */
  myLidarLite.begin(0, true); // Set configuration to default and I2C to 400 kHz

  /*
    configure(int configuration, char lidarliteAddress)
    Selects one of several preset configurations.
    Parameters
    ----------------------------------------------------------------------------
    configuration:  Default 0.
      0: Default mode, balanced performance.
      1: Short range, high speed. Uses 0x1d maximum acquisition count.
      2: Default range, higher speed short range. Turns on quick termination
          detection for faster measurements at short range (with decreased
          accuracy)
      3: Maximum range. Uses 0xff maximum acquisition count.
      4: High sensitivity detection. Overrides default valid measurement detection
          algorithm, and uses a threshold value for high sensitivity and noise.
      5: Low sensitivity detection. Overrides default valid measurement detection
          algorithm, and uses a threshold value for low sensitivity and noise.
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */
  myLidarLite.configure(0); // Change this number to try out alternate configurations
  digitalWrite(Switch, HIGH); //Turn off device after startup
  TCCR0B = TCCR0B & 0xF8;
  TCCR0B = TCCR0B | 0x05;
//  Serial.println(TCCR0B);
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  Serial.println("LiDAR-Lite-V3");
  Serial.println("License CC-BY-SA-4.0");
  Serial.println("Bobby Schulz\nNorthern Widget LLC");
  Serial.println("When God said “Let there be light” he surely must have meant perfectly coherent light.\n-Charles Townes");
  
}

void loop()
{
  /*
    distance(bool biasCorrection, char lidarliteAddress)
    Take a distance measurement and read the result.
    Parameters
    ----------------------------------------------------------------------------
    biasCorrection: Default true. Take aquisition with receiver bias
      correction. If set to false measurements will be faster. Receiver bias
      correction must be performed periodically. (e.g. 1 out of every 100
      readings).
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */
//  unsigned long LocalTime = millis();
//  while(millis() - LocalTime < UpdatePeriod);

  SMCR = 0x01; //Enable idle sleep mode
  asm("sleep"); //Put device to sleep
  SMCR = 0x00; //Clear sleep mode bit

  if(Update) { //If update flag is set
    digitalWrite(Switch, LOW); //Turn device on to take measurment
//    delay(2); //Wait for 30ms with new div
    int Rollover = Count;
    while(Count - Rollover < 3); //Increment 2 counts (16 ~ 32ms)
    // Take a measurement with receiver bias correction and print to serial terminal
  //  Serial.println(myLidarLite.distance());
  
//    // Take 99 measurements without receiver bias correction and print to serial terminal
//    for(int i = 0; i < 100; i++) //Clear buffer?
//    {
//      myLidarLite.distance(true);
//    }
    Data = myLidarLite.distance(true)*10; //Return value in mm to corespond with Maxbotix convention
    digitalWrite(Switch, HIGH); //Turn device off when done
    wdt_reset(); //Reset watchdog after compleating read
    Update = false; //Clear update flag
  }
  
//  
//  unsigned long LocalTime = millis();
//  while(millis() - LocalTime < UpdatePeriod) {
//    Serial.print(Data);
//    Serial.println(" cm"); //Remove newline??
//    delay(10);
//  }
}

SIGNAL(TIMER0_COMPA_vect) 
{
//  static int Count = 0;
  if(Count % 10 == 0) {  //Update value every 150ms
    Serial.print("R"); //"Range" prefix, again following maxbotix convention
    Serial.print(Data);
    Serial.print("\r"); //Send carrrage return
//    delay(10);
//    Count = 0; //Clear counter
  }
  if(Count > UpdatePeriod){
    Update = true; //Set update flag
    Count = 0; //Clear count
  }
  Count++;
} 
