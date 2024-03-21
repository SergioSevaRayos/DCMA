#include <Wire.h>
#include <BluetoothSerial.h>
#include <MPU6050.h>

BluetoothSerial SerialBT;
MPU6050 mpu;

const int pinAnalogo = 34;
const int ledPin = 2; // LED interno del ESP32 NodeMCU 32S

int16_t prevAccelX, prevAccelY, prevAccelZ;
int16_t prevGyroX, prevGyroY, prevGyroZ;

float variationPercentage = 20.0; // Porcentaje de variación permitido (+/-)

const int numReadings = 10; // Número de lecturas para el filtro de promedio móvil
int16_t accelXReadings[numReadings];
int16_t accelYReadings[numReadings];
int16_t accelZReadings[numReadings];
int16_t gyroXReadings[numReadings];
int16_t gyroYReadings[numReadings];
int16_t gyroZReadings[numReadings];
int readingIndex = 0; // Renombrado de 'index' a 'readingIndex'

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_SSR");
  delay(1000);
  pinMode(pinAnalogo, INPUT);
  pinMode(ledPin, OUTPUT); // Configura el pin del LED como salida

  Wire.begin();
  mpu.initialize();

  // Inicializa las lecturas previas con los valores iniciales
  mpu.getAcceleration(&prevAccelX, &prevAccelY, &prevAccelZ);
  mpu.getRotation(&prevGyroX, &prevGyroY, &prevGyroZ);

  // Inicializa los arreglos de lecturas para el filtro de promedio móvil
  for (int i = 0; i < numReadings; i++) {
    accelXReadings[i] = prevAccelX;
    accelYReadings[i] = prevAccelY;
    accelZReadings[i] = prevAccelZ;
    gyroXReadings[i] = prevGyroX;
    gyroYReadings[i] = prevGyroY;
    gyroZReadings[i] = prevGyroZ;
  }
}

void loop() {
  int16_t accelX, accelY, accelZ;
  int16_t gyroX, gyroY, gyroZ;

  mpu.getAcceleration(&accelX, &accelY, &accelZ);
  mpu.getRotation(&gyroX, &gyroY, &gyroZ);
  
  // Actualiza los arreglos de lecturas para el filtro de promedio móvil
  accelXReadings[readingIndex] = accelX;
  accelYReadings[readingIndex] = accelY;
  accelZReadings[readingIndex] = accelZ;
  gyroXReadings[readingIndex] = gyroX;
  gyroYReadings[readingIndex] = gyroY;
  gyroZReadings[readingIndex] = gyroZ;
  readingIndex = (readingIndex + 1) % numReadings;

  // Calcula la media de las lecturas para el filtro de promedio móvil
  float accelXAvg = 0;
  float accelYAvg = 0;
  float accelZAvg = 0;
  float gyroXAvg = 0;
  float gyroYAvg = 0;
  float gyroZAvg = 0;
  for (int i = 0; i < numReadings; i++) {
    accelXAvg += accelXReadings[i];
    accelYAvg += accelYReadings[i];
    accelZAvg += accelZReadings[i];
    gyroXAvg += gyroXReadings[i];
    gyroYAvg += gyroYReadings[i];
    gyroZAvg += gyroZReadings[i];
  }
  accelXAvg /= numReadings;
  accelYAvg /= numReadings;
  accelZAvg /= numReadings;
  gyroXAvg /= numReadings;
  gyroYAvg /= numReadings;
  gyroZAvg /= numReadings;

  // Calcula el cambio porcentual en las lecturas
  float accelChangeX = abs((accelXAvg - prevAccelX) / (float)prevAccelX) * 100;
  float accelChangeY = abs((accelYAvg - prevAccelY) / (float)prevAccelY) * 100;
  float accelChangeZ = abs((accelZAvg - prevAccelZ) / (float)prevAccelZ) * 100;

  // Actualiza las lecturas previas
  prevAccelX = accelXAvg;
  prevAccelY = accelYAvg;
  prevAccelZ = accelZAvg;

  // Lee el monitor serial para ajustar la sensibilidad
if (Serial.available() > 0) {
  int input = Serial.parseInt(); // Lee el número entero ingresado

  // Utiliza un switch-case para asignar el porcentaje de sensibilidad correspondiente
  switch(input) {
    case 1:
      variationPercentage = 10.0;
      break;
    case 2:
      variationPercentage = 20.0;
      break;
    case 3:
      variationPercentage = 30.0;
      break;
    // Agrega más casos según sea necesario
    default:
      Serial.println("Opción inválida.");
      return; // Sal del bucle si la opción es inválida
  }
  
  Serial.print("Nuevo porcentaje de variación: ");
  Serial.println(variationPercentage);
}
// Lee Bluetooth para ajustar la sensibilidad
while (SerialBT.available() > 0) {
  int input = SerialBT.parseInt(); // Lee el número entero recibido por Bluetooth

  // Utiliza un switch-case para asignar el porcentaje de sensibilidad correspondiente
  switch(input) {
    case 1:
      variationPercentage = 10.0;
      break;
    case 2:
      variationPercentage = 20.0;
      break;
    case 3:
      variationPercentage = 30.0;
      break;
    // Agrega más casos según sea necesario
    default:
      SerialBT.println("Opción inválida.");
      return; // Sal del bucle si la opción es inválida
  }
  
  SerialBT.print("Nuevo porcentaje de variación: ");
  SerialBT.println(variationPercentage);
}


  // Comprueba si alguna variación excede el porcentaje permitido
  if (accelChangeX > variationPercentage || accelChangeY > variationPercentage || accelChangeZ > variationPercentage) {
    digitalWrite(ledPin, HIGH); // Enciende el LED
  } else {
    digitalWrite(ledPin, LOW); // Apaga el LED si no hay variación
  }
  
  Serial.print("Porcentaje de sensibilidad: ");
  Serial.println(variationPercentage);

  Serial.print("AccelX: ");
  Serial.print(accelXAvg);
  Serial.print(" AccelY: ");
  Serial.print(accelYAvg);
  Serial.print(" AccelZ: ");
  Serial.print(accelZAvg);

  Serial.print("  GyroX: ");
  Serial.print(gyroXAvg);
  Serial.print("  GyroY: ");
  Serial.print(gyroYAvg);
  Serial.print("  GyroZ: ");
  Serial.println(gyroZAvg);

  if (SerialBT.connected()) {
    SerialBT.print("Porcentaje de sensibilidad: ");
    SerialBT.println(variationPercentage);

    SerialBT.print("AccelX: ");
    SerialBT.print(accelXAvg);
    SerialBT.print("  AccelY: ");
    SerialBT.print(accelYAvg);
    SerialBT.print("  AccelZ: ");
    SerialBT.print(accelZAvg);

    SerialBT.print("  GyroX: ");
    SerialBT.print(gyroXAvg);
    SerialBT.print("  GyroY: ");
    SerialBT.print(gyroYAvg);
    SerialBT.print("  GyroZ: ");
    SerialBT.println(gyroZAvg);

    SerialBT.flush();
  }

  delay(50);
}
