
//***************************************************************** 
// 这里书写的是运动控制系统头文件，重用时只需要拷贝下列文档即可
// "MCSystem.h"
// "PmacRuntime.h"
// "PmacRuntime.c"
// "MCSysExe.C"
// 为了支持windows变量类别及函数，请在CVi工程中包含window头文件
// #include <windows.h>
// Release 2011-05-19 by mjq_clint(马腱杞 mjq_clint@hotmail.com)
// Release 2011-05-23 新增设备状态获取方法及相关（详查20110523）
//***************************************************************** 
/************************************************************************/
/*                                                                      */
/************************************************************************/
#pragma once

//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <windows.h>

//////////////////////////////////////////////////////////////////////////
//
#include "MCAxisBase.h"

//////////////////////////////////////////////////////////////////////////
//#ifndef _PCOMM_AXS_
//#define _PCOMM_AXS_

//***************************************************************** 
// COMM Type Defines 
//***************************************************************** 
#define PMAC_RES_ERR			"err"
#define PMAC_CMD_CHMAX			256
#define MCSYS_DEV_AXS_MAX		8
//定义电机选择宏
#define DEV_AXS_MAX				0x08//设备最大轴数
#define DEV_AXS_SEL_ALL			0xFF
#define DEV_AXS_SEL_01			0x01
#define DEV_AXS_SEL_02			0x02
#define DEV_AXS_SEL_03			0x04
#define DEV_AXS_SEL_04			0x08
#define DEV_AXS_SEL_05			0x10
#define DEV_AXS_SEL_06			0x20
#define DEV_AXS_SEL_07			0x40
#define DEV_AXS_SEL_08			0x80

//定义错误代码
#define ERRO_NOERR				0x0000
#define ERRO_INIT_LINK			0x0001
#define ERRO_INIT_OPEN			0x0002
#define ERRO_RELEASE_CLOSE		0x0011
#define ERRO_RELEASE_DLINK		0x0012
#define ERRO_NORMAL_CMD			0x0100

//////////////////////////////////////////////////////////////////////////
//定义控制设备结构
typedef struct CMCDevice {
	int m_nRWStep;//读写类别
	int m_nSRType;//发送接收类别
	DWORD m_dwID;//设备ID
	DWORD m_nDevStt;//设备状态
	DWORD m_dwAxisSel;//轴选择
	CMCAXS m_Axis[8];//轴定义
} CMCDEV;

//***************************************************************** 
// Function Define
//***************************************************************** 
//检查是否增加空格
#define CMD_SEND_CHECK(chChk,chAdd)  if (strlen(chChk)) \
                                 strcat(chChk,chAdd)

//设备轴动态初始化函数
//CMCAXS MCAxisInit(DWORD dwAxisID);
//***************************************************************** 
// 用户接口函数，在使用MCDevCmdSend发送指令前请务必设置
// 设备各轴选择情况及指令类别
//***************************************************************** 
//上位操作函数
int MCSysInit(CMCDEV * pCMCDev);
int MCSysRelease(CMCDEV * pCMCDev);

//命令及相应相关函数
int MCDevCmdSend(CMCDEV * pCMCDev,char *chRes,int ichLen,char *chSend);

//命令响应解析函数
int MCDevCmdResDataGet(CMCDEV * pCMCDev,char *chRes,int ichLen);
int MCDevCmdResDataGetPst(CMCAXS * pCMCAxs,char *chRes,int iDataset,int ichLen);
int MCDevCmdResDataGetVlc(CMCAXS * pCMCAxs,char *chRes,int iDataset,int ichLen);
int MCDevCmdResDataGetStt(CMCAXS * pCMCAxs,char *chRes,int iDataset,int ichLen);

//***************************************************************** 
// 功能控制函数，供以上三个函数调用
//***************************************************************** 
//////////////////////////////////////////////////////////////////////////
//设备控制函数
int MCDevInit(CMCDEV * pCMCDev);
int MCDevOpen(CMCDEV * pCMCDev);
int MCDevClose(CMCDEV * pCMCDev);

//控制轴选择函数
int MCDevAxisSelReset(CMCDEV * pCMCDev);
int MCDevAxisSelSet(CMCDEV * pCMCDev,DWORD dwAxSel,DWORD dwCmdType);

//信息测试函数，供main函数调用
int MCDevCmdResShow(CMCDEV * pCMCDev,int iMsgType,char *chMsg);

//////////////////////////////////////////////////////////////////////////
//动作指令函数
//获取设备信息
int MCDevAxisGetStt(CMCAXS * pCMCAxs,char *chCmd);
int MCDevAxisGetPst(CMCAXS * pCMCAxs,char *chCmd);
int MCDevAxisGetVlc(CMCAXS * pCMCAxs,char *chCmd);

//轴运动指令
int MCDevAxisJogVlc(CMCAXS * pCMCAxs,char *chCmd);
int MCDevAxisJogPst(CMCAXS * pCMCAxs,char *chCmd);
int MCDevAxisJogStp(CMCAXS * pCMCAxs,char *chCmd);
int MCDevAxisJogHom(CMCAXS * pCMCAxs,char *chCmd);

//设置轴信息指令
int MCDevAxisSetVlc(CMCAXS * pCMCAxs,char *chCmd);
int MCDevAxisSetSpc(CMCAXS * pCMCAxs,char *chCmd);

//***************************************************************** 
// Function Define End
//***************************************************************** 
//#endif
//////////////////////////////////////////////////////////////////////////
