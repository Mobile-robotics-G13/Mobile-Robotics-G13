// ================= IR DISTANCE SENSOR SETUP =================
const int irSensorPin = 16;  // IR sensor pin (ADC1 channel)
const float vRef = 3.3;      // ESP32 ADC reference voltage
const int adcRes = 4095;     // ESP32 12-bit resolution

// ================= LINE SENSOR SETUP =================
int AnalogValue[5] = {0, 0, 0, 0, 0};
int AnalogPin[5]   = {4, 5, 6, 7, 15};
int threshold = 1500;

// ================= NAVIGATION SETUP =================
float stopDistance = 7;   // Distance in cm to trigger 180 turn
unsigned long lastTurnTime = 0;
int turnCooldown = 200;    // Ignore obstacles for 3 seconds after turning

// ================= MOTOR SETUP =================
int motorLeftPWM    = 37;
int motorLeftPhase  = 38;
int motorRightPWM   = 39;
int motorRightPhase = 20;

int maxSpeed    = 255;
int cruiseSpeed = 220;
int turnSpeed   = 20;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // Set 12-bit ADC resolution
  analogSetAttenuation(ADC_11db);  // Set ADC attenuation for full 0-3.3V range
  
  pinMode(motorLeftPWM, OUTPUT);
  pinMode(motorLeftPhase, OUTPUT);
  pinMode(motorRightPWM, OUTPUT);
  pinMode(motorRightPhase, OUTPUT);
  
  stopMotors();
}

void loop() {
  // ===== CHECK IR SENSOR FIRST =====
  float distance = getIRDistance();
  
  // Debug output for IR sensor
  Serial.print("IR Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  // Immediate 180 turn when obstacle detected (no consecutive count needed)
  if (distance < stopDistance && (millis() - lastTurnTime > turnCooldown)) {
    Serial.print("OBSTACLE at ");
    Serial.print(distance);
    Serial.println(" cm - Turning around!");
    
    // Perform 180 turn
    perform180Turn();
    
    // Set cooldown timer
    lastTurnTime = millis();
    
    return;
  }
  
  // ===== LINE FOLLOWING LOGIC =====
  bool onLine[5];
  int sensorsActive = 0;

  // Read all line sensors
  for (int i = 0; i < 5; i++) {
    AnalogValue[i] = analogRead(AnalogPin[i]);
    onLine[i] = (AnalogValue[i] < threshold);
    if (onLine[i]) sensorsActive++;
  }

  // Intersection detection (3 or more sensors see the line)
  if (sensorsActive >= 3) {
    handleIntersection();
  }
  
  // Straight - Only center is on line
  else if (onLine[2] && !onLine[0] && !onLine[4]) {
    moveForward(maxSpeed, maxSpeed);
  }

  // Slight Left (Sensor 1)
  else if (onLine[1]) {
    moveForward(120, cruiseSpeed);
  }

  // Slight Right (Sensor 3)
  else if (onLine[3]) {
    moveForward(cruiseSpeed, 120);
  }

  // Hard Left (Sensor 0)
  else if (onLine[0]) {
    moveForward(turnSpeed,180);
  }

  // Hard Right (Sensor 4)
  else if (onLine[4]) {
    moveForward(180,turnSpeed);
  }

  // Lost the line - move forward slowly
  else {
    moveForward(cruiseSpeed, cruiseSpeed);
  }
}

// ================= HELPER FUNCTIONS =================

float getIRDistance() {
  // Average multiple readings to reduce noise
  float rawSum = 0;
  for(int i = 0; i < 10; i++) {  // Reduced from 15 to speed up
    rawSum += analogRead(irSensorPin);
    delay(1);
  }
  float averageADC = rawSum / 10.0;
  
  // Convert ADC value to voltage
  float voltage = (averageADC * vRef) / adcRes;
  
  // Handle very low voltage readings (no object detected)
  if (voltage < 0.4) return 999.0;
  
  // Calculate distance using power formula for GP2Y0A41SK (4cm-30cm range)
  float distanceCM = 13.0 * pow(voltage, -1.10);
  
  // Constrain to sensor's valid range
  if (distanceCM > 30) distanceCM = 999.0;
  if (distanceCM < 4) distanceCM = 4.0;
  
  return distanceCM;
}

void perform180Turn() {
  // Stop first
  stopMotors();
  delay(300);
  
  // Back up slightly
  motorReverse(150);
  delay(300);
  
  // Turn enough to get OFF the line first
  spinRight(200);
  delay(700);
  
  // Keep turning until we find the line again (but with timeout)
  int sensorsActive = 0;
  int turnAttempts = 0;
  int maxAttempts = 40;  // Prevent infinite loop
  
  while (sensorsActive < 1 && turnAttempts < maxAttempts) {
    spinRight(200);
    
    // Check sensors
    sensorsActive = 0;
    for (int i = 0; i < 5; i++) {
      if (analogRead(AnalogPin[i]) < threshold) sensorsActive++;
    }
    
    turnAttempts++;
    delay(50);  // Longer delay between checks
  }
  
  Serial.print("Turn complete. Attempts: ");
  Serial.println(turnAttempts);
  
  // Stop after finding line
  stopMotors();
  delay(300);
  
  // Move forward a bit to stabilize
  moveForward(200, 200);
  delay(100);
}

void handleIntersection() {
 // stopMotors();
  Serial.println("Intersection Detected!");
  //delay(500);
  
  // Drive forward to clear the wide line
  unsigned long clearTime = millis();
  while(millis() - clearTime < 250) {
   moveForward(200, 200);
  }
}

void moveForward(int left, int right) {
  digitalWrite(motorLeftPhase, HIGH);
  digitalWrite(motorRightPhase, HIGH);
  analogWrite(motorLeftPWM, left);
  analogWrite(motorRightPWM, right);
}

void spinLeft(int speed) {
  digitalWrite(motorLeftPhase, LOW);
  digitalWrite(motorRightPhase, HIGH);
  analogWrite(motorLeftPWM, speed);
  analogWrite(motorRightPWM, speed);
}

void spinRight(int speed) {
  digitalWrite(motorLeftPhase, HIGH);
  digitalWrite(motorRightPhase, LOW);
  analogWrite(motorLeftPWM, speed);
  analogWrite(motorRightPWM, speed);
}

void stopMotors() {
  analogWrite(motorLeftPWM, 0);
  analogWrite(motorRightPWM, 0);
}

void motorReverse(int speed) {
  digitalWrite(motorLeftPhase, LOW);
  digitalWrite(motorRightPhase, LOW);
  analogWrite(motorLeftPWM, speed);
  analogWrite(motorRightPWM, speed);
}
