#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <VL53L0X.h>
#include <Adafruit_I2CDevice.h>
// Khai báo đối tượng cho mỗi cảm biến
VL53L0X sensor1, sensor2, sensor3, sensor4, sensor5;
#define SHT_LOX1 25 // left 90
#define SHT_LOX2 32 // left 45
#define SHT_LOX3 33 //  Front
#define SHT_LOX4 15 // right 45
#define SHT_LOX5 5  // right90
#define ENA 14      // Enable/speed motors Right        GPIO14(D19)
#define IN1 18      // L298N in1 motors Rightx          GPIO15(D18)
#define IN2 19      // L298N in2 motors Right           GPIO13(D5)

#define ENB 4  // Enable/speed motors Left         GPIO12(D14)
#define IN3 12 // L298N in3 motors Left            GPIO2(D26)
#define IN4 13 // L298N in4 motors Left            GPIO0(D27)

float leftSensor90, rightSensor90, frontSensor, rightSensor45, leftSensor45;

#define HasleftWallValue_45 400
#define HasrightWallValue_45 400
#define HasleftWallValue_90 150
#define HasrightWallValue_90 150
#define rightWallValue 300
#define leftWallValue 300

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);
  pinMode(SHT_LOX3, OUTPUT);
  pinMode(SHT_LOX4, OUTPUT);
  pinMode(SHT_LOX5, OUTPUT);
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);
  digitalWrite(SHT_LOX4, LOW);
  digitalWrite(SHT_LOX5, LOW);
  // Khởi tạo và bắt đầu cảm biến 1
  sensor1.init(SHT_LOX1);
  sensor1.setTimeout(500);
  sensor1.startContinuous();
  // Đặt địa chỉ I2C cho cảm biến 1 (vd: 0x30)
  sensor1.setAddress(0x30);
  sensor1.setMeasurementTimingBudget(30000) ;
  // //

  //   // Khởi tạo và bắt đầu cảm biến 2
  digitalWrite(SHT_LOX2, HIGH);

  sensor2.init(SHT_LOX2);
  sensor2.setTimeout(500);
  sensor2.startContinuous();
  // Đặt địa chỉ I2C cho cảm biến 2 (vd: 0x31)
  sensor2.setAddress(0x31);
  sensor2.setMeasurementTimingBudget(30000); 

  //   // Khởi tạo và bắt đầu cảm biến 3
  digitalWrite(SHT_LOX3, HIGH);
  sensor3.init(SHT_LOX3);
  sensor3.setTimeout(500);
  sensor3.startContinuous();
  // Đặt địa chỉ I2C cho cảm biến 3 (vd: 0x32)
  sensor3.setAddress(0x32);
    sensor3.setMeasurementTimingBudget(30000); 


  //   // Khởi tạo và bắt đầu cảm biến 4
  digitalWrite(SHT_LOX4, HIGH);
  sensor4.init();
  sensor4.setTimeout(500);
  sensor4.startContinuous();
  // Đặt địa chỉ I2C cho cảm biến 4 (vd: 0x33)
  sensor4.setAddress(0x33);
    sensor4.setMeasurementTimingBudget(30000); 

  //   // Khởi tạo và bắt đầu cảm biến 5
  digitalWrite(SHT_LOX5, HIGH);
  sensor5.init();
  sensor5.setTimeout(500);
  sensor5.startContinuous();
  // Đặt địa chỉ I2C cho cảm biến 5 (vd: 0x34)
  sensor5.setAddress(0x34);
    sensor5.setMeasurementTimingBudget(30000); 
}

void read_sensors()
{
  leftSensor90 = sensor1.readRangeContinuousMillimeters();
  leftSensor45 = sensor2.readRangeContinuousMillimeters();
  frontSensor = sensor3.readRangeContinuousMillimeters();
  rightSensor45 = sensor4.readRangeContinuousMillimeters();
  rightSensor90 = sensor5.readRangeContinuousMillimeters() - 85;

  // // In dữ liệu ra Serial Monitor
  Serial.print("left90: "); // ok
  Serial.println(leftSensor90);

  Serial.print("left45: "); // ok
  Serial.println(leftSensor45);

  Serial.print("front : "); // ok
  Serial.println(frontSensor);

  Serial.print("right45 : "); // ok
  Serial.println(rightSensor45);

  Serial.print("right 90: "); // ok
  Serial.println(rightSensor90);
  // delay(1000);
}
void FORWARD()
{

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 100);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, 100);
}

void stop()
{

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void quaytrai45()
{

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 160);
  analogWrite(ENB, 0);
  delay(450);
  stop();
  delay(300);
}

void quayphai45()
{

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 160);
  delay(450);
  stop();
  delay(300);
}

