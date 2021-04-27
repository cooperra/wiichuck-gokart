#include <WiiChuck.h>

Accessory nunchuck1;

const int MAX_MOTOR_POWER = 1023;

void setup() {
   Serial.begin(115200);
   nunchuck1.begin();
   if (nunchuck1.type == Unknown) {
     /** If the device isn't auto-detected, set the type explicitly
     * NUNCHUCK,
     WIICLASSIC,
     GuitarHeroController,
     GuitarHeroWorldTourDrums,
     DrumController,
     DrawsomeTablet,
     Turntable
     */
     nunchuck1.type = NUNCHUCK;
   }
   // LED funtimes
   pinMode(LED_BUILTIN, OUTPUT);
}

int tiltAngleHelper(int rawValue) {
  // Raw value seems to be from left to right:
  // (full left twist) 212-255 (center/upright) 0-45 (full right twist)

  if (rawValue > 45 && rawValue < 212) {
    Serial.print("Tilt value  ");
    Serial.print(String(rawValue));
    Serial.println(" outside of range!");
  }
  
  if (rawValue < 126) {
    // TODO make left and right equal precision
    return map(rawValue, 0, 45, 0, 180); // 46 units, 181 degrees
  } else {
    return map(rawValue, 212, 255, -179, -1); // 44 units, 179 degrees
  }
}

void setMotorPowers(int leftMotorPower, int rightMotorPower) {
  // TODO real hardware pins

  // And now to draw a fun graph
  const int BAR_WIDTH = 30;
  Serial.print("[");
  for (int i = BAR_WIDTH; i > BAR_WIDTH * leftMotorPower / MAX_MOTOR_POWER; i--) {
    Serial.print(" ");
  }
  for (int i = 0; i < BAR_WIDTH * leftMotorPower / MAX_MOTOR_POWER; i++) {
    Serial.print("<");
  }
  Serial.print("|");
  for (int i = 0; i < BAR_WIDTH * rightMotorPower / MAX_MOTOR_POWER; i++) {
    Serial.print(">");
  }
  for (int i = BAR_WIDTH; i > BAR_WIDTH * rightMotorPower / MAX_MOTOR_POWER; i--) {
    Serial.print(" ");
  }
  Serial.print("] ");
  Serial.print(String(leftMotorPower));
  Serial.print(", ");
  Serial.print(String(rightMotorPower));
}

void loop() {
  // Poll nunchuck
  nunchuck1.readData();    // Read inputs and update maps

  // Grab the data we care about
  
  // Motion controls toggle: Z button
  int useMotionControls = nunchuck1.values[10];
  
  // Throttle: Control-Stick Y-axis forward
  int throttleStrength = constrain(map(nunchuck1.values[1], 126, 255, 0, 1023), 0, 1023);

  // Steering
  int steeringAngle;
  if (useMotionControls) {
    // Accelerometer roll tilt
    steeringAngle = tiltAngleHelper(nunchuck1.values[2]);
  } else {
    // ControlStick X-axis
    steeringAngle = map(nunchuck1.values[0], 0, 255, -179, 180);
  }

  // Calculate motor powers
  const int MAX_TILT_ANGLE = 180; // Only require the user to tilt this far for max effect
  int angleStrength = map(abs(steeringAngle), 0, MAX_TILT_ANGLE, 1023, 0);
  // Inside/outside wheel with respect to the turn
  int outsideMotorPower = throttleStrength;
  int insideMotorPower = throttleStrength * angleStrength / 1023;
  
  // Set motor powers
  if (steeringAngle < 0) {
    // Steer left
    setMotorPowers(insideMotorPower, outsideMotorPower);
  } else {
    // Steer right
    setMotorPowers(outsideMotorPower, insideMotorPower);
  }

  // Print steering angle
  Serial.print(", Angle: ");
  Serial.print(String(steeringAngle));

  // Light LED just because
  int brightness = map(throttleStrength, 0, 1023, 1023, 0);
  analogWrite(LED_BUILTIN, brightness);
  
  Serial.println();
}
