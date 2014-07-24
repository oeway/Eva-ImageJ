//***************************************************************** 
// 这里书写的是功能函数，重用时只需要拷贝下列文档即可
// "MCSystem.h"
// "PmacRuntime.h"
// "PmacRuntime.c"
// "MCSysExe.C"
// 为了支持windows变量类别及函数，请在CVi工程中包含window头文件
// #include <windows.h>
//***************************************************************** 
//////////////////////////////////////////////////////////////////////////
//
#include <windows.h>

//////////////////////////////////////////////////////////////////////////
//
#include "DataDefine.h"

#include "PCommAxis.h"
#include "MCAxisBase.h"
#include "PmacRuntime.h"
#include <string.h>
//////////////////////////////////////////////////////////////////////////
//完全初始化
int MCSysInit(CMCDEV * pCMCDev)
{
	//////////////////////////////////////////////////////////////////////////
	//初始化设备信息
	MCDevInit(&(*pCMCDev));

	//////////////////////////////////////////////////////////////////////////
	//导出动态库函数
	hinsPCommLib = PmacRuntimeLink();
	if (!hinsPCommLib)
	{
		return ERRO_INIT_LINK;
	}

	//////////////////////////////////////////////////////////////////////////
	//打开设备	
	pCMCDev->m_nDevStt = MCDevOpen(pCMCDev);
	if (!pCMCDev->m_nDevStt)
	{
		return ERRO_INIT_OPEN;
	}

	return ERRO_NOERR;
}
//完全释放控制
int MCSysRelease(CMCDEV * pCMCDev)
{
	if (!MCDevClose(pCMCDev))
	{
		return ERRO_RELEASE_CLOSE;
	}

	PmacRuntimeClose();

	return ERRO_NOERR;
}
//发送控制操作指令
int MCDevCmdSend(CMCDEV * pCMCDev, char * chRes, int nChLen, char * chSend)
{	
	int nIndex = 0;
	chRes[0] = '\0';
	chSend[0] = '\0';

	int nFlagSav = 0;
	char chCmdEx[10] = {'\0'};

	//根据设备选择及命令类别生成命令字符串
	for (nIndex = 0; nIndex < MCSYS_DEV_AXS_MAX; nIndex++)
	{
		if (pCMCDev->m_Axis[nIndex].m_AxsCtr.m_nIsSel) 
		{
			switch(pCMCDev->m_Axis[nIndex].m_AxsCtr.m_nCmdTyp)
			{
			case AXS_CMDTYPE_UNDEF:
				break;
			case AXS_CMDTYPE_GET_STT:
				MCDevAxisGetStt(&(pCMCDev->m_Axis[nIndex]),chSend);
				break;
			case AXS_CMDTYPE_GET_PST:
				MCDevAxisGetPst(&(pCMCDev->m_Axis[nIndex]),chSend);
				break;
			case AXS_CMDTYPE_GET_VLC:
				MCDevAxisGetVlc(&(pCMCDev->m_Axis[nIndex]),chSend);
				break;
				
			case AXS_CMDTYPE_JOG_VLC:
				MCDevAxisJogVlc(&(pCMCDev->m_Axis[nIndex]),chSend);
			case AXS_CMDTYPE_JOG_PST:
				MCDevAxisJogPst(&(pCMCDev->m_Axis[nIndex]),chSend);
				break;
			case AXS_CMDTYPE_JOG_HM:
			case AXS_CMDTYPE_JOG_HMZ:
				MCDevAxisJogHom(&(pCMCDev->m_Axis[nIndex]),chSend);
				break;
			case AXS_CMDTYPE_JOG_STP:
			case AXS_CMDTYPE_JOG_KLL:
				MCDevAxisJogStp(&(pCMCDev->m_Axis[nIndex]),chSend);
				break;
			case AXS_CMDTYPE_SET_VLC:
				{
					MCDevAxisSetVlc(&(pCMCDev->m_Axis[nIndex]),chSend);
					if (!nFlagSav)
						nFlagSav = 1;
				}
				break;
			case AXS_CMDTYPE_SET_SPC:
				{
					MCDevAxisSetSpc(&(pCMCDev->m_Axis[nIndex]),chSend);
					if (!nFlagSav)
						nFlagSav = 1;
				}
				break;
			//默认不执行任何指令
			default:
				sprintf(chSend,"\0");
			}
		}
	}
	if (nFlagSav) 
	{
		sprintf(chCmdEx," SAV");
		strcat(chSend, chCmdEx);
	}
	//发送命令字符串
	PmacGetResponce(pCMCDev->m_dwID, chRes, nChLen, chSend);
	//Sleep(1000);
	//判断是否有返回值
	if (!strlen(chRes))
	{
		return ERRO_NORMAL_CMD;
	}
	else
	{	
		//判断是否返回错误
		if (strstr(chRes,PMAC_RES_ERR))
		{
			return ERRO_NORMAL_CMD;
		}
		else
		{
			MCDevCmdResDataGet(pCMCDev,chRes,strlen(chRes));
			MCDevAxisSelReset(pCMCDev);
		}
	}

	return ERRO_NOERR;
}

