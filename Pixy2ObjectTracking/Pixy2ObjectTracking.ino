#include <Pixy2.h>
#include <stdio.h>

Pixy2 pixy;

// Initial Ports
const int motorLPin = 5;
const int motorRPin = 6;

// Setup Speed
const int MAX_SPEED = 255; // 最大速度值
const int MIN_SPEED = 0;   // 最小速度值


// Setup motor and Pixy2
void setup() {
  // Setup Serial Port
  Serial.begin(115200);

  // Pixy2
  Serial.print("Starting...\n");
  pixy.init();

  // Motor
  pinMode(motorLPin, OUTPUT);
  pinMode(motorRPin, OUTPUT);
}


// Pixy2 Tracking
int getPositionDifference(int xPosition, int middlePosition = 157) {
    int positionDifference = xPosition - middlePosition;
    int mappedDifference = map(positionDifference, -middlePosition, middlePosition, -128, 128);
    
    return mappedDifference;
}


// Motor Control
void motorControl(int speed, int rotationOffset) {
    int speedL = speed + rotationOffset;
    int speedR = speed - rotationOffset;

    // 保证速度值在合理范围内
    speedL = constrain(speedL, -128, 128);
    speedR = constrain(speedR, -128, 128);

    // 将速度范围从 -128-128 映射到 PWM 范围 MIN_SPEED-MAX_SPEED
    speedL = map(speedL, -128, 128, MIN_SPEED, MAX_SPEED);
    speedR = map(speedR, -128, 128, MIN_SPEED, MAX_SPEED);

    analogWrite(motorLPin, speedL);
    analogWrite(motorRPin, speedR);
}

void motorControlOld(int speedL, int speedR) {
  const int maxSpeed = 255;
  const int minSpeed = 0;

  speedL = map(speedL, 0, 100, minSpeed, maxSpeed);
  speedR = map(speedR, 0, 100, minSpeed, maxSpeed);

  analogWrite(motorLPin, speedL);
  analogWrite(motorRPin, speedR);
}


// Pig Mode
void idleMode() {
    // 闲置模式的逻辑（随机旋转和行走）
}

void followCarrot(int xPosition) {
    // 跟随胡萝卜的逻辑
}

void interactWithHuman(int xPosition) {
    // 检测到人时的逻辑（随机跟随或旋转并注视）
}



// Test Unit
void testPrintPositionDiff() {
  // 获取并打印检测到的所有物体
  int blockCount = pixy.ccc.getBlocks();

  // 如果至少检测到一个物体
  if (blockCount > 0) {
    // 遍历所有检测到的物体
    for (int i = 0; i < blockCount; i++) {
      int xPosition = pixy.ccc.blocks[i].m_x;

      Serial.print("物體");
      Serial.print(i + 1);
      Serial.print(": X Position = ");
      Serial.println(xPosition); 

      Serial.println(getPositionDifference(xPosition));
    }
  }
}


// Main
void loop() {


}