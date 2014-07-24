/************************************************************************/
/*                                                                      */
/************************************************************************/
#pragma once
//#ifndef _AXBS_
//#define _AXBS_ 
//////////////////////////////////////////////////////////////////////////
//电机类别
#define AXS_TYPE_UNDEF			0x00
#define AXS_TYPE_STEPER			0x01
#define AXS_TYPE_SERVO			0x02

//////////////////////////////////////////////////////////////////////////
//电机运动宏
//反向运动，取非为正向运动
#define AXS_DRC_NGT				0x0010

#define AXS_JOG_RUN				0x0001
#define AXS_JOG_STOP			0x0002
#define AXS_JOG_PST				0x0200

//点动模式，取非为一般运动模式
#define AXS_MOD_PST				0x0004

//////////////////////////////////////////////////////////////////////////
//轴状态
#define AXS_STAT_UNDEF			0x00
#define AXS_STAT_STOP			0x01
#define AXS_STAT_RUN			0x02
#define AXS_STAT_ERR			0x03

//定义是否选定宏
#define AXS_ISSEL_NO			0x00
#define AXS_ISSEL_YES			0x01

//定义指令类别宏
//未定义
#define	AXS_CMDTYPE_UNDEF		0x00
//获取:状态、位移、速度
#define	AXS_CMDTYPE_GET_STT		0x01
#define	AXS_CMDTYPE_GET_PST		0x02
#define	AXS_CMDTYPE_GET_VLC		0x03
//运动:速度模式、位置模式、回零运动、回零设置
#define	AXS_CMDTYPE_JOG_VLC		0x11
#define	AXS_CMDTYPE_JOG_PST		0x12
#define	AXS_CMDTYPE_JOG_HM		0x13
#define	AXS_CMDTYPE_JOG_HMZ		0x14
#define	AXS_CMDTYPE_JOG_STP		0x15
#define	AXS_CMDTYPE_JOG_KLL		0x16
//设置:速度级别、特殊指令
#define	AXS_CMDTYPE_SET_VLC		0x21
#define	AXS_CMDTYPE_SET_SPC		0x22
//////////////////////////////////////////////////////////////////////////
//定义脉冲方式宏或运动方向
#define PST_PLS_TYPE_REL		0x01//相对/正向
#define PST_PLS_TYPE_ABS		0x02//绝对/反向

//定义运行方式宏
#define PST_CTR_TYPE_OLOOP		0x01//开环
#define PST_CTR_TYPE_CLOOP		0x02//闭环

//定义回零方式宏
#define PST_HOM_TYPE_RUN		0x01//运动
#define PST_HOM_TYPE_SET		0x02//设置

//////////////////////////////////////////////////////////////////////////
//轴运动变量限制
#define AXS_MINVAL_PSTASK		50
#define AXS_MAXVAL_PSTASK		65535
#define AXS_MAXVAL_VLCASK		32000
#define AXS_MAXVAL_VLCADD		1000
#define AXS_MAXVAL_VLCDEC		1000

//////////////////////////////////////////////////////////////////////////
//
//***************************************************************** 
// Struct Define
//***************************************************************** 
//定义轴基本信息
/*
导程-》mmPr，传动机构每转传动距离
步距角-》plsPr，是一个基准与细分共同组成实际plsPr
细分-》plsPr Rang，倍率，乘以步距角得到轴的实际plsPr
减速比-》plsPr Rang，倍率，乘以步距角，乘以细分得到传动plsPr
脉冲速度-》plsPs，单位时间内发出的脉冲数
 */
typedef struct CMCAXSInfo {
	int m_nAxsID;/*轴编号*/
	int m_nAxsTyp;/*轴类型:Servo/Steper*/
	double m_fLead;/*导程(mm) Lead*/
	double m_fStpnNg;/*步距角 Step Angle (Pls/r)*/
	double m_fSbdvsn;/*细分 Subdivision (Pls/r Rang)*/
	double m_fDecRate;/*减速比*/
	double m_fVlcCur;/*速度(脉冲每秒) -Pls/s*/
	double m_fSpdMin;/*速度最小值-mm/s*/
	double m_fSpdMax;/*速度最大值-mm/s*/
	double m_fSpdAddMax;/*加速度最大值-mm/s2*/
	double m_fSpdDecMax;/*减速度最大值-mm/s2*/
	double m_fSpdNormal;/*速度(脉冲每秒) -mm/s*/
	double m_fSpdSpecial;/*速度(脉冲每秒) -mm/s*/
	double m_dfPlsMin;/*脉冲输出量最小值*/
	double m_dfPlsMax;/*脉冲输出量最小值*/
} CMCAXSINF;

