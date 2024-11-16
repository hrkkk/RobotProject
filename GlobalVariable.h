#ifndef GLOBALVARIABLE_H
#define GLOBALVARIABLE_H

// 地轨信号
extern int TRACK_WRITE_NUMS;         // 读取地轨PLC寄存器数量  D2202 - D2214
extern int TRACK_READ_NUMS;          // 读取地轨PLC寄存器数量  D2260 - D2263

extern int TRACK_POSITION_SET;    // 设置位置
extern int TRACK_VOLECITY_SET;    // 设置速度
extern int TRACK_FORWARD_SET;     // 前进
extern int TRACK_BACKWARD_SET;    // 后退
extern int TRACK_ORIGIN_SET;      // 回原点
extern int TRACK_MOVE_SET;       // 启动
extern int TRACK_RESET_SET;       // 复位

extern int TRACK_ORIGIN_GET;         // 回原点完成
extern int TRACK_MOVE_GET;           // 定位完成
extern int TRACK_ERROR_GET;          // 发生故障
extern int TRACK_HEART_GET;          // 心跳包

// 地轨位置对应表
extern int ORIGIN_POSITION;
extern int SHELVES_POSITION;
extern int BOX_1_POSITION;
extern int BOX_2_POSITION;
extern int BOX_3_POSITION;

// IO对应表
extern int IO_READ_NUMS;
extern int IO_WRITE_NUMS;
extern int IO_BASE_ADDRESS;

extern int FIXTURE_1_OUT;
extern int FIXTURE_1_IN1;
extern int FIXTURE_1_IN2;

extern int FIXTURE_2_OUT;
extern int FIXTURE_2_IN1;
extern int FIXTURE_2_IN2;

extern int FIXTURE_3_OUT;
extern int FIXTURE_3_IN1;
extern int FIXTURE_3_IN2;

extern int FIXTURE_4_OUT;
extern int FIXTURE_4_IN1;
extern int FIXTURE_4_IN2;

extern int FIXTURE_5_OUT;
extern int FIXTURE_5_IN1;
extern int FIXTURE_5_IN2;

extern int FIXTURE_6_OUT;
extern int FIXTURE_6_IN1;
extern int FIXTURE_6_IN2;

extern int DOOR_1_OUT;
extern int DOOR_1_IN1;
extern int DOOR_1_IN2;

extern int DOOR_2_OUT;
extern int DOOR_2_IN1;
extern int DOOR_2_IN2;

extern int DOOR_3_OUT;
extern int DOOR_3_IN1;
extern int DOOR_3_IN2;

extern int DOOR_LOCK_1_OUT;
extern int DOOR_LOCK_1_IN1;
extern int DOOR_LOCK_1_IN2;

extern int DOOR_LOCK_2_OUT;
extern int DOOR_LOCK_2_IN1;
extern int DOOR_LOCK_2_IN2;

extern int DOOR_LOCK_3_OUT;
extern int DOOR_LOCK_3_IN1;
extern int DOOR_LOCK_3_IN2;

extern int WARNING_LIGHT_OUT;

// 机器人信号
extern int ROBOT_READ_BASE_ADDRESS;
extern int ROBOT_WRITE_BASE_ADDRESS;

extern int ROBOT_ORIGIN;
extern int TBD2;
extern int TBD3;
extern int TBD4;
extern int TBD5;
extern int UNUSED1;
extern int TBD6;
extern int ROBOT_TRACK1;
extern int ROBOT_TRACK2;
extern int ROBOT_TRACK3;
extern int ROBOT_TRACK4;
extern int ROBOT_TRACK5;
extern int UNUSED2;
extern int UNUSED3;
extern int UNUSED4;
extern int UNUSED5;
extern int UNUSED6;

extern int ZWJQ1;
extern int ZWJQ2;
extern int ZWJQ3;
extern int ZWJQ4;
extern int ZWJQ5;
extern int ZWJQ6;
extern int ZWJQ7;
extern int ZWJQ8;
extern int ZWJQ9;
extern int ZWJQ10;
extern int ZWJQ11;
extern int ZWJQ12;
extern int ZWJF1;
extern int ZWJF2;
extern int ZWJF3;
extern int ZWJF4;
extern int ZWJF5;
extern int ZWJF6;
extern int ZWJF7;
extern int ZWJF8;
extern int ZWJF9;
extern int ZWJF10;
extern int ZWJF11;
extern int ZWJF12;
extern int WKXQ1;
extern int WKXQ2;
extern int WKXQ3;
extern int WKXQ4;
extern int WKXQ5;
extern int WKXQ6;
extern int WKXF1;
extern int WKXF2;
extern int WKXF3;
extern int WKXF4;
extern int WKXF5;
extern int WKXF6;

extern int EXTERN_PAUSE;
extern int EXTERN_RUN;
extern int EXTERN_STOP;
extern int EXTERN_MAIN;
extern int EXTERN_RESET;
extern int EXTERN_UPLOAD;
#endif // GLOBALVARIABLE_H
