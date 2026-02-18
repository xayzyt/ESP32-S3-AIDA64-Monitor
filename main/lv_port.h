#ifndef __LV_PORT_H // 防止头文件被重复引用（Header Guard），标准写法
#define __LV_PORT_H

// 声明初始化函数，让 main.c 能找到它
void lv_port_init(void);

#endif