//从命令返回字符串获取数据
int MCDevCmdResDataGet(CMCDEV * pCMCDev, char * chRes, int nChLen)
{
	int nIndex = 0, nOffSet = 0;
	//根据设备选择及命令类别生成命令字符串
	for (nIndex = 0; nIndex < MCSYS_DEV_AXS_MAX; nIndex++)
	{
		if (AXS_ISSEL_YES == pCMCDev->m_Axis[nIndex].m_AxsCtr.m_nIsSel) 
		{
			switch(pCMCDev->m_Axis[nIndex].m_AxsCtr.m_nCmdTyp)
			{
			case AXS_CMDTYPE_UNDEF:
				break;
			case AXS_CMDTYPE_GET_STT:
				nOffSet = MCDevCmdResDataGetStt(&(pCMCDev->m_Axis[nIndex]),chRes,nOffSet,nChLen);
				break;
			case AXS_CMDTYPE_GET_PST:
				nOffSet = MCDevCmdResDataGetPst(&(pCMCDev->m_Axis[nIndex]),chRes,nOffSet,nChLen);
				break;
			case AXS_CMDTYPE_GET_VLC:
				nOffSet = MCDevCmdResDataGetVlc(&(pCMCDev->m_Axis[nIndex]),chRes,nOffSet,nChLen);
				break;
			//默认不执行任何指令
			default:
				break;
			}
		}
	}

	return TRUE;
}