void quay90()
{
  // Stop the robot's motion

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 150);
  analogWrite(ENB, 150);
  delay(400);

  stop();
  delay(1000);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 30);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 30);
  delay(800);
  stop();
  delay(500);
}
void wallleft()
{

  int32_t Err = 0, P, D, old_Error = 0;
  bool useIRSensor = true;
  int32_t ofs;
  uint16_t speed_1 = 110;
  uint16_t speed_2 = 110;
  if (leftSensor45 < HasleftWallValue_45 && leftSensor90 < HasleftWallValue_90 && rightSensor45 < HasrightWallValue_45 && rightSensor90 < HasrightWallValue_90)
  {
    Err = (int32_t)rightSensor45 - leftSensor45;
    useIRSensor = true;
  }
  else if (leftSensor45 < HasleftWallValue_45 && leftSensor90 < HasleftWallValue_90)
  {
    Err = (int32_t)leftWallValue - leftSensor45;
    D = Err - old_Error;
    old_Error = Err;
    useIRSensor = true;
  }
  else if (rightSensor45 < HasrightWallValue_45 && rightSensor90 < HasrightWallValue_90)
  {
    Err = (int32_t)rightSensor45 - rightWallValue;
    D = Err - old_Error;
    old_Error = Err;
    useIRSensor = true;
  }
  if (useIRSensor)
  {
    P = 0.7 * Err + D * 0; /// OK0.75
  }
  P = max(-20, min(P, 20));
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, speed_1 - P);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, speed_2 + P);

  ///  NẾU CẢM BIẾN TRÁI LỚN HƠN TƯỞNG PHẢI === RẺ TRÁI
  // if (leftSensor90 > HasleftWallValue_90)
  // {
  //   stop();
  //   delay(200);
  //   quaytrai45();
  //   stop();
  //   delay(500);
  //   FORWARD();
  //   delay(500);

  //   if (leftSensor45 < HasleftWallValue_45 && leftSensor90 < HasleftWallValue_90 && rightSensor45 < HasrightWallValue_45 && rightSensor90 < HasrightWallValue_90)
  //   {
  //     Err = (int32_t)rightSensor45 - leftSensor45;
  //     useIRSensor = true;
  //   }
  //   else if (leftSensor45 < HasleftWallValue_45 && leftSensor90 < HasleftWallValue_90)
  //   {
  //     Err = (int32_t)leftWallValue - leftSensor45;
  //     D = Err - old_Error;
  //     old_Error = Err;
  //     useIRSensor = true;
  //   }
  //   else if (rightSensor45 < HasrightWallValue_45 && rightSensor90 < HasrightWallValue_90)
  //   {
  //     Err = (int32_t)rightSensor45 - rightWallValue;
  //     D = Err - old_Error;
  //     old_Error = Err;
  //     useIRSensor = true;
  //   }
  //   if (useIRSensor)
  //   {
  //     P = 0.4  * Err + D * 0;
  //   }
  //   P = max(-20, min(P, 20));
  //   digitalWrite(IN2, HIGH);
  //   analogWrite(ENA, speed_1 + P);
  //   digitalWrite(IN4, HIGH);
  //   analogWrite(ENB, speed_2 - P);
  // }
  // ///  KHÔNG CÓ TƯỜNG PHẢI, TƯỜNG TRƯỚC NHỎ HƠN CHỈ ĐỊNH ,CÓ TƯỜNG TRÁI  == RẺ PHẢI

  // else if (rightSensor90 > HasleftWallValue_90 && frontSensor < 250 ) // neu tuowng phai lon chi dinh
  // {
  //   stop();
  //   delay(500);
  //   quayphai45();
  //   stop();
  //   delay(500);
  //   FORWARD();
  //   delay(500);
  //   if (leftSensor45 < HasleftWallValue_45 && leftSensor90 < HasleftWallValue_90 && rightSensor45 < HasrightWallValue_45 && rightSensor90 < HasrightWallValue_90)
  //   {
  //     Err = (int32_t)rightSensor45 - leftSensor45;
  //     useIRSensor = true;
  //   }
  //   else if (leftSensor45 < HasleftWallValue_45 && leftSensor90 < HasleftWallValue_90)
  //   {
  //     Err = (int32_t)leftWallValue - leftSensor45;
  //     D = Err - old_Error;
  //     old_Error = Err;
  //     useIRSensor = true;
  //   }
  //   else if (rightSensor45 < HasrightWallValue_45 && rightSensor90 < HasrightWallValue_90)
  //   {
  //     Err = (int32_t)rightSensor45 - rightWallValue;
  //     D = Err - old_Error;
  //     old_Error = Err;
  //     useIRSensor = true;
  //   }
  //   if (useIRSensor)
  //   {
  //     P = 0.4 * Err + D * 0;
  //   }
  //   P = max(-20, min(P, 20));
  //   digitalWrite(IN2, HIGH);
  //   analogWrite(ENA, speed_1 + P);
  //   digitalWrite(IN4, HIGH);
  //   analogWrite(ENB, speed_2 - P);
  // }
  // if (frontSensor < 150  )
  // {
  //   stop();
  //   delay(500);
  //   quay90();
  // }
  
  }
// }
void loop()
{
  read_sensors();
  wallleft();
}