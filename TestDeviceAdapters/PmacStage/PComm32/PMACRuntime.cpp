//***************************************************************** 
// 这里书写的是Pmac运行时系统文件，重用时只需要拷贝下列文档即可
// "MCSystem.h"
// "PmacRuntime.h"
// "PmacRuntime.c"
// "MCSysExe.C"
// 为了支持windows变量类别及函数，请在CVi工程中包含window头文件
// #include <windows.h>
// Release 2011-05-19 by mjq_clint(马腱杞 mjq_clint@hotmail.com)
// 本次发布只导出了4个常用功能函数
// 同时实现了PmacRuntimeLink和PmacRuntimeClose函数
//***************************************************************** 
#include "PMACRuntime.h"

//***************************************************************** 
// COMM function Defines 
//***************************************************************** 
//
pOpenPmacDevice			PmacDevOpen;
pClosePmacDevice		PmacDevClose;
pPmacGetResponseA		PmacGetResponce;
pSelectPmacDevice		PmacDevSelect;

//////////////////////////////////////////////////////////////////////////
//
HINSTANCE hinsPCommLib;

//////////////////////////////////////////////////////////////////////////
HINSTANCE PmacRuntimeLink()
{
	int nDllIm = 0;
	//加载动态库
	HINSTANCE hPmacLib;
	hPmacLib = LoadLibrary(PMAC_DLL_NAME);
	//导出动态库函数
	for (nDllIm = 0; nDllIm < 1; nDllIm++)
	{
		DLL_LINK(PmacDevOpen,pOpenPmacDevice,"OpenPmacDevice");
		DLL_LINK(PmacDevClose,pClosePmacDevice,"ClosePmacDevice");
		DLL_LINK(PmacGetResponce,pPmacGetResponseA,"PmacGetResponseA");
		DLL_LINK(PmacDevSelect,pSelectPmacDevice,"PmacSelect");
	}
	//判断加载结果
	if (nDllIm < 1)
	{
		FreeLibrary(hPmacLib);
		hPmacLib = NULL;
		return NULL;
	}
	else
	{
		return hPmacLib;
	}
}

void PmacRuntimeClose()
{
	if (hinsPCommLib) 
	{
		FreeLibrary(hinsPCommLib);
		hinsPCommLib = NULL;
	}
}
//////////////////////////////////////////////////////////////////////////
