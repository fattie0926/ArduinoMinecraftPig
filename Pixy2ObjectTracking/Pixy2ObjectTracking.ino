#include <Pixy2.h>
#include <stdio.h>
#include <PCM.h>

Pixy2 pixy;

// Speed : 0 ~ 100
// rotationOffset : -255 ~ 255

// Pixy Setup
const int CARROT_SIGNATURE = 1; // 胡蘿蔔的 Signature 為 1

// Setup DC Gear Motor Pin
int pwmPin[2] = {5, 6};  // PWM速度控制

// Global Variable

// Speed Setup
const int MAX_SPEED = 255; // 最大速度
const int MIN_SPEED = 0;   // 最小速度

// Size
const int SIZE_THRESHOLD = 100; // 这个值可以根据您的需要进行调整

// Sliding Window Average
const int numReadings = 3;  // 平滑化窗口的大小
int readings[numReadings];   // 存储读数的数组
int readIndex = 0;           // 当前读数的索引
int total = 0;               // 读数的总和
int average = 0;             // 平均读数

// Pig Sound

// Setup Serial, motor and Pixy2
void setup() {
  // Setup Serial Port
  Serial.begin(115200);

  // Pixy2
  Serial.print("Starting...\n");
  pixy.init();

  // Motor
  for (int i = 0; i < 2; i++) {
      pinMode(pwmPin[i], OUTPUT);
  }

  // Sliding Window Average
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}


// Functions

// Sliding Window Average
int updateAverage(int newValue) {
  // 从总和中减去最早的读数
  total = total - readings[readIndex];
  // 读取新的值
  readings[readIndex] = newValue;
  // 将新值加到总和中
  total = total + readings[readIndex];
  // 移动到下一个位置
  readIndex = readIndex + 1;

  // 如果我们到达数组的末尾，则回到开始
  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  // 计算平均值
  average = total / numReadings;
  return average;
}

// Calculate Offset
int getPositionDifference(int xPosition, int middlePosition = 157) {
    int positionDifference = xPosition - middlePosition;
    int mappedDifference = map(positionDifference, -middlePosition, middlePosition, -128, 128);
    
    return mappedDifference;
}

void motorControl(int targetSpeed, int rotationOffset) {
  // 将旋转偏移映射到较小的范围
  int width = (pixy.frameWidth / 2);
  rotationOffset = map(rotationOffset, -width, width, -10, 10);

  // 计算左右电机的速度
  int speedL = targetSpeed + rotationOffset;
  int speedR = targetSpeed - rotationOffset;

  // 映射到 PWM 范围
  speedL = map(speedL, 0, 100, MIN_SPEED, MAX_SPEED);
  speedR = map(speedR, 0, 100, MIN_SPEED, MAX_SPEED);

  printCurrentSpeed(speedL, speedR);

  // 为电机设置PWM
  analogWrite(pwmPin[0], speedL);
  analogWrite(pwmPin[1], speedR);
}


// Pig Mode
void randomWalk() {
    int walkSpeed = MAX_SPEED;
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
  bool isTargetDetected = false;

  if (blockCount > 0) {
    for (int i = 0; i < blockCount; i++) {
      if (pixy.ccc.blocks[i].m_signature == CARROT_SIGNATURE) {
        int xPosition = pixy.ccc.blocks[i].m_x;
        int objectWidth = pixy.ccc.blocks[i].m_width;
        int objectHeight = pixy.ccc.blocks[i].m_height;

        Serial.print("物體");
        Serial.print(i + 1);
        Serial.print(": X Position = ");
        Serial.println(xPosition);
        Serial.print("Width: ");
        Serial.print(objectWidth);
        Serial.print(", Height: ");
        Serial.println(objectHeight);

        // 检查物体大小是否超过阈值
        if (objectWidth > SIZE_THRESHOLD || objectHeight > SIZE_THRESHOLD) {
          motorControl(0, 0); // 停止电机
        } else {
          int newPosition = getPositionDifference(xPosition);
          int smoothPosition = updateAverage(newPosition);
          motorControl(100, smoothPosition); // 控制电机
        }

        isTargetDetected = true;
      }
    }
  }

  if (!isTargetDetected) {
    motorControl(0, 0); // 如果没有检测到目标物体，停止电机
  }
}

void followCarrotOld() {
  // 获取并打印检测到的所有物体
  int blockCount = pixy.ccc.getBlocks();

  // 是否检测到了指定签名的物体
  bool isTargetDetected = false;

  // 如果至少检测到一个物体
  if (blockCount > 0) {
    // 遍历所有检测到的物体
    for (int i = 0; i < blockCount; i++) {
      if (pixy.ccc.blocks[i].m_signature == CARROT_SIGNATURE) {
        int xPosition = pixy.ccc.blocks[i].m_x;

        Serial.print("物體");
        Serial.print(i + 1);
        Serial.print(": X Position = ");
        Serial.println(xPosition); 

        Serial.println(getPositionDifference(xPosition));

        int newPosition = getPositionDifference(xPosition);
        int smoothPosition = updateAverage(newPosition);
        motorControl(100, smoothPosition);
        isTargetDetected = true;
      }
    }
  }

  // 如果没有检测到目标物体
  if (!isTargetDetected) {
    motorControl(0, 0); // 停止电机
  }
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
      motorControl(100, getPositionDifference(xPosition));
    }
  }
}

void printCurrentSpeed(int speedL, int speedR) { // 列印速度數值
    Serial.print("Left: ");
    Serial.print(speedL);
    Serial.print(" | Right: ");
    Serial.println(speedR);
    
}


// Main
void loop() {
  followCarrot();
}