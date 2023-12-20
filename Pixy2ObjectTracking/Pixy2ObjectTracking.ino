#include <Pixy2.h>
#include <stdio.h>

Pixy2 pixy;

// Speed : 0 ~ 100
// rotationOffset : -255 ~ 255

// Initial Pin
const int motorLPin = 5;
const int motorRPin = 6;

// Pixy Setup
const int CARROT_SIGNATURE = 1; // 假设胡蘿蔔的 Signature 為 1

// Global Setup Speed
const int MAX_SPEED = 255; // 最大速度
const int MIN_SPEED = 0;   // 最小速度

// Global Variable



// Setup Serial, motor and Pixy2
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


// Functions

// Calculate Offset
int getPositionDifference(int xPosition, int middlePosition = 157) {
    int positionDifference = xPosition - middlePosition;
    int mappedDifference = map(positionDifference, -middlePosition, middlePosition, -128, 128);
    
    return mappedDifference;
}


// Motor Control
void motorControl(int speed, int rotationOffset) {
  rotationOffset = map(rotationOffset, -128, 128, -50, 50); //減少 roatationOffset 的影響

  int speedL = speed + rotationOffset;
  int speedR = speed - rotationOffset;

  // 保证速度值在合理范围内（0-100）
  speedL = constrain(speedL, 0, 100);
  speedR = constrain(speedR, 0, 100);

  // 将速度范围从 0-100 映射到 PWM 范围 MIN_SPEED-MAX_SPEED
  speedL = map(speedL, 0, 100, MIN_SPEED, MAX_SPEED);
  speedR = map(speedR, 0, 100, MIN_SPEED, MAX_SPEED);

  // 透過 Function 打印 PWM 值
  printCurrentSpeed(speedL, speedR);
  
  // 寫入速度
  analogWrite(motorLPin, speedL);
  analogWrite(motorRPin, speedR);
}

void motorControlOld(int speedL, int speedR) { // 舊版 自行輸入左右輪速度
  const int maxSpeed = 255;
  const int minSpeed = 0;

  speedL = map(speedL, 0, 100, minSpeed, maxSpeed);
  speedR = map(speedR, 0, 100, minSpeed, maxSpeed);

  analogWrite(motorLPin, speedL);
  analogWrite(motorRPin, speedR);
}


// Pig Mode
void randomWalk() {
    int walkSpeed = 50;
    int totalWalkDuration = random(2000, 5000); // 总行走持续时间（毫秒）
    int decisionInterval = random(500, 1500); // 偏移决策间隔（毫秒）

    unsigned long startTime = millis();
    unsigned long lastDecisionTime = 0;

    while (millis() - startTime < totalWalkDuration) {
        // 每隔一定时间决定是否偏移
        if (millis() - lastDecisionTime > decisionInterval) {
            int rotationOffset = random(-128, 128); // 随机偏移量
            if (random(2) == 0) { // 有一半的概率应用偏移
                rotationOffset = 0;
            }

            // 应用当前的速度和偏移量
            motorControl(walkSpeed, rotationOffset);

            lastDecisionTime = millis();
            decisionInterval = random(500, 1500); // 重新设置下次决策间隔
        }

        delay(50); // 短暂延迟以减少处理
    }

    // 停下来
    motorControl(0, 0);
    int stopDuration = random(500, 3000); // 停止时间
    delay(stopDuration);
}


void followCarrot() {
    int blockCount = pixy.ccc.getBlocks();
    
    if (blockCount > 0) {
        for (int i = 0; i < blockCount; i++) {
            if (pixy.ccc.blocks[i].m_signature == CARROT_SIGNATURE) {
                // 计算中心偏移
                int xOffset = pixy.ccc.blocks[i].m_x - (pixy.frameWidth / 2);
                int yOffset = pixy.ccc.blocks[i].m_y - (pixy.frameHeight / 2);

                // 简单的追踪逻辑：如果胡萝卜足够接近中心，停止移动
                if (abs(xOffset) < 20 && abs(yOffset) < 20) {
                    motorControl(0, 0); // 停止
                } else {
                    motorControl(50, xOffset); // 调整速度和方向
                }
            }
        }
    } else {
        motorControl(0, 0); // 无目标时停止
    }

}

void interactWithHuman(int xPosition) {
    // 检测到人时的逻辑（跟随并注视）
    // 原本打算用 ESP32-CAM 搭配 OpenCV，但發現 Ardunio 上的運算太麻煩，因此改用 Pixy2（不支援辨識人類）
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

void printCurrentSpeed(int speedL, int speedR) { // 列印速度數值
    Serial.print("Left Motor Speed: ");
    Serial.print(speedL);
    Serial.print(" | Right Motor Speed: ");
    Serial.println(speedR);
    
}

void printCurrentSpeedWithSymbol(int speedL, int speedR) { // 用符號列印出速度
    Serial.print("Left Motor Speed: ");

    // 对于左侧马达
    if (speedL > 0) {
        for (int i = 0; i < speedL; i += 10) { // 假设每 10 单位速度一个 '+' 符号
            Serial.print("+");
        }
    } else {
        for (int i = 0; i < -speedL; i += 10) { // 假设每 10 单位速度一个 '-' 符号
            Serial.print("-");
        }
    }

    Serial.print(" | Right Motor Speed: ");

    // 对于右侧马达
    if (speedR > 0) {
        for (int i = 0; i < speedR; i += 10) {
            Serial.print("+");
        }
    } else {
        for (int i = 0; i < -speedR; i += 10) {
            Serial.print("-");
        }
    }

    Serial.println();
}



// Main
void loop() {
  followCarrot();

}