#include <Pixy2.h>
#include <stdio.h>

Pixy2 pixy;

int mapValue(int x, int in_min, int in_max, int out_min, int out_max) {
    // 確保x不超出輸入範圍
    if (x < in_min) {
        x = in_min;
    } else if (x > in_max) {
        x = in_max;
    }
    // 計算映射的浮點值
    float mapped = (float)(x - in_min) * (float)(out_max - out_min) / (float)(in_max - in_min) + out_min;
    // 將映射的浮點值轉換為整數
    return (int)(mapped + 0.5); // 加0.5用於四捨五入到最接近的整數
}

int getPositionDifference(int xPosition, int middlePosition = 157) {
    int positionDifference = xPosition - middlePosition;
    // 假设屏幕宽度是315，因此最小值是0，最大值是315
    int mappedDifference = mapValue(positionDifference, -middlePosition, middlePosition, -128, 128);
    
    return mappedDifference;
}

void setup() {
  Serial.begin(115200);
  Serial.print("Starting...\n");

  pixy.init();
}

void loop() {
  // 获取并打印检测到的所有物体
  int blockCount = pixy.ccc.getBlocks();

  // 如果至少检测到一个物体
  if (blockCount > 0) {
    // 遍历所有检测到的物体
    for (int i = 0; i < blockCount; i++) {
      // 获取物体的x坐标
      int xPosition = pixy.ccc.blocks[i].m_x;
      //int middlePosition = 157;
      //int positionDifference = xPosition - middlePosition;

      Serial.print("物体");
      Serial.print(i + 1);
      Serial.print(": X轴位置 = ");
      Serial.println(xPosition); 
      //Serial.println(positionDifference);
      Serial.println(getPositionDifference(xPosition));
    }
  }
}