//获取速度返回值－设备轴状态
int MCDevCmdResDataGetStt(CMCAXS * pCMCAxs,char *chRes,int nOffSet,int nChLen)
{
	int nChDtLen = 0,nIndex = 0;
	char chBuffData[16] = {'\0'},
		chBuffAscii[3] = {'\0'},
		chBuffHex[16] = {'\0'},
		chDataBuff[32] = {'\0'};
	
	//查找并拷贝设备状态字符串
	for(nIndex = nOffSet; nIndex < nChLen; nIndex++)
	{
		if (0x0D == chRes[nIndex]) 
		{
			nChDtLen = nIndex - nOffSet;
			break;
		}
	}
	memcpy(chBuffData,chRes + nOffSet,nChDtLen);
	//ASCII转HEX
	//sprintf(chBuffHex,"%06X",chBuffData+0);
	
	for(nIndex = 0;nIndex < nChDtLen;nIndex++)
	{		
		chBuffHex[1] = (BYTE)chBuffData[2*nIndex] & 0x0F;
		if (chBuffData[2*nIndex] > 0x40)
		{
			chBuffHex[1] += 9;
		}
		chBuffHex[2] = chBuffData[2*nIndex + 1] & 0x0F;
		if (chBuffData[2*nIndex + 1] > 0x40)
		{
			chBuffHex[2] += 9;
		}
		chBuffHex[0] = ((BYTE)chBuffHex[1] << 4) | (BYTE)chBuffHex[2];
		//chBuffHex[0] |= (BYTE)chBuffHex[2];
		chDataBuff[nIndex] = chBuffHex[0];
	}
	/**/
	//获取数据
	//First DWORD X
	pCMCAxs->m_AxsStt.m_nActived = (BYTE)chDataBuff[0] >> 7;//23
	pCMCAxs->m_AxsStt.m_nNgLmted = ((BYTE)chDataBuff[0] >> 6) & 0x01;//22
	pCMCAxs->m_AxsStt.m_nPsLmted = ((BYTE)chDataBuff[0] >> 5) & 0x01;//21
	pCMCAxs->m_AxsStt.m_nHwEnable = ((BYTE)chDataBuff[0] >> 4) & 0x01;//20
	pCMCAxs->m_AxsStt.m_nPhasedMotor = ((BYTE)chDataBuff[0] >> 3) & 0x01;//19
	pCMCAxs->m_AxsStt.m_nOpenLoop = ((BYTE)chDataBuff[0] >> 2) & 0x01;//18
	pCMCAxs->m_AxsStt.m_nRunDfTime = ((BYTE)chDataBuff[0] >> 1) & 0x01;//17
	pCMCAxs->m_AxsStt.m_nIntegration = (BYTE)chDataBuff[0] & 0x01;//16
	//
	pCMCAxs->m_AxsStt.m_nDwell = (BYTE)chDataBuff[1] >> 7;//15
	pCMCAxs->m_AxsStt.m_nDBErr = ((BYTE)chDataBuff[1] >> 6) & 0x01;//14
	pCMCAxs->m_AxsStt.m_nDVZero = ((BYTE)chDataBuff[1] >> 5) & 0x01;//13
	pCMCAxs->m_AxsStt.m_nArtDcl = ((BYTE)chDataBuff[1] >> 4) & 0x01;//12
	pCMCAxs->m_AxsStt.m_nBloackRs = ((BYTE)chDataBuff[1] >> 3) & 0x01;//11
	pCMCAxs->m_AxsStt.m_nHmSrhing = ((BYTE)chDataBuff[1] >> 2) & 0x01;//10
	pCMCAxs->m_AxsStt.m_nPNextDB01 = (BYTE)chDataBuff[1] & 0x03;//8-9
	//
	pCMCAxs->m_AxsStt.m_nPNextDB02 = (BYTE)chDataBuff[2];//0-7
	//Secend DWORD Y
	pCMCAxs->m_AxsStt.m_nAssCS = (BYTE)chDataBuff[3] >> 7;//23
	pCMCAxs->m_AxsStt.m_nCS01Num = ((BYTE)chDataBuff[3] >> 4) & 0x07;//20-22
	pCMCAxs->m_AxsStt.m_nResFFU01 = (BYTE)chDataBuff[3] & 0x0F;//16-19
	//
	pCMCAxs->m_AxsStt.m_nResFFU02 = ((BYTE)chDataBuff[4] >> 7) & 0x01;//15
	pCMCAxs->m_AxsStt.m_nAmpEn = ((BYTE)chDataBuff[4] >> 6) & 0x01;//14
	pCMCAxs->m_AxsStt.m_nResFFU03 = ((BYTE)chDataBuff[4] >> 4) & 0x03;//12-13
	pCMCAxs->m_AxsStt.m_nStpOnPstLmt = ((BYTE)chDataBuff[4] >> 3) & 0x01;//11
	pCMCAxs->m_AxsStt.m_nHmCmp = ((BYTE)chDataBuff[4] >> 2) & 0x01;//10
	pCMCAxs->m_AxsStt.m_nResFFU04 = ((BYTE)chDataBuff[4] >> 1) & 0x01;//9
	pCMCAxs->m_AxsStt.m_nPhsSErr = (BYTE)chDataBuff[4] & 0x01;//8
	//
	pCMCAxs->m_AxsStt.m_nTrgMv = ((BYTE)chDataBuff[5] >> 7) & 0x01;//7
	pCMCAxs->m_AxsStt.m_nntFFErr = ((BYTE)chDataBuff[5] >> 6) & 0x01;//6
	pCMCAxs->m_AxsStt.m_nI2TAmpFErr = ((BYTE)chDataBuff[5] >> 5) & 0x01;//5
	pCMCAxs->m_AxsStt.m_nBckDrFlag = ((BYTE)chDataBuff[5] >> 4) & 0x01;//4
	pCMCAxs->m_AxsStt.m_nAmpFErr = ((BYTE)chDataBuff[5] >> 3) & 0x01;//3
	pCMCAxs->m_AxsStt.m_nFFErr = ((BYTE)chDataBuff[5] >> 2) & 0x01;//2
	pCMCAxs->m_AxsStt.m_nWFErr = ((BYTE)chDataBuff[5] >> 1) & 0x01;//1
	pCMCAxs->m_AxsStt.m_nInPst = (BYTE)chDataBuff[5] & 0x01;//0
	
	//返回操作结果
	return nOffSet + nChDtLen + 1;
}

//获取位移返回值－位移
int MCDevCmdResDataGetPst(CMCAXS * pCMCAxs,char *chRes,int nOffSet,int nChLen)
{
	int nChDtLen = 0,nIndex = 0;
	char chDataBuff[32] = {'\0'};
	
	for(nIndex = nOffSet; nIndex < nChLen; nIndex++)
	{
		if (0x0D == chRes[nIndex]) 
		{
			nChDtLen = nIndex - nOffSet;
			break;
		}
	}
	
	memcpy(chDataBuff,chRes + nOffSet,nChDtLen);
	pCMCAxs->m_AxsDat.m_dfPlsCur = atof(chDataBuff);
	return nOffSet + nChDtLen + 1;
}

//获取速度返回值－速度
int MCDevCmdResDataGetVlc(CMCAXS * pCMCAxs,char *chRes,int nOffSet,int nChLen)
{
	int nChDtLen = 0, nIndex = 0;
	char chDataBuff[32] = {'\0'};
	for(nIndex = nOffSet; nIndex < nChLen; nIndex++)
	{
		if (0x0D == chRes[nIndex]) 
		{
			nChDtLen = nIndex - nOffSet;
			break;
		}
	}
	
	memcpy(chDataBuff,chRes + nOffSet,nChDtLen);
	pCMCAxs->m_AxsInf.m_fVlcCur = (float)atof(chDataBuff);
	return nOffSet + nChDtLen + 1;
}

int MCDevCmdResShow(CMCDEV * pCMCDev,int iMsgType,char *chMsg)
{
	int nIndex = 0;
	char chBuff[32] = {'\0'};
	//根据设备选择及命令类别生成命令字符串
	for (nIndex = 0; nIndex < MCSYS_DEV_AXS_MAX; nIndex++)
	{
		switch(iMsgType)
		{
		case AXS_CMDTYPE_UNDEF:
			break;
		case AXS_CMDTYPE_GET_STT:
			sprintf(chBuff,"Axis%d LmtNg=%d\n LmtPs=%d\n",
				pCMCDev->m_Axis[nIndex].m_AxsInf.m_nAxsID,
				pCMCDev->m_Axis[nIndex].m_AxsStt.m_nNgLmted,
				pCMCDev->m_Axis[nIndex].m_AxsStt.m_nPsLmted);
			strcat(chMsg,chBuff);
			break;
		case AXS_CMDTYPE_GET_PST:
			sprintf(chBuff,"Axis%dP=%d\n",
				pCMCDev->m_Axis[nIndex].m_AxsInf.m_nAxsID,
				pCMCDev->m_Axis[nIndex].m_AxsDat.m_dfPlsCur);
			strcat(chMsg,chBuff);
			break;
		case AXS_CMDTYPE_GET_VLC:
			sprintf(chBuff,"Axis%dV=%d\n",
				pCMCDev->m_Axis[nIndex].m_AxsInf.m_nAxsID,
				pCMCDev->m_Axis[nIndex].m_AxsInf.m_fVlcCur);
			strcat(chMsg,chBuff);
			break;
			//默认不执行任何指令
		default:
			break;
		}
	}
	
	return TRUE;
}
//设置运动轴选择状态
int MCDevAxisSelSet(CMCDEV * pCMCDev, DWORD dwAxSel, DWORD dwCmdType)
{
	int nIndex = 0;

	for (nIndex = 0; nIndex < MCSYS_DEV_AXS_MAX; nIndex++)
	{
		pCMCDev->m_Axis[nIndex].m_AxsCtr.m_nIsSel = (dwAxSel >> nIndex) & 0x01;
		if (AXS_ISSEL_YES == pCMCDev->m_Axis[nIndex].m_AxsCtr.m_nIsSel) 
			pCMCDev->m_Axis[nIndex].m_AxsCtr.m_nCmdTyp = dwCmdType;
	}

	//设置参数值
	return TRUE;
}
//重置运动轴选择状态
int MCDevAxisSelReset(CMCDEV * pCMCDev)
{
	int nIndex = 0;
	for (nIndex = 0; nIndex < MCSYS_DEV_AXS_MAX; nIndex++)
	{
		pCMCDev->m_Axis[nIndex].m_AxsCtr.m_nIsSel = AXS_ISSEL_NO;
		pCMCDev->m_Axis[nIndex].m_AxsCtr.m_nCmdTyp = AXS_CMDTYPE_UNDEF;
	}
	return TRUE;
}
//初始化设备信息
int MCDevInit(CMCDEV * pCMCDev)
{
	int nIndex;
	//////////////////////////////////////////////////////////////////////////
	//初始化设备参数
	if ((int)pCMCDev->m_dwID < 0 || (int)pCMCDev->m_dwID > 32)
	{
		pCMCDev->m_dwID = 0x00;
	}

	//初始化运动轴信息
	pCMCDev->m_dwAxisSel = DEV_AXS_SEL_ALL;
	for(nIndex = 0; nIndex < DEV_AXS_MAX; nIndex++)
	{
		CMCAxsInit(
			&(pCMCDev->m_Axis[nIndex]),
			(DWORD)nIndex,
			1,
			AXS_TYPE_SERVO,
			200,
			1,
			1,
			AXS_MAXVAL_VLCASK,
			AXS_MAXVAL_VLCADD,
			AXS_MAXVAL_VLCDEC,
			600,
			200,
			long(0xFFFFFFF),
			long(0xEFFFFFF));
	}
	
	return TRUE;
}
//设备打开
int MCDevOpen(CMCDEV * pCMCDev)
{
	//判断是否设备ID
	if ((int)pCMCDev->m_dwID < 0 || (int)pCMCDev->m_dwID > 32)
	{
		pCMCDev->m_dwID = PmacDevSelect(NULL);
		return PmacDevOpen(pCMCDev->m_dwID);
	}
	else
	{
		return PmacDevOpen(pCMCDev->m_dwID);
	}
}
//设备关闭
int MCDevClose(CMCDEV * pCMCDev)
{
	if (pCMCDev->m_dwID > 0
		&& pCMCDev->m_dwID < 8) 
	{
		return PmacDevClose(pCMCDev->m_dwID);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//获取设备状态信息
int MCDevAxisGetStt(CMCAXS * pCMCAxs,char *chCmd)
{
	char chBuff[32];
	
	sprintf(chBuff,"#%d?",pCMCAxs->m_AxsInf.m_nAxsID);
	
	CMD_SEND_CHECK(chCmd," ");
	
	strcat(chCmd,chBuff);
	
	return ERRO_NOERR;
}

//
int MCDevAxisGetPst(CMCAXS * pCMCAxs,char *chSend)
{
	char chBuff[32];
	sprintf(chBuff,"#%dp",pCMCAxs->m_AxsInf.m_nAxsID);

	CMD_SEND_CHECK(chSend," ");

	strcat(chSend,chBuff);

	return ERRO_NOERR;
}

//速度控制指令
int MCDevAxisGetVlc(CMCAXS * pCMCAxs,char *chSend)
{
	char chBuff[32];
	
	sprintf(chBuff,"i%d22",pCMCAxs->m_AxsInf.m_nAxsID);
	
	CMD_SEND_CHECK(chSend," ");
	
	strcat(chSend,chBuff);
	
	return ERRO_NOERR;
}

//
int MCDevAxisJogVlc(CMCAXS * pCMCAxs,char *chSend)
{
	char chBuff[32];
	
	(PST_PLS_TYPE_REL == pCMCAxs->m_AxsCtr.m_nPlsTyp) ? 
		sprintf(chBuff,"#%dJ+",pCMCAxs->m_AxsInf.m_nAxsID) :
	sprintf(chBuff,"#%dJ-",pCMCAxs->m_AxsInf.m_nAxsID);
	
	CMD_SEND_CHECK(chSend," ");
	
	strcat(chSend,chBuff);
	
	return ERRO_NOERR;
}

//
int MCDevAxisJogPst(CMCAXS * pCMCAxs,char *chSend)
{
	char chBuff[32];
	
	(PST_PLS_TYPE_REL == pCMCAxs->m_AxsCtr.m_nPlsTyp) ? 
		sprintf(chBuff,"#%dJ:%2.2f",pCMCAxs->m_AxsInf.m_nAxsID,pCMCAxs->m_AxsDat.m_dfPlsAsk) :
	sprintf(chBuff,"#%dJ=%2.2f",pCMCAxs->m_AxsInf.m_nAxsID,pCMCAxs->m_AxsDat.m_dfPlsAsk);
	
	CMD_SEND_CHECK(chSend," ");
	
	strcat(chSend,chBuff);
	
	return ERRO_NOERR;
}

//
int MCDevAxisJogStp(CMCAXS * pCMCAxs,char *chSend)
{
	char chBuff[32];

	(PST_CTR_TYPE_OLOOP == pCMCAxs->m_AxsCtr.m_nCtrTyp) ? 
		sprintf(chBuff,"#%dK",pCMCAxs->m_AxsInf.m_nAxsID) :
		sprintf(chBuff,"#%dJ/",pCMCAxs->m_AxsInf.m_nAxsID);

	CMD_SEND_CHECK(chSend," ");

	strcat(chSend,chBuff);

	return ERRO_NOERR;
}

//
int MCDevAxisJogHom(CMCAXS * pCMCAxs,char *chSend)
{
	char chBuff[32];

	(PST_HOM_TYPE_RUN == pCMCAxs->m_AxsCtr.m_nHomTyp) ? 
		sprintf(chBuff,"#%dHom",pCMCAxs->m_AxsInf.m_nAxsID) :
		sprintf(chBuff,"#%dHomZ",pCMCAxs->m_AxsInf.m_nAxsID);

	CMD_SEND_CHECK(chSend," ");

	strcat(chSend,chBuff);

	return ERRO_NOERR;
}

//
int MCDevAxisSetVlc(CMCAXS * pCMCAxs,char *chSend)
{
	char chBuff[32];
	
	sprintf(chBuff,"i%d22=%f",
		pCMCAxs->m_AxsInf.m_nAxsID,pCMCAxs->m_AxsInf.m_fVlcCur);
	
	CMD_SEND_CHECK(chSend," ");
	
	strcat(chSend,chBuff);
	
	return ERRO_NOERR;
}

//
int MCDevAxisSetSpc(CMCAXS * pCMCAxs,char *chSend)
{
	char chBuff[64];
	//为避免越界，先强制结束符
	pCMCAxs->m_AxsDat.m_chData[31] = '\0';

	sprintf(chBuff,"#%d%s",
		pCMCAxs->m_AxsInf.m_nAxsID,pCMCAxs->m_AxsDat.m_chData);
	
	CMD_SEND_CHECK(chSend," ");
	
	strcat(chSend,chBuff);
	
	return ERRO_NOERR;
}