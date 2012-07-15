
//***************************************************************** 
// 这里书写的是Pmac运行时系统头文件，重用时只需要拷贝下列文档即可
// "MCSystem.h"
// "PmacRuntime.h"
// "PmacRuntime.c"
// "MCSysExe.C"
// 为了支持windows变量类别及函数，请在CVi工程中包含window头文件
// #include <windows.h>
// Release 2011-05-19 by mjq_clint(马腱杞 mjq_clint@hotmail.com)
// 本次发布只导出了4个常用功能函数
//***************************************************************** 
#include "stdio.h"
#include <windows.h>

//***************************************************************** 
// COMM normal Defines 
//***************************************************************** 
//
#define PMAC_DLL_NAME	TEXT("PComm32W.dll")

//////////////////////////////////////////////////////////////////////////
//***************************************************************** 
// COMM Type Defines 
//***************************************************************** 
//
typedef int (CALLBACK *pOpenPmacDevice)(DWORD dwDevID);
typedef int (CALLBACK *pClosePmacDevice)(DWORD dwDevID);
typedef long (CALLBACK *pSelectPmacDevice)(HWND hwnd );
typedef int (CALLBACK *pPmacGetResponseA)(DWORD dwDevID,char *chRes,int chLenMax,char *chSend);

//////////////////////////////////////////////////////////////////////////
//动态库连接操作宏
#define DLL_LINK(var,type,name)  var=(type)GetProcAddress(hPmacLib,name); \
                                 if (var==NULL) break

//***************************************************************** 
// COMM function Defines 
//***************************************************************** 
HINSTANCE PmacRuntimeLink(void);
void PmacRuntimeClose();

//***************************************************************** 
// COMM function Defines 
//***************************************************************** 
//为避免出现函数重复定义的错误，应在PMACRuntime源文件中定义函数
extern pOpenPmacDevice			PmacDevOpen;
extern pClosePmacDevice			PmacDevClose;
extern pPmacGetResponseA		PmacGetResponce;
extern pSelectPmacDevice		PmacDevSelect;

// COMM Defines End
//***************************************************************** 
//////////////////////////////////////////////////////////////////////////

//#endif