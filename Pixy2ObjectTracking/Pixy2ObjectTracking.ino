#include <Pixy2.h>

Pixy2 pixy;

void setup()
{
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
      // 获取物体的y坐标
      int yPosition = pixy.ccc.blocks[i].m_y;
      Serial.print("物体");
      Serial.print(i + 1);
      Serial.print(": Y轴位置 = ");
      Serial.println(yPosition);
    }
  }
}
