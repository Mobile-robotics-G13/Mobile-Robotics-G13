// ================= SENSOR SETUP =================
int AnalogValue[5] = {0, 0, 0, 0, 0};
int AnalogPin[5]   = {4, 5, 6, 7, 15};

// ================= MOTOR SETUP =================
int motorLeftPWM    = 37;
int motorLeftPhase  = 38;
int motorRightPWM   = 39;
int motorRightPhase = 20;

// ================= CALIBRATION =================
int threshold = 1500;  

int maxSpeed    = 255;  
int cruiseSpeed = 210;  
int turnSpeed   = 170;  

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  pinMode(motorLeftPWM, OUTPUT);
  pinMode(motorLeftPhase, OUTPUT);
  pinMode(motorRightPWM, OUTPUT);
  pinMode(motorRightPhase, OUTPUT);
  stopMotors();
}

void loop() {
  bool onLine[5];
  int sensorsActive = 0;

  // ===== SENSOR READING =====
  for (int i = 0; i < 5; i++) {
    AnalogValue[i] = analogRead(AnalogPin[i]);
    onLine[i] = (AnalogValue[i] < threshold);
    if (onLine[i]) sensorsActive++; // Count how many sensors see the line
  }

  // ===== IMPROVED INTERSECTION LOGIC =====
  
  // If 3 or more sensors see white, it's almost certainly an intersection
  // even if the robot is slightly tilted or coming off a corner.
  if (sensorsActive >= 3) {
    handleIntersection();
  }
  
  // ===== HIGH SPEED LINE FOLLOWING =====
  
  // Straight - Only center is on line
  else if (onLine[2] && !onLine[0] && !onLine[4]) {
    moveForward(maxSpeed, maxSpeed); 
  }

  // Slight Left (Sensor 1)
  else if (onLine[1]) {
    moveForward(150, cruiseSpeed); 
  }

  // Slight Right (Sensor 3)
  else if (onLine[3]) {
    moveForward(cruiseSpeed, 150);
  }

  // Hard Left (Sensor 0)
  else if (onLine[0]) {
    spinLeft(turnSpeed);
  }

  // Hard Right (Sensor 4)
  else if (onLine[4]) {
    spinRight(turnSpeed);
  }

  // Lost the line
  else {
    moveForward(200, 200); 
  }
}

// ================= HELPER FUNCTIONS =================

void handleIntersection() {
  stopMotors();
  Serial.println("Intersection Detected!");
  delay(1000);
  
  // CRITICAL: Drive forward enough to clear the wide line 
  // so the sensors don't immediately trigger another stop.
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
