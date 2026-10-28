// Self-Balancing Robot (ESP32 + BNO08x + ST3020 Servos)
#include <Adafruit_BNO08x.h>
#include <Wire.h>
#include <SCServo.h>

// === IMU Setup ===
Adafruit_BNO08x bno08x;
sh2_SensorValue_t sensorValue;
#define REPORT_INTERVAL 5000 // 200 Hz

// === Servo Setup ===
HardwareSerial servoSerial(2); // UART2
SMS_STS st;
#define SERVO_TX_PIN 17
#define SERVO_RX_PIN 16
const uint8_t servo1 = 1;
const uint8_t servo2 = 2;

// === PID Control ===
float kp = 58000, ki = 1.2, kd = 500;
float error = 0, prev_error = 0, integral = 0;
float targetAngle = 0.0;
unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  
  // BNO08x IMU Init
  if (!bno08x.begin_I2C()) {
    Serial.println("IMU not detected!");
    while (1);
  }
  bno08x.enableReport(SH2_ROTATION_VECTOR, REPORT_INTERVAL);

  // Servo Init
  servoSerial.begin(1000000, SERIAL_8N1, SERVO_RX_PIN, SERVO_TX_PIN);
  st.pSerial = &servoSerial;
  delay(200);
  st.WheelMode(servo1);
  st.WheelMode(servo2);
  delay(100);
  Serial.println("Setup complete");
  lastTime = millis();
}

void readSerialPID() {
  static String input = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      if (input.startsWith("PID:")) {
        input.remove(0, 4);
        int i1 = input.indexOf(',');
        int i2 = input.indexOf(',', i1 + 1);
        kp = input.substring(0, i1).toFloat();
        ki = input.substring(i1 + 1, i2).toFloat();
        kd = input.substring(i2 + 1).toFloat();
        Serial.print("Updated PID: ");
        Serial.print(kp); Serial.print(", ");
        Serial.print(ki); Serial.print(", ");
        Serial.println(kd);
      }
      input = "";
    } else {
      input += c;
    }
  }
}

void loop() {
  readSerialPID();
  if (bno08x.getSensorEvent(&sensorValue)) {
    if (sensorValue.sensorId == SH2_ROTATION_VECTOR) {
      float qw = sensorValue.un.rotationVector.real;
      float qx = sensorValue.un.rotationVector.i;
      float qy = sensorValue.un.rotationVector.j;
      float qz = sensorValue.un.rotationVector.k;
      float pitch = asin(2.0 * (qw * qy - qz * qx))-0.15;

      if (pitch>0.5235987755983 || pitch<-0.5235987755983){
        Serial.print("Pitch: "); Serial.print(pitch);
        st.WriteSpe(servo1, 0);
        st.WriteSpe(servo2, 0);
      }
      else{
        unsigned long now = millis();
        float dt = (now - lastTime) / 1000.0;
        lastTime = now;

        // PID
        error = pitch - targetAngle;
        integral += error * dt;
        integral = constrain(integral, -50, 50);  // Anti-windup

        float derivative = (error - prev_error) / dt;
        float output = kp * error + ki * integral + kd * derivative;
        prev_error = error;

        // Constrain output and send to motors
        int pwm = output;
        pwm = constrain(pwm, -10000, 10000);
        st.WriteSpe(servo1, -pwm);
        st.WriteSpe(servo2, pwm);
        Serial.print(pwm);
      

        // Debug
        Serial.print("Pitch: "); Serial.print(pitch);
        Serial.print("\tOutput: "); Serial.println(output);
      }
    }
  }
  delay(2);
}