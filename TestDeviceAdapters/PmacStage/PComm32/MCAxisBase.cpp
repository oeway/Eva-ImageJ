
/************************************************************************/
/*                                                                      */
/************************************************************************/
#include <windows.h>
#include "MCAxisBase.h"

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
			   float nVlcMax,/*速度最大值-Pls/s*/
			   float nVlcAddMax,/*加速度最大值-Pls/s2*/
			   float nVlcDecMax,/*减速度最大值-Pls/s2*/
			   float nVlcNormal,/*速度(脉冲每秒) -Pls/s*/
			   float nVlcSpecial,/*速度(脉冲每秒) -Pls/s*/
			   double dfPlsMin,/*脉冲输出量最小值*/
			   double dfPlsMax/*脉冲输出量最小值*/
			   )
{
	CMCAxsInfInit(&myAxs->m_AxsInf, 
		nAxsID, nAxsTyp, nLead, nStpnNg, nSbdvsn, nDecRate, 
		nVlcMax, nVlcAddMax, nVlcDecMax, nVlcNormal, nVlcSpecial, dfPlsMin, dfPlsMax);
	CMCAxsSttInit(&myAxs->m_AxsStt);
	CMCAxsCtrInit(&myAxs->m_AxsCtr);
	CMCAxsDatInit(&myAxs->m_AxsDat);

	return TRUE;
}

//初始化基本信息
int CMCAxsInfInit(
			  CMCAXSINF * myAxsInf,
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
			  )
{
	if (!myAxsInf)
	{
		return FALSE;
	}
	
	myAxsInf->m_nAxsID = nAxsID;/**/
	myAxsInf->m_nAxsTyp = nAxsTyp;/*轴类型:Servo/Steper*/
	myAxsInf->m_fLead = nLead;/*导程(mm) Lead*/
	myAxsInf->m_fStpnNg = nStpnNg;/*步距角 Step Angle*/
	myAxsInf->m_fSbdvsn = nSbdvsn;/*细分 Subdivision*/
	myAxsInf->m_fDecRate = nDecRate;
	myAxsInf->m_fVlcCur = 1.0f;/*速度(mm/s) mm/s*/
	myAxsInf->m_fSpdMin = 0.1f;
	myAxsInf->m_fSpdMax = nSpdMax;
	myAxsInf->m_fSpdAddMax = nSpdAddMax;
	myAxsInf->m_fSpdDecMax = nSpdDecMax;
	myAxsInf->m_fSpdNormal = nSpdNormal;/*速度(mm/s) mm/s*/
	myAxsInf->m_fSpdSpecial = nSpdSpecial;/*速度(mm/s) mm/s*/
	myAxsInf->m_dfPlsMin = dfPlsMin;/*脉冲输出量最小值*/
	myAxsInf->m_dfPlsMax = dfPlsMax;/*脉冲输出量最小值*/

	return TRUE;
}
//初始化状态信息
int CMCAxsSttInit(CMCAXSSTT * myAxsStt)
{
	myAxsStt->m_nActived = 0x00;//23
	myAxsStt->m_nNgLmted = 0x00;//22
	myAxsStt->m_nPsLmted = 0x00;//21
	myAxsStt->m_nHwEnable = 0x00;//20
	myAxsStt->m_nPhasedMotor = 0x00;//19
	myAxsStt->m_nOpenLoop = 0x00;//18
	myAxsStt->m_nRunDfTime = 0x00;//17
	myAxsStt->m_nIntegration = 0x00;//16
	myAxsStt->m_nDwell = 0x00;//15
	myAxsStt->m_nDBErr = 0x00;//14
	myAxsStt->m_nDVZero = 0x00;//13
	myAxsStt->m_nArtDcl = 0x00;//12
	myAxsStt->m_nBloackRs = 0x00;//11
	myAxsStt->m_nHmSrhing = 0x00;//10
	myAxsStt->m_nPNextDB01 = 0x00;//8-9
	myAxsStt->m_nPNextDB02 = 0x00;//0-7
	//Secend DWORD Y
	myAxsStt->m_nAssCS = 0x00;//23
	myAxsStt->m_nCS01Num = 0x00;//20-22
	myAxsStt->m_nResFFU01 = 0x00;//16-19
	myAxsStt->m_nResFFU02 = 0x00;//15
	myAxsStt->m_nAmpEn = 0x00;//14
	myAxsStt->m_nResFFU03 = 0x00;//12-13
	myAxsStt->m_nStpOnPstLmt = 0x00;//11
	myAxsStt->m_nHmCmp = 0x00;//10
	myAxsStt->m_nResFFU04 = 0x00;//9
	myAxsStt->m_nPhsSErr = 0x00;//8
	myAxsStt->m_nTrgMv = 0x00;//7
	myAxsStt->m_nntFFErr = 0x00;//6
	myAxsStt->m_nI2TAmpFErr = 0x00;//5
	myAxsStt->m_nBckDrFlag = 0x00;//4
	myAxsStt->m_nAmpFErr = 0x00;//3
	myAxsStt->m_nFFErr = 0x00;//2
	myAxsStt->m_nWFErr = 0x00;//1
	myAxsStt->m_nInPst = 0x00;//0
	
	return TRUE;
}

//初始化控制信息
int CMCAxsCtrInit(CMCAXSCTR * myAxsCtr)
{
	myAxsCtr->m_nIsSel = AXS_ISSEL_NO;//是否选择
	myAxsCtr->m_nCmdTyp = AXS_CMDTYPE_UNDEF;//指令类别:获取/设置
	myAxsCtr->m_nPlsTyp = PST_PLS_TYPE_REL;//脉冲类别:相对/绝对
	myAxsCtr->m_nCtrTyp = PST_CTR_TYPE_CLOOP;//控制方式:开环/闭环
	myAxsCtr->m_nHomTyp = PST_HOM_TYPE_RUN;//回零方式:运动/置零
	
	return TRUE;
}

//初始化基本数据
int CMCAxsDatInit(CMCAXSDAT * myAxsDat)
{
	myAxsDat->m_dfPlsAsk = 0.0f;//目标脉冲
	myAxsDat->m_dfPlsBgn = 0.0f;//起始脉冲
	myAxsDat->m_dfPlsCur = 0.0f;//当前脉冲

	return TRUE;
}

//获取轴位移值
int CMCAxsGetPst(CMCAXSINF * myAxsInf, double nPls, double * nPst)
{
	double nAxPlsPr;/*每转脉冲量(驱动):puls/r*/
	double nAxPlsPmm;/*单位位移脉冲量(减速后):puls/mm*/
	
	if (!myAxsInf)
	{
		return FALSE;
	}
	//计算中间值
	nAxPlsPr = (myAxsInf->m_fSbdvsn) * (myAxsInf->m_fStpnNg);/*每转脉冲量:puls/r*/
	nAxPlsPr = nAxPlsPr * (myAxsInf->m_fDecRate);/*每转脉冲量(减速后):puls/r*/
	nAxPlsPmm = nAxPlsPr / (myAxsInf->m_fLead);/*单位位移脉冲量(减速后):puls/mm*/
	//计算反馈值
	(* nPst) = nPls / nAxPlsPmm;
	
	return TRUE;
}

//获取轴脉冲量
int CMCAxsGetPls(CMCAXSINF * myAxsInf, double nPst, double * nPls)
{
	double nAxPlsPr;/*每转脉冲量(驱动):puls/r*/
	double nAxPlsPmm;/*单位位移脉冲量(减速后):puls/mm*/
	
	if (!myAxsInf)
	{
		return FALSE;
	}
	//计算中间值
	nAxPlsPr = (myAxsInf->m_fSbdvsn) * (myAxsInf->m_fStpnNg);/*每转脉冲量:puls/r*/
	nAxPlsPr = nAxPlsPr * (myAxsInf->m_fDecRate);/*每转脉冲量(减速后):puls/r*/
	nAxPlsPmm = nAxPlsPr / (myAxsInf->m_fLead);/*单位位移脉冲量(减速后):puls/mm*/
	//计算反馈值
	(* nPls) = nPst * nAxPlsPmm;
	
	return TRUE;
}
