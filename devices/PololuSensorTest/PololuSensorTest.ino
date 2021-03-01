




#include <Wire.h>
#include <LSM6.h>
#include <LPS.h>
#include <LIS3MDL.h>

LSM6 imu;
LPS ps;
LIS3MDL mag;

char report[80];

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  if (!imu.init())
  {
    Serial.println("Failed to detect and initialize IMU!");
    while (1);
  }
  imu.enableDefault();
  imu.setAccScale(ACC4g);


  // ** Pressure sensor setup **
  if (!ps.init())
  {
    Serial.println("Failed to autodetect pressure sensor!");
    while (1);
  }

  ps.enableDefault();

  // ** Magenetometer Setup **
  if (!mag.init())
  {
    Serial.println("Failed to detect and initialize magnetometer!");
    while (1);
  }

  mag.enableDefault();
  
}

void loop()
{
  // ************ IMU Sensor
  imu.readCalc();
  snprintf(report, sizeof(report), "A: %6f %6f %6f    G: %6f %6f %6f",
    imu.acc_mps2.x, imu.acc_mps2.y, imu.acc_mps2.z,
    imu.gyro_dps.x, imu.gyro_dps.y, imu.gyro_dps.z);
  Serial.println(report);

  // ************ Pressure Sensor
  float pressure = ps.readPressureMillibars();
  float altitude = ps.pressureToAltitudeMeters(pressure);
  float temperature = ps.readTemperatureC();
  
  Serial.print("p: ");
  Serial.print(pressure);
  Serial.print(" mbar\ta: ");
  Serial.print(altitude);
  Serial.print(" m\tt: ");
  Serial.print(temperature);
  Serial.println(" deg C");

  // ************ Magenetomer
  mag.read();

  snprintf(report, sizeof(report), "M: %6d %6d %6d",
    mag.m.x, mag.m.y, mag.m.z);
  Serial.println(report);

  delay(20);
}
