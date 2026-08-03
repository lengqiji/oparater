#pragma once
#include <stdint.h>

enum _client_type_idx {
    eCLIENT_TYPE_IDX_RAYNEN = 0,
    eCLIENT_TYPE_IDX_GAI    = 9,
    eCLIENT_TYPE_IDX_PMK,
    eCLIENT_TYPE_IDX_FORTEVER,
    eCLIENT_TYPE_IDX_JIADAI,
    eCLIENT_TYPE_IDX_ZHAOSHAN,
    eCLIENT_TYPE_IDX_YUEMEI,
    eCLIENT_TYPE_IDX_YONGXIN,
    eCLIENT_TYPE_IDX_MEIFURUI,
    eCLIENT_TYPE_IDX_SINSIM,
    eCLIENT_TYPE_IDX_HONGYI,
    eCLIENT_TYPE_IDX_QIHONG,
    eCLIENT_TYPE_IDX_CHENGYU,
    eCLIENT_TYPE_IDX_HAILIXIN,
    eCLIENT_TYPE_IDX_ZHIBO,
    eCLIENT_TYPE_IDX_BAOSHIJIE,
    eCLIENT_TYPE_IDX_MAYA,
    eCLIENT_TYPE_IDX_XINTIANLONG,
    eCLIENT_TYPE_IDX_HENGJIU,
    eCLIENT_TYPE_IDX_XINGYUE,
    eCLIENT_TYPE_IDX_LEJIA,
    eCLIENT_TYPE_IDX_FUTONG,
    eCLIENT_TYPE_IDX_RES1,
    eCLIENT_TYPE_IDX_HENXIN,
    eCLIENT_TYPE_IDX_YIHANG,
    eCLIENT_TYPE_IDX_RES2,
    eCLIENT_TYPE_IDX_AIJIA,
    eCLIENT_TYPE_IDX_LEYE,
    eCLIENT_TYPE_IDX_GUANGJIONG,
    eCLIENT_TYPE_IDX_HUACHUANG,
    eCLIENT_TYPE_IDX_JINGYU,
    eCLIENT_TYPE_IDX_YUHAO,
    eCLIENT_TYPE_IDX_ERED,
    eCLIENT_TYPE_IDX_MAX
};
// 使用参数获取厂家id 和机械id
// flash_board_param.facId
#define CLIENT_TYPE_BASE          (30000)
#define GET_CLIENT_TYPE_IDX(type) ((type) - CLIENT_TYPE_BASE)

#define CLIENT_TYPE_UNKNOW ((uint32_t) -1)
#define CLIENT_TYPE_RAYNEN (CLIENT_TYPE_BASE)
#define CLIENT_TYPE_ERED   (CLIENT_TYPE_BASE + eCLIENT_TYPE_IDX_ERED)
#define CLIENT_TYPE_QIHONG   (CLIENT_TYPE_BASE + eCLIENT_TYPE_IDX_QIHONG)
#define CLIENT_TYPE_FORTEVER   (CLIENT_TYPE_BASE + eCLIENT_TYPE_IDX_FORTEVER) //合和
#define CLIENT_TYPE_YUEMEI   (CLIENT_TYPE_BASE + eCLIENT_TYPE_IDX_YUEMEI) //越美
#define CLIENT_TYPE_MAX    (CLIENT_TYPE_BASE + eCLIENT_TYPE_IDX_MAX)

// 机头状态类型
typedef enum {
    STATUS_INITIAL,  // 初始化
    STATUS_NORMAL,   // 正常
    STATUS_PATCH,    // 补绣
    STATUS_STOP,     // 停车
    STATUS_OFF,      // 关，关使能或按键下拨都是此状态
    STATUS_TSTBRK,   // 测断线
} status_e;

// 机头类型
typedef enum {
    HEAD_TYPE_NONE,
    HEAD_TYPE_FLAT,          // 平绣
    HEAD_TYPE_TAPE,          // 盘带绣
    HEAD_TYPE_TOWEL,         // 毛巾绣
    HEAD_TYPE_POIL,          // 喷油控制板
    HEAD_TYPE_LASER   = 6,   // 激光
    HEAD_TYPE_TUFTING = 10,  // 植绒绣
} head_type_e;

typedef enum _machType {
    MACH_TYPE_FOOT_ABS_18 = 0x01,  // 独立压脚1.8度，电磁铁跳跃
    MACH_TYPE_FOOT_ABS_SINSIM_HAT_EMB = 0x0A, /* 信胜全独立压脚摆杆式（帽绣） */
    MACH_TYPE_FOOT_COM_ZS = 0x0B,  // 兆山集中式半独立压脚
    MACH_TYPE_FOOT_COM_JD,         // 佳代集中式半独立压脚
} MachType_e;

// 机头参数
typedef struct {
    uint32_t addr;              // 机头地址
    uint32_t set_addr;          // 编址的地址
    uint32_t addr_auto_flag;    // 自动编址标志 =0不 =1是
    uint32_t addr_no_set_flag;  // 是否未设置地址标志
                                // =0不 =1是
    uint32_t addr_ok_flag;  // 编址或寻址成功标志，目前为编址成功编址
    uint32_t addr_ing;  // 编址
    uint32_t addr_cnt;  // 寻址标志保持次数

    uint32_t en;             // 机头使能标志 =0失能 =1失能
    uint32_t set_en;         // 主控设置的使能状态
    uint32_t mainshaft_run;  // 主轴运转标志 =0停止
                             // =1运转
    uint32_t type_err;       // 机头类型错误标志 =0正确
                             // =1不对
    uint32_t iskey_patch;    //=1是按键补绣
                             // =0无 =1设备测试
                             // =2can通信测试
    status_e    stat;        // 机头状态
    head_type_e type;        // 机头类型
    head_type_e addr_type;   // 机头编址的设备类型
    head_type_e cur_type;
    uint32_t    jump_type;  // 跳跃类型
    // 1:打开)
    uint32_t splice_master;  // 拼接机头主头(控制补绣、开关头、跳跃电磁铁等)
    uint32_t is_splice;        // 拼接头标志 =0不是 =1是
    uint32_t splice_head_num;  // 拼接机头数目
    uint32_t manual_trim;  // 手动剪线状态 1：处于手动剪线 0：不处于手动剪线
    uint32_t small_cur_mode; //小电流模式，越美使用
    uint32_t   pressFoot_test;  // 压脚测试
    uint32_t   fac_id;
    uint32_t   jump_no_follow;//初始化之前不跟随，主控找零用的
    uint32_t ndl_num;                            // 机型针数
    MachType_e mach_type;
} head_param_s;

// 绣作参数
typedef struct {
    status_e stat;
    int      bak;
    int      IsBrk;            // 面线断
    int      mainshaft_speed;  // 主轴速度
    int      jump;//补绣的第一针正常头遇到跳针强制跳跃
} emb_param_s;

extern head_param_s Para_head;
extern emb_param_s  Para_emb;
