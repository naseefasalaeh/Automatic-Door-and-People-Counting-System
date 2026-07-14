#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define TRIG_IN 3
#define ECHO_IN 4
#define TRIG_OUT 6
#define ECHO_OUT 7
#define SERVO_PIN 5

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

int peopleCount = 0;
bool isEntering = false;
bool isExiting = false;
bool justEntered = false;
unsigned long inTime = 0;
unsigned long outTime = 0;
const unsigned long maxDelay = 3000; // 3 วินาที
const unsigned long doorDelay = 3000;
unsigned long doorOpenTime = 0;

void setup() {
  pinMode(TRIG_IN, OUTPUT);
  pinMode(ECHO_IN, INPUT);
  pinMode(TRIG_OUT, OUTPUT);
  pinMode(ECHO_OUT, INPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // ปิดประตู

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("People: Ready");
  lcd.setCursor(0, 1);
  lcd.print("In Room: 0");

  Serial.begin(9600);
}

float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return (duration * 0.0343) / 2;
}

void loop() {
  float distIn = readDistance(TRIG_IN, ECHO_IN);
  float distOut = readDistance(TRIG_OUT, ECHO_OUT);
  unsigned long currentTime = millis();

  // เริ่มเข้า
  if (distIn > 0 && distIn < 10 && !isEntering && !isExiting) {
    isEntering = true;
    inTime = currentTime;
    myServo.write(180);
    doorOpenTime = currentTime;
    lcd.setCursor(0, 0);
    lcd.print("People: In     ");
  }

  // เข้าเสร็จ
  if (isEntering && distOut > 0 && distOut < 10 && (currentTime - inTime < maxDelay)) {
    peopleCount++;
    isEntering = false;
    justEntered = true;
    myServo.write(0);
    lcd.setCursor(0, 0);
    lcd.print("People: In OK  ");
    delay(1000);
    while (readDistance(TRIG_IN, ECHO_IN) < 10 || readDistance(TRIG_OUT, ECHO_OUT) < 10) {
      delay(200);
    }
    delay(1000);
    justEntered = false;
  }

  // เริ่มออก
  if (distOut > 0 && distOut < 10 && !isExiting && !isEntering && distIn >= 15 && !justEntered) {
    isExiting = true;
    outTime = currentTime;
    myServo.write(180);
    doorOpenTime = currentTime;
    lcd.setCursor(0, 0);
    lcd.print("People: Out    ");
  }

  // ออกเสร็จ
  if (isExiting && distIn > 0 && distIn < 10 && (currentTime - outTime < maxDelay)) {
    if (peopleCount > 0) {
      peopleCount--;
    }
    isExiting = false;
    myServo.write(0);
    lcd.setCursor(0, 0);
    lcd.print("People: Out OK ");
    delay(1000);
    while (readDistance(TRIG_IN, ECHO_IN) < 10 || readDistance(TRIG_OUT, ECHO_OUT) < 10) {
      delay(200);
    }
  }

  // ถ้าเข้าหรือออกไม่สมบูรณ์ภายใน maxDelay → ปิดประตูเฉยๆ
  if (isEntering && (currentTime - inTime >= maxDelay)) {
    isEntering = false;
    myServo.write(0);
  }

  if (isExiting && (currentTime - outTime >= maxDelay)) {
    isExiting = false;
    myServo.write(0);
  }

  // ปิดประตูอัตโนมัติหลัง delay
  if (millis() - doorOpenTime >= doorDelay) {
    myServo.write(0);
  }

  // แสดงจำนวนคนในห้อง
  lcd.setCursor(0, 1);
  lcd.print("In Room: ");
  lcd.print(peopleCount);
  lcd.print("   ");

  delay(100);
}