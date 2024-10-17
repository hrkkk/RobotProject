#include "GlobalVariable.h"

// 地轨信号
int TRACK_WRITE_NUMS = 13;   // 读取地轨PLC寄存器数量  D2202 - D2214
int TRACK_READ_NUMS = 4;          // 读取地轨PLC寄存器数量  D2260 - D2263

int TRACK_POSITION_SET = 2202;    // 设置位置
int TRACK_VOLECITY_SET = 2204;    // 设置速度
int TRACK_FORWARD_SET = 2210;     // 前进
int TRACK_BACKWARD_SET = 2211;    // 后退
int TRACK_ORIGIN_SET = 2212;      // 回原点
int TRACK_MOVE_SET = 2213;        // 启动
int TRACK_RESET_SET = 2214;       // 复位

int TRACK_ORIGIN_GET = 2260;         // 回原点完成
int TRACK_MOVE_GET = 2261;           // 定位完成
int TRACK_ERROR_GET = 2262;          // 发生故障
int TRACK_HEART_GET = 2263;          // 心跳包

// 地轨位置对应表
int ORIGIN_POSITION = 0;
int SHELVES_POSITION = 13350;       // 133.5cm
int BOX_1_POSITION = 17720;         // 172.2cm
int BOX_2_POSITION = 8000;          // 80cm
int BOX_3_POSITION = 1000;          // 10cm

// IO对应表
int IO_READ_NUMS = 24;
int IO_WRITE_NUMS = 12;
int IO_BASE_ADDRESS = 0;

// 1号检测箱
int FIXTURE_5_OUT = 0;   // O1
int FIXTURE_5_IN1 = 0;   // I1
int FIXTURE_5_IN2 = 1;   // I2

int FIXTURE_6_OUT = 1;   // O2
int FIXTURE_6_IN1 = 2;   // I3
int FIXTURE_6_IN2 = 3;   // I4

int DOOR_3_OUT = 2;    // O3
int DOOR_3_IN1 = 4;    // I5
int DOOR_3_IN2 = 5;    // I6

int DOOR_LOCK_3_OUT = 3;       // O4
int DOOR_LOCK_3_IN1 = 6;       // I7
int DOOR_LOCK_3_IN2 = 7;       // I8

// 2号检测箱
int FIXTURE_3_OUT = 4;      // O5
int FIXTURE_3_IN1 = 8;      // I9
int FIXTURE_3_IN2 = 9;      // I10

int FIXTURE_4_OUT = 5;      // O6
int FIXTURE_4_IN1 = 10;     // I11
int FIXTURE_4_IN2 = 11;     // I12

int DOOR_2_OUT = 6;        // O7
int DOOR_2_IN1 = 12;        // I13
int DOOR_2_IN2 = 13;        // I14

int DOOR_LOCK_2_OUT = 7;   // O8
int DOOR_LOCK_2_IN1 = 14;   // I15
int DOOR_LOCK_2_IN2 = 15;   // I16

// 3号检测箱（最左边的）
int FIXTURE_1_OUT = 8;     // O9
int FIXTURE_1_IN1 = 16;     // I17
int FIXTURE_1_IN2 = 17;     // I18

int FIXTURE_2_OUT = 9;     // O10
int FIXTURE_2_IN1 = 18;     // I19
int FIXTURE_2_IN2 = 19;     // I20

int DOOR_1_OUT = 10;        // O11
int DOOR_1_IN1 = 20;        // I21
int DOOR_1_IN2 = 21;        // I22

int DOOR_LOCK_1_OUT = 11;   // O12
int DOOR_LOCK_1_IN1 = 22;   // I23
int DOOR_LOCK_1_IN2 = 23;   // I24

// 机器人信号
int ROBOT_READ_BASE_ADDRESS = 257;
int ROBOT_WRITE_BASE_ADDRESS = 1;

int ROBOT_ORIGIN = 0;   // 33
int TBD2 = 1;   // 34
int TBD3 = 2;   // 35
int TBD4 = 3;   // 36
int TBD5 = 4;   // 37
int UNUSED1 = 5;
int TBD6 = 6;   // 39
int ROBOT_TRACK1 = 7;   // 40
int ROBOT_TRACK2 = 8;   // 41
int ROBOT_TRACK3 = 9;   // 42
int ROBOT_TRACK4 = 10;   // 43
int ROBOT_TRACK5 = 11;   // 44
int UNUSED2 = 12;
int UNUSED3 = 13;
int UNUSED4 = 14;
int UNUSED5 = 15;
int UNUSED6 = 16;

int ZWJQ1 = 17;     // 50
int ZWJQ2 = 18;     // 51
int ZWJQ3 = 19;     // 52
int ZWJQ4 = 20;
int ZWJQ5 = 21;
int ZWJQ6 = 22;
int ZWJQ7 = 23;
int ZWJQ8 = 24;
int ZWJQ9 = 25;
int ZWJQ10 = 26;
int ZWJQ11 = 27;
int ZWJQ12 = 28;
int ZWJF1 = 29;
int ZWJF2 = 30;
int ZWJF3 = 31;
int ZWJF4 = 32;
int ZWJF5 = 33;
int ZWJF6 = 34;
int ZWJF7 = 35;
int ZWJF8 = 36;
int ZWJF9 = 37;
int ZWJF10 = 38;
int ZWJF11 = 39;
int ZWJF12 = 40;
int WKXQ1 = 41;
int WKXQ2 = 42;
int WKXQ3 = 43;
int WKXQ4 = 44;
int WKXQ5 = 45;
int WKXQ6 = 46;
int WKXF1 = 47;
int WKXF2 = 48;
int WKXF3 = 49;
int WKXF4 = 50;
int WKXF5 = 51;
int WKXF6 = 52;
