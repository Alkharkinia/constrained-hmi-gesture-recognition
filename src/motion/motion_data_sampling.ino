#include <MPU9250.h>
#include <Wire.h>

#define I2C_SDA 8
#define I2C_SCL 9

MPU9250 IMU(Wire, 0x68);  // default I2C address

// Gyro and accelerometer offsets
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;
float accelOffsetX = 0, accelOffsetY = 0, accelOffsetZ = 0;

const int numSamples = 120; // samples per gesture (~1 s at 100 Hz)
const int sampleDelay = 10;  // ms delay → ~100 Hz

void calibrateSensor();

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Booting...");
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize IMU
  if (!IMU.begin()) {
    Serial.println("No MPU6050/MPU9250 detected!");
    while (1) { delay(10); }
  }
  Serial.println("MPU9250 initialized!");

  // Optional: Calibrate offsets
  calibrateSensor();

  Serial.println("Enter pressed gesture capture is ready!");
  Serial.println("Press Enter in Serial Monitor to start recording.");
  Serial.println("Format: ax,ay,az,gx,gy,gz");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {  // Enter pressed
      // Countdown
      for (int i = 3; i > 0; i--) {
        Serial.print("Recording starts in ");
        Serial.println(i);
        delay(1000);
      }
      Serial.println("Recording gesture...");

      // Record samples
      for (int n = 0; n < numSamples; n++) {
        IMU.readSensor();

        // Subtract offsets
        float ax = IMU.getAccelX_mss() - accelOffsetX;
        float ay = IMU.getAccelY_mss() - accelOffsetY;
        float az = IMU.getAccelZ_mss() - accelOffsetZ;

        float gx = IMU.getGyroX_rads() - gyroOffsetX;
        float gy = IMU.getGyroY_rads() - gyroOffsetY;
        float gz = IMU.getGyroZ_rads() - gyroOffsetZ;

        // Print CSV line
        Serial.print(ax); Serial.print(",");
        Serial.print(ay); Serial.print(",");
        Serial.print(az); Serial.print(",");
        Serial.print(gx); Serial.print(",");
        Serial.print(gy); Serial.print(",");
        Serial.println(gz);

        delay(sampleDelay);
      }

      Serial.println("Gesture recording complete!");
      Serial.println(); // empty line between gestures
      Serial.println("Press Enter for next gesture...");
    }
  }
}

void calibrateSensor() {
  Serial.println("Calibrating sensor, keep it stationary...");
  const int samples = 500;  // ~5 seconds at 100 Hz
  const float alpha = 0.02; // smoothing factor

  gyroOffsetX = gyroOffsetY = gyroOffsetZ = 0;
  accelOffsetX = accelOffsetY = accelOffsetZ = 0;

  for (int i = 0; i < samples; i++) {
    IMU.readSensor();

    gyroOffsetX = alpha * IMU.getGyroX_rads() + (1 - alpha) * gyroOffsetX;
    gyroOffsetY = alpha * IMU.getGyroY_rads() + (1 - alpha) * gyroOffsetY;
    gyroOffsetZ = alpha * IMU.getGyroZ_rads() + (1 - alpha) * gyroOffsetZ;

    accelOffsetX = alpha * IMU.getAccelX_mss() + (1 - alpha) * accelOffsetX;
    accelOffsetY = alpha * IMU.getAccelY_mss() + (1 - alpha) * accelOffsetY;
    accelOffsetZ = alpha * IMU.getAccelZ_mss() + (1 - alpha) * accelOffsetZ;

    delay(sampleDelay);
  }

  Serial.print("Gyro offsets: ");
  Serial.print(gyroOffsetX); Serial.print(",");
  Serial.print(gyroOffsetY); Serial.print(",");
  Serial.println(gyroOffsetZ);

  Serial.print("Accel offsets: ");
  Serial.print(accelOffsetX); Serial.print(",");
  Serial.print(accelOffsetY); Serial.print(",");
  Serial.println(accelOffsetZ);
}