//定义轴状态结构
typedef struct CMCAXSStat {
	//First DWORD X
 	int m_nActived;//23
	int m_nNgLmted;//22
	int m_nPsLmted;//21
	int m_nHwEnable;//20
	int m_nPhasedMotor;//19
	int m_nOpenLoop;//18
	int m_nRunDfTime;//17
	int m_nIntegration;//16
	int m_nDwell;//15
	int m_nDBErr;//14
	int m_nDVZero;//13
	int m_nArtDcl;//12
	int m_nBloackRs;//11
	int m_nHmSrhing;//10
	int m_nPNextDB01;//8-9
	int m_nPNextDB02;//0-7
	//Secend DWORD Y
	int m_nAssCS;//23
	int m_nCS01Num;//20-22
	int m_nResFFU01;//16-19
	int m_nResFFU02;//15
	int m_nAmpEn;//14
	int m_nResFFU03;//12-13
	int m_nStpOnPstLmt;//11
	int m_nHmCmp;//10
	int m_nResFFU04;//9
	int m_nPhsSErr;//8
	int m_nTrgMv;//7
	int m_nntFFErr;//6
	int m_nI2TAmpFErr;//5
	int m_nBckDrFlag;//4
	int m_nAmpFErr;//3
	int m_nFFErr;//2
	int m_nWFErr;//1
	int m_nInPst;//0
} CMCAXSSTT;

//定义轴控制信息
typedef struct CMCAXSCtrol {
	int m_nIsSel;//是否选择
	int m_nCmdTyp;//指令类别:获取/设置
	int m_nPlsTyp;//脉冲类别:相对/绝对
	int m_nCtrTyp;//控制方式:开环/闭环
	int m_nHomTyp;//回零方式:运动/置零
} CMCAXSCTR;

//定义轴结构
typedef struct CMCAXSData {
	char m_chData[32];//特殊数据
	double m_dfPlsAsk;//目标脉冲
	double m_dfPlsBgn;//起始脉冲
	double m_dfPlsCur;//当前脉冲
} CMCAXSDAT;

//定义轴结构
typedef struct CMCAxis {
	CMCAXSINF m_AxsInf;
	CMCAXSSTT m_AxsStt;
	CMCAXSCTR m_AxsCtr;
	CMCAXSDAT m_AxsDat;
} CMCAXS;

//////////////////////////////////////////////////////////////////////////
//初始化运动轴
int CMCAxsInit(
			   CMCAXS * myAxs,
			   int nAxsID,/*轴编号*/
			   int nAxsTyp,/*轴类型:Servo/Steper*/
			   float nLead,/*导程(mm) Lead*/
			   float nStpnNg,/*步距角 Step Angle*/
			   float nSbdvsn,/*细分 Subdivision*/
			   float nDecRate,/*减速比*/
			   float nSpdMax,/*速度最大值-Pls/s*/
			   float nSpdAddMax,/*加速度最大值-Pls/s2*/
			   float nSpdDecMax,/*减速度最大值-Pls/s2*/
			   float nSpdNormal,/*速度(脉冲每秒) -Pls/s*/
			   float nSpdSpecial,/*速度(脉冲每秒) -Pls/s*/
			   double dfPlsMin,/*脉冲输出量最小值*/
			   double dfPlsMax/*脉冲输出量最小值*/
			   );
//初始化轴信息
int CMCAxsInfInit(
				  CMCAXSINF * myAxsInf,
				  int nAxsID,/*轴编号*/
				  int nAxsTyp,/*轴类型:Servo/Steper*/
				  float nLead,/*导程(mm) Lead*/
				  float nStpnNg,/*步距角 Step Angle*/
				  float nSbdvsn,/*细分 Subdivision*/
				  float nDecRate,/*减速比*/
				  float nVlcMax,/*速度最大值-Pls/s*/
				  float nVlcAddMax,/*加速度最大值-Pls/s2*/
				  float nVlcDecMax,/*减速度最大值-Pls/s2*/
				  float nVlcNormal,/*速度(脉冲每秒) -Pls/s*/
				  float nVlcSpecial,/*速度(脉冲每秒) -Pls/s*/
				  double dfPlsMin,/*脉冲输出量最小值*/
				  double dfPlsMax/*脉冲输出量最小值*/
				  );
//初始化状态信息
int CMCAxsSttInit(CMCAXSSTT * myAxsStt);
//初始化控制信息
int CMCAxsCtrInit(CMCAXSCTR * myAxsCtr);
//初始化运动数据
int CMCAxsDatInit(CMCAXSDAT * myAxsDat);

//获取轴位移值
int CMCAxsGetPst(CMCAXSINF * myAxsInf, double nPls, double * nPst);
//获取轴脉冲量
int CMCAxsGetPls(CMCAXSINF * myAxsInf, double nPst, double * nPls);

//////////////////////////////////////////////////////////////////////////
//#endif
