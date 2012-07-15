/************************************************************************/
/*        运动控制相关数据及宏定义			                            */
/************************************************************************/
#pragma once

//////////////////////////////////////////////////////////////////////////
//文件名称
#define PRJCT_NAME					"MCSysConfig"
#define PRJCT_MAIN_TITLE			"3-Axis Control System"
#define FILENAME_INFO				"%s.info"
#define FILENAME_DATA				"%s.data"
#define FILENAME_CNFG				".\\MCSysConfig.ini"
#define MAXSIZE_CHAR_CNFG			128

//////////////////////////////////////////////////////////////////////////
//轴信息定义
#define AXS_SEV_LEAD				1
#define AXS_SEV_STPNNG				20000//减速后
#define AXS_SEV_SBDVSN				1
#define AXS_SEV_DECRATE				1//虚拟减速比
//#define AXS_SEV_DECRATE				234.77f//实际减速比

#define AXS_STEP_LEAD				5
#define AXS_STEP_STPNNG				200
#define AXS_STEP_SBDVSN				1
#define AXS_STEP_DECRATE			1

//////////////////////////////////////////////////////////////////////////
//读写控制宏
#define DEV_RWSTP_RD				0x01
#define DEV_RWSTP_WT				0x02

#define DEV_SRTYP_SD				0x01
#define DEV_SRTYP_RS				0x02

//////////////////////////////////////////////////////////////////////////
//全局变量
extern HINSTANCE hinsPCommLib;

//////////////////////////////////////////////////////////////////////////
//定时器及其时间设定值
const int TIMER_DEVCM_EVENT =		0x01;//设备信息交换
const int TIMER_VEWUD_EVENT =		0x02;//画面信息刷新

const int TIMER_DEVCM_TIME =		100;//设备信息交换
const int TIMER_VEWUD_TIME =		130;//画面信息刷新

//设备信息交换控制
const int DEVCM_STEP_RD =			0x01;//读设备信息
const int DEVCM_STEP_WT =			0x02;//写控制指令

const int DEVCM_TYPE_SND =			0x01;//发送指令
const int DEVCM_TYPE_RSP =			0x02;//接收指令
