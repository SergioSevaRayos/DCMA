#include <Wire.h>
#include <BluetoothSerial.h>
#include <MPU6050.h>

BluetoothSerial SerialBT;
MPU6050 mpu;

const int pinAnalogo = 34;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_SSR");
  delay(1000);
  pinMode(pinAnalogo, INPUT);

  Wire.begin();
  mpu.initialize();
}

void loop() {
  int16_t accelX, accelY, accelZ;
  int16_t gyroX, gyroY, gyroZ;

  mpu.getAcceleration(&accelX, &accelY, &accelZ);
  mpu.getRotation(&gyroX, &gyroY, &gyroZ);
  
  Serial.print("AccelX: ");
  Serial.print(accelX);
  Serial.print(" AccelY: ");
  Serial.print(accelY);
  Serial.print(" AccelZ: ");
  Serial.print(accelZ);

  Serial.print("  GyroX: ");
  Serial.print(gyroX);
  Serial.print("  GyroY: ");
  Serial.print(gyroY);
  Serial.print("  GyroZ: ");
  Serial.println(gyroZ);

  if (SerialBT.connected()) {
    SerialBT.print("AccelX: ");
    SerialBT.print(accelX);
    SerialBT.print("  AccelY: ");
    SerialBT.print(accelY);
    SerialBT.print("  AccelZ: ");
    SerialBT.print(accelZ);

    SerialBT.print("  GyroX: ");
    SerialBT.print(gyroX);
    SerialBT.print("  GyroY: ");
    SerialBT.print(gyroY);
    SerialBT.print("  GyroZ: ");
    SerialBT.println(gyroZ);

    SerialBT.flush();
  }

  delay(50);
}
