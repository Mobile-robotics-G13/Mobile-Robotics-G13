// Pin Definitions
const int trigPin = 48;
const int echoPin = 45;

// Variables for math
long duration;
float distanceCm;

int motorleftspeed = 37;
int motorleft = 38;
int motorrightspeed = 39;
int motorright = 20;

void setup() {
  Serial.begin(115200);
  
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);

  // IMPORTANT: Other code likely works because it explicitly sets these as OUTPUT
  pinMode(motorleft, OUTPUT);
  pinMode(motorright, OUTPUT);
  pinMode(motorleftspeed, OUTPUT);
  pinMode(motorrightspeed, OUTPUT);

  motorStop(); // Start stopped
}

void loop() {
  // 1. Get Distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * 0.034 / 2;

  // 2. Logic: The "If/Else" prevents the motor from jittering
  // If no object is detected (0) or distance is clear (> 6cm)
  if (distanceCm > 7.5 || duration == 0) {
    moveForward();
  } 
  else {
    motorStop();
  }

  // 3. Monitor
  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  delay(60); // Faster sampling (15Hz approx) for better reaction
}

void moveForward() {
  // If the motor turns the wrong way, software-flip the logic here:
  // Change HIGH to LOW or vice-versa
  digitalWrite(motorleft, HIGH); 
  digitalWrite(motorright, HIGH);
  analogWrite(motorleftspeed, 255);
  analogWrite(motorrightspeed, 237);
}

void motorStop() {
  // Force both speed AND direction pins to neutral
  analogWrite(motorleftspeed, 0);
  analogWrite(motorrightspeed, 0);
  digitalWrite(motorleft, LOW);
  digitalWrite(motorright, LOW);
}