#include <Pixy2.h>
#include <stdio.h>

#define CW  1   // 定義順時針轉動
#define CCW 2   // 定義逆時針轉動

Pixy2 pixy;

// Speed : 0 ~ 100
// rotationOffset : -255 ~ 255

// 定義Arduino引腳到MonsterMoto Shield的連接
int inApin[2] = {7, 4};  // INA: 順時針輸入
int inBpin[2] = {8, 9};  // INB: 逆時針輸入
int pwmpin[2] = {5, 6};  // PWM輸入

// Pixy Setup
const int CARROT_SIGNATURE = 1; // 假设胡蘿蔔的 Signature 為 1

// Global Setup Speed
const int MAX_SPEED = 40; // 最大速度
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
  for (int i = 0; i < 2; i++) {
    pinMode(inApin[i], OUTPUT);
    pinMode(inBpin[i], OUTPUT);
    pinMode(pwmpin[i], OUTPUT);
  }

  // 初始化馬達為制動狀態
  for (int i = 0; i < 2; i++) {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
  }
}


// Functions

// Calculate Offset
int getPositionDifference(int xPosition, int middlePosition = 157) {
    int positionDifference = xPosition - middlePosition;
    int mappedDifference = map(positionDifference, -middlePosition, middlePosition, -128, 128);
    
    return mappedDifference;
}


// Motor Control
void motorControl(int targetSpeed, int rotationOffset) {
  // 将旋转偏移映射到较小的范围
  rotationOffset = map(rotationOffset, -128, 128, -50, 50); 

  // 软启动
  int startSpeed = 180; // 假设启动速度为目标速度的150%，但不超过100
  applyMotorSpeed(startSpeed, rotationOffset);
  delay(200); // 短暂的高速运行以启动马达

  // 减速到目标速度
  applyMotorSpeed(targetSpeed, rotationOffset);
}

void applyMotorSpeed(int speed, int rotationOffset) {
  int speedL = speed + rotationOffset;
  int speedR = speed - rotationOffset;

  // 保证速度值在合理范围内
  speedL = constrain(speedL, 0, 100);
  speedR = constrain(speedR, 0, 100);

  // 映射到 PWM 范围
  speedL = map(speedL, 0, 100, MIN_SPEED, MAX_SPEED);
  speedR = map(speedR, 0, 100, MIN_SPEED, MAX_SPEED);

  // 打印 PWM 值
  // printCurrentSpeed(speedL, speedR);
  
  // 控制马达
  motorGo(0, CW, abs(speedL));
  motorGo(1, CCW, abs(speedR));
}

// Monster Moto Shield

void motorOff(int motor) {
  digitalWrite(inApin[motor], LOW);
  digitalWrite(inBpin[motor], LOW);
  analogWrite(pwmpin[motor], 0);
}

void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm) {
  if (motor <= 1) {
    if (direct <= 2) {
      digitalWrite(inApin[motor], direct == CW);
      digitalWrite(inBpin[motor], direct == CCW);
      analogWrite(pwmpin[motor], pwm);
    }
  }
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
    
    if (blockCount > 0) {
        for (int i = 0; i < blockCount; i++) {
            if (pixy.ccc.blocks[i].m_signature == CARROT_SIGNATURE-1) {
                // 计算中心偏移
                int xOffset = pixy.ccc.blocks[i].m_x - (pixy.frameWidth / 2);
                int yOffset = pixy.ccc.blocks[i].m_y - (pixy.frameHeight / 2);

                // 简单的追踪逻辑：如果胡萝卜足够接近中心，停止移动
                if (abs(xOffset) < 20 && abs(yOffset) < 20) {
                    motorControl(0, 0); // 停止
                } else {
                    motorControl(MAX_SPEED, xOffset); // 调整速度和方向
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

}