// ================= SENSOR SETUP =================
int AnalogValue[5] = {0, 0, 0, 0, 0};
int AnalogPin[5]   = {4, 5, 6, 7, 15};

// ================= MOTOR SETUP =================
int motorLeftPWM    = 37;
int motorLeftPhase  = 38;
int motorRightPWM   = 39;
int motorRightPhase = 20;

// ================= CALIBRATION =================
// FOR WHITE LINE: The value is usually SMALLER than the threshold
int threshold = 1500;  // Try lowering this if it jitters on black floor

int baseSpeed  = 180; // Start slower to prevent jitters
int turnSpeed  = 140;

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

  // ===== SENSOR READING =====
  for (int i = 0; i < 5; i++) {
    AnalogValue[i] = analogRead(AnalogPin[i]);
    // WHITE LINE logic: Analog value drops when seeing white
    onLine[i] = (AnalogValue[i] < threshold); 
  }

  // ===== LINE FOLLOWING LOGIC =====

  // 1. ALL ON WHITE (STOP)
  if (onLine[0] && onLine[1] && onLine[2] && onLine[3] && onLine[4]) {
    stopMotors();
    delay(1000);
    // Nudge forward to clear the mark
    driveForward(baseSpeed, baseSpeed);
    delay(200);
  }
  // 2. CENTER SENSOR ONLY (STRAIGHT)
  else if (onLine[2] && !onLine[0] && !onLine[4]) {
    driveForward(baseSpeed, baseSpeed);
  }
  // 3. DRIFTING RIGHT (Sensor 3 or 4 sees white) -> Turn Right
  else if (onLine[3] || onLine[4]) {
    spinRight();
  }
  // 4. DRIFTING LEFT (Sensor 0 or 1 sees white) -> Turn Left
  else if (onLine[0] || onLine[1]) {
    spinLeft();
  }
  // 5. LOST LINE
  else {
    driveForward(100, 100); // Slow crawl to find it
  }
}

// ================= MOTOR CONTROL =================

void driveForward(int left, int right) {
  digitalWrite(motorLeftPhase, HIGH); 
  digitalWrite(motorRightPhase, HIGH);
  analogWrite(motorLeftPWM, left);
  analogWrite(motorRightPWM, right);
}

void spinLeft() {
  digitalWrite(motorLeftPhase, LOW);   // Reverse left
  digitalWrite(motorRightPhase, HIGH);  // Forward right
  analogWrite(motorLeftPWM, turnSpeed);
  analogWrite(motorRightPWM, turnSpeed);
}

void spinRight() {
  digitalWrite(motorLeftPhase, HIGH);  // Forward left
  digitalWrite(motorRightPhase, LOW);   // Reverse right
  analogWrite(motorLeftPWM, turnSpeed);
  analogWrite(motorRightPWM, turnSpeed);
}

void stopMotors() {
  analogWrite(motorLeftPWM, 0);
  analogWrite(motorRightPWM, 0);
}