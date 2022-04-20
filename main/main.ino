/*     -- Bluetooth --     */
// Pin 1(RX) & Pin 2(TX) used for Serial -> Bluetooth
const int btStatePin = 4;   // To check if bluetooth is connected or not

/*     -- Sonar --     */
const int trigPin = 7, echoPin = 6;     // For ultrasound sensor
int distance = -1;
bool isDistanceSent = false;

const int loopPeriod = 250;    // a period of 250ms = a frequency of 4Hz || Checks distance in every -> loopPeriod
unsigned long timeLoopDelay = 0;

/*     -- Motor --     */
const int enA = 10, in1 = A3, in2 = A2;      // Motor A connections
const int enB = 11, in3 = A1, in4 = A0;      // Motor B connections
int motorDirection = 0, motorSpeed = 0, motorRunFor = 3;

bool isSpeechDelayDone = false;

String uid = "*1234";   // Unique id for car
bool isSecureConnection = false;

void setup() {
  /*     -- Bluetooth --     */
  pinMode(btStatePin, INPUT);
  
  /*     -- Sonar --     */
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  /*     -- Motor --     */
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  Serial.begin(9600);

  while (!isSecureConnection) {
    if (Serial.available()) {
      String command = Serial.readStringUntil('#');
      if (command == uid) isSecureConnection = true;
    }
  }
}

void loop() {
  if (digitalRead(btStatePin) == LOW) {
    Stop();
  }
  
  isDistanceSent = false;

  if (isSpeechDelayDone) {
    Stop();
    isSpeechDelayDone = false;
  }
  
  if (Serial.available()) {
    String command = Serial.readStringUntil('#');
    
    if (command[0] == '*') {    // Check if pin command
      if (command == uid) isSecureConnection = true;
      else isSecureConnection = false;
    }
    if (!isSecureConnection) goto skip;
    
    if (command.length() == 4) {
      motorDirection = command.substring(0, 1).toInt();
      Stop();
      
      if (motorDirection <= 4) {
        // Joystick / Gesture control
        motorSpeed = command.substring(1, 4).toInt();       // speed (0-100)
        motorSpeed = map(motorSpeed, 0, 100, 70, 255);
        setMotorSpeed(motorSpeed);
        
        switch (motorDirection) {
          case 1:
            forward();
            break;
          case 2:
            right();
            break;
          case 3:
            backward();
            break;
          case 4:
            left();
            break;
        }
        sendDistanceAndWait(10);
        isDistanceSent = true;
      }
      else {
        // Voice control
        motorRunFor = command.substring(1, 4).toInt();      // delay time (second)
        setMotorSpeed(255);
        
        switch (motorDirection) {
          case 5:
            forward();
            break;
          case 6:
            right();
            break;
          case 7:
            backward();
            break;
          case 8:
            left();
            break;
        }
        sendDistanceAndWait(motorRunFor * 1000);
        isDistanceSent = true;

        isSpeechDelayDone = true;
      }
    }
  }
  
  if (!isDistanceSent) sendDistance();
  skip: ;
}

float getDistance() {
  // Returns distance in cm
  /*  Sends out an 8 cycle sonic burst from the transmitter (Trig Pin),
      which then bounces of an object and hits the receiver (Echo Pin)  */
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW); 
  
  float duration = pulseIn(echoPin, HIGH);
  return (duration*.0343)/2;
}

void sendDistance() {
  // Checks distance every 250ms
  if (millis() - timeLoopDelay >= loopPeriod) {
    distance = round(getDistance());
    timeLoopDelay = millis();
  }

  if (distance != -1) {
    int len = String(distance).length();
    if (len < 4) {
      Serial.print("abc"[len - 1]);
      Serial.print(distance);
      Serial.print('#');
    }
  }
}

void sendDistanceAndWait(int ms) {
  unsigned long timeStart = millis();
  while (millis() - timeStart <= ms) {
    sendDistance();
  }
}

void Stop() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void forward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void backward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void left() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void right() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void setMotorSpeed(int value) {
  // PWM(0-255)
  analogWrite(enA, value);
  analogWrite(enB, value);
}
