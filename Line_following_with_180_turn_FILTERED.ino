// ================= SENSOR SETUP =================
int AnalogValue[5] = {0, 0, 0, 0, 0};
int AnalogPin[5]   = {4, 5, 6, 7, 15};

// ================= ULTRASONIC SENSOR SETUP =================
int trigPin = 48;
int echoPin = 45;
int stopDistance = 7; // Distance in cm to stop (adjust as needed)
int consecutiveDetections = 0;
int requiredDetections = 2; // Must detect obstacle 3 times in a row before turning

// ================= MOTOR SETUP =================
int motorLeftPWM    = 37;
int motorLeftPhase  = 38;
int motorRightPWM   = 39;
int motorRightPhase = 20;

// ================= CALIBRATION =================
int threshold = 1500;  

int maxSpeed    = 220;  
int cruiseSpeed = 180;  
int turnSpeed   = 140;  

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  pinMode(motorLeftPWM, OUTPUT);
  pinMode(motorLeftPhase, OUTPUT);
  pinMode(motorRightPWM, OUTPUT);
  pinMode(motorRightPhase, OUTPUT);
  
  // Ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  stopMotors();
}

void loop() {
  // ===== CHECK ULTRASONIC SENSOR FIRST =====
  long distance = getDistance();
  
  // Check if obstacle is detected
// Check if obstacle is detected 
if (distance < stopDistance && distance > 0) { 
  consecutiveDetections++; 
  Serial.print("Detection #"); 
  Serial.print(consecutiveDetections); 
  Serial.print(" at "); 
  Serial.print(distance); 
  Serial.println(" cm"); 
  
  // Only turn around if we've detected it multiple times in a row 
  if (consecutiveDetections >= requiredDetections) { 
    Serial.println("CONFIRMED OBSTACLE - SHOULD BE TURNING!");  // ADD THIS
    
    // Stop first 
    stopMotors(); 
    delay(300); 
    
    spinRight(200);
    delay(200);
    Serial.println("Motors stopped, starting turn...");  // ADD THIS
    
    bool onLine[5];
    int sensorsActive = 0;
    int turnAttempts = 0;
    
    // Keep turning in increments until we find the line
    while (sensorsActive < 1 && turnAttempts < 40) {
      Serial.print("Turn attempt: ");  // ADD THIS
      Serial.println(turnAttempts);     // ADD THIS
      
      spinRight(200);
      delay(50);
      
      // Read sensors
      sensorsActive = 0;
      for (int i = 0; i < 5; i++) {
        AnalogValue[i] = analogRead(AnalogPin[i]);
        if (AnalogValue[i] < threshold) sensorsActive++;
      }
      
      Serial.print("Sensors active: ");  // ADD THIS
      Serial.println(sensorsActive);      // ADD THIS
      
      turnAttempts++;
    }
    
    Serial.println("Turn complete!");  // ADD THIS
    
    // Stop after turning
    stopMotors();
    delay(300);
    
    // Move forward a bit to get back on the line
    moveForward(200, 200);
    delay(50);
    
    // Reset detection counter
    consecutiveDetections = 0;
    
    return; // Exit loop early, then resume line following
  } 
}
  
  // ===== ORIGINAL LINE FOLLOWING CODE BELOW =====
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
    moveForward(250, 250); 
  }
}

// ================= HELPER FUNCTIONS =================

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000);
  
  if (duration == 0) {
    return 999; // No echo = no obstacle
  }
  
  long distance = (duration / 2) * 0.0343;
  
  if (distance < 2 || distance > 400) {
    return 999; // Invalid reading
  }
  
  return distance;
}

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
