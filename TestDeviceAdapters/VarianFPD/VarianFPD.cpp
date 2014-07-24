///////////////////////////////////////////////////////////////////////////////
// FILE:          VarianFPD.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Skeleton code for the micro-manager camera adapter. Use it as
//                starting point for writing custom device adapters
//                
// AUTHOR:        Nenad Amodaj, http://nenad.amodaj.com
//                
// COPYRIGHT:     University of California, San Francisco, 2011
//
// LICENSE:       This file is distributed under the BSD license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//

#include "VarianFPD.h"
#include "ModuleInterface.h"
#include <math.h>
#include "WriteCompactTiffRGB.h"

using namespace std;

const char* g_CameraName = "VarianFPD";

const char* g_PixelType_8bit = "8bit";
const char* g_PixelType_16bit = "16bit";

const char* g_CameraModelProperty = "Model";
const char* g_CameraModel_A = "2520";
//const char* g_CameraModel_B = "B";

#define LOWER_FRAME_RATE 1.0
// windows DLL entry code
#ifdef WIN32
BOOL APIENTRY DllMain(  HANDLE /*hModule*/, 
                        DWORD  ul_reason_for_call, 
                        LPVOID /*lpReserved*/ )
{
   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
      break;
   }
   return TRUE;
}
#endif


///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////

/**
 * List all supported hardware devices here
 */
MODULE_API void InitializeModuleData()
{
   AddAvailableDeviceName(g_CameraName, "Varian FPD by OEway");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return 0;

   // decide which device class to create based on the deviceName parameter
   if (strcmp(deviceName, g_CameraName) == 0)
   {
      // create camera
      return new VarianFPD();
   }

   // ...supplied name not recognized
   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// VarianFPD implementation
// ~~~~~~~~~~~~~~~~~~~~~~~

/**
* VarianFPD constructor.
* Setup default all variables and create device properties required to exist
* before intialization. In this case, no such properties were required. All
* properties will be created in the Initialize() method.
*
* As a general guideline Micro-Manager devices do not access hardware in the
* the constructor. We should do as little as possible in the constructor and
* perform most of the initialization in the Initialize() method.
*/
VarianFPD::VarianFPD() :
   bytesPerPixel_(2),
   initialized_(false),
   roiX_(0),
   roiY_(0),
   thd_(0),
   nominalPixelSizeUm_(127)
{

   // call the base class method to set-up default error codes/messages
   InitializeDefaultErrorMessages();

   // Description property
   int ret = CreateProperty(MM::g_Keyword_Description, "VarianFPD adapter--by OEway", MM::String, true);
   assert(ret == DEVICE_OK);

   // camera type pre-initialization property
   ret = CreateProperty(g_CameraModelProperty, g_CameraModel_A, MM::String, false, 0, true);
   assert(ret == DEVICE_OK);

   vector<string> modelValues;
   modelValues.push_back(g_CameraModel_A);
   //modelValues.push_back(g_CameraModel_B); 

   ret = SetAllowedValues(g_CameraModelProperty, modelValues);
   assert(ret == DEVICE_OK);
   GLiveParams=NULL;
   memset(&modeInfo, 0, sizeof(SModeInfo));
   modeInfo.StructSize = sizeof(SModeInfo);
   memset(&sysInfo, 0, sizeof(SSysInfo));
   sysInfo.StructSize = sizeof(SSysInfo);

   // create live video thread
   thd_ = new MySequenceThread(this);

}

/**
* VarianFPD destructor.
* If this device used as intended within the Micro-Manager system,
* Shutdown() will be always called before the destructor. But in any case
* we need to make sure that all resources are properly released even if
* Shutdown() was not called.
*/
VarianFPD::~VarianFPD()
{
   if (initialized_)
      Shutdown();
   delete thd_;
}

/**
* Obtains device name.
* Required by the MM::Device API.
*/
void VarianFPD::GetName(char* name) const
{
   // We just return the name we use for referring to this
   // device adapter.
   CDeviceUtils::CopyLimitedString(name, g_CameraName);
}

/**
* Intializes the hardware.
* Typically we access and initialize hardware at this point.
* Device properties are typically created here as well.
* Required by the MM::Device API.
*/
int VarianFPD::Initialize()
{
   if (initialized_)
      return DEVICE_OK;
   int ret = DEVICE_OK;
   CPropertyAction *pAct;
   // set property list
   // -----------------
   //VirtualCPLink
   const char* const v_Keyword_VirtualCPLink    = "VirtualCPLink";
   pAct = new CPropertyAction (this, &VarianFPD::OnVirtualCPLink);
   ret = CreateProperty(v_Keyword_VirtualCPLink, "OffLine", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> VirtualCPLinkValues;
   VirtualCPLinkValues.push_back("OffLine");
   VirtualCPLinkValues.push_back("OnLine"); 
   ret = SetAllowedValues(v_Keyword_VirtualCPLink, VirtualCPLinkValues);
   assert(ret == DEVICE_OK);
   //configurationPath
   const char* const v_Keyword_ConfigurationPath = "ConfigurationPath";
   pAct = new CPropertyAction (this, &VarianFPD::OnConfigurationPath);
   ret = CreateProperty(v_Keyword_ConfigurationPath, "C:\\IMAGERs\\9104-06", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   configurationPath_.assign("C:\\IMAGERs\\9104-06");
   // binning
   pAct = new CPropertyAction (this, &VarianFPD::OnBinning);
   ret = CreateProperty(MM::g_Keyword_Binning, "1", MM::Integer, false, pAct);
   assert(ret == DEVICE_OK);
   vector<string> binningValues;
   binningValues.push_back("1");
   binningValues.push_back("2"); 
   ret = SetAllowedValues(MM::g_Keyword_Binning, binningValues);
   assert(ret == DEVICE_OK);
   binning_=1;
   //gain
   ret = CreateProperty(MM::g_Keyword_Gain, "0.8", MM::Float, false);
   assert(ret == DEVICE_OK);
   SetPropertyLimits(MM::g_Keyword_Gain, 0.0, 1.0);
   gain_=0.8;
   //mode
   const char* const v_Keyword_Mode    = "Mode";
   pAct = new CPropertyAction (this, &VarianFPD::OnMode);
   ret = CreateProperty(v_Keyword_Mode, "High-Res Fluoro", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> modeValues;
   modeValues.push_back("High-Res Fluoro");
   modeValues.push_back("Low-Res Fluoro");
   modeValues.push_back("RAD"); 
   modeValues.push_back("Demo");
   ret = SetAllowedValues(v_Keyword_Mode, modeValues);
   assert(ret == DEVICE_OK);
   mode_.assign("Demo");
   crntModeSelect = -1;
   //FrameRate
   const char* const v_Keyword_FrameRate    = "FrameRate";
   pAct = new CPropertyAction (this, &VarianFPD::OnFrameRate);
   ret = CreateProperty(v_Keyword_FrameRate, "1.0", MM::Float, false,pAct);
   assert(ret == DEVICE_OK);
   ret = SetPropertyLimits(v_Keyword_FrameRate, 0.001 ,100.0);
   assert(ret == DEVICE_OK);
   frameRate_=1.0;
   //Exposure
   const char* const v_Keyword_Exposure    = "Exposure";
   pAct = new CPropertyAction (this, &VarianFPD::OnExposure);
   ret = CreateProperty(v_Keyword_Exposure, "1000.0", MM::Float, false,pAct);
   assert(ret == DEVICE_OK);
   //ret = SetPropertyLimits(v_Keyword_Exposure, 0.1 ,1000.0);
   assert(ret == DEVICE_OK);
   exposure_=1000.0;

   const char* const v_Keyword_AcqFrameCount   = "AcqFrameCount";
   ret = CreateProperty(v_Keyword_AcqFrameCount, "1", MM::Integer, true);
   assert(ret == DEVICE_OK);

   const char* const v_Keyword_AcqCalibrationCount   = "AcqCalibrationCount";
   ret = CreateProperty(v_Keyword_AcqCalibrationCount, "10", MM::Integer, true);
   assert(ret == DEVICE_OK);

   const char* const v_Keyword_ModeDescription  = "ModeDescription";
   ret = CreateProperty(v_Keyword_ModeDescription, "NULL", MM::String, true);
   assert(ret == DEVICE_OK);

   //AnalogGain
   const char* const v_Keyword_AnalogGain    = "AnalogGain";
   pAct = new CPropertyAction (this, &VarianFPD::OnAnalogGain);
   ret = CreateProperty(v_Keyword_AnalogGain, "1.0", MM::Float, false,pAct);
   assert(ret == DEVICE_OK);
   ret = SetPropertyLimits(v_Keyword_AnalogGain, 0.0,1.0);
   assert(ret == DEVICE_OK);
   analogGain_=1.0;

   //FrameNum4Cal
   const char* const v_Keyword_FrameNum4Cal    = "FrameNum4Cal";
   pAct = new CPropertyAction (this, &VarianFPD::OnFrameNum4Cal);
   ret = CreateProperty(v_Keyword_FrameNum4Cal, "32", MM::Integer, false,pAct);
   assert(ret == DEVICE_OK);
   ret = SetPropertyLimits(v_Keyword_FrameNum4Cal, 1,255);
   assert(ret == DEVICE_OK);
   frameNum4Cal_=32;

   //FrameNum4Acq
   const char* const v_Keyword_FrameNum4Acq    = "FrameNum4Acq";
   pAct = new CPropertyAction (this, &VarianFPD::OnFrameNum4Acq);
   ret = CreateProperty(v_Keyword_FrameNum4Acq, "1", MM::Integer, false,pAct);
   assert(ret == DEVICE_OK);
   ret = SetPropertyLimits(v_Keyword_FrameNum4Acq, 1,255);
   assert(ret == DEVICE_OK);
   frameNum4Acq_=1;

   //CalibrationMode
   const char* const v_Keyword_CalibrationMode    = "CalibrationMode";
   pAct = new CPropertyAction (this, &VarianFPD::OnCalibrationMode);
   ret = CreateProperty(v_Keyword_CalibrationMode, "Normal", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> CalibrationModeValues;
   CalibrationModeValues.push_back("Normal");
   CalibrationModeValues.push_back("OffsetCalibration"); 
   CalibrationModeValues.push_back("GainCalibration"); 
   ret = SetAllowedValues(v_Keyword_CalibrationMode, CalibrationModeValues);
   assert(ret == DEVICE_OK);
   calibrationMode_.assign("Normal");
   //AutoRectify
   const char* const v_Keyword_AutoRectify    = "AutoRectify";
   pAct = new CPropertyAction (this, &VarianFPD::OnAutoRectify);
   ret = CreateProperty(v_Keyword_AutoRectify, "Enable", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> AutoRectifyValues;
   AutoRectifyValues.push_back("Enable");
   AutoRectifyValues.push_back("Disable"); 
   ret = SetAllowedValues(v_Keyword_AutoRectify, AutoRectifyValues);
   assert(ret == DEVICE_OK);
   autoRectify_.assign("Enable");
   //OffsetCorrect
   const char* const v_Keyword_OffsetCorrect    = "OffsetCorrect";
   pAct = new CPropertyAction (this, &VarianFPD::OnOffsetCorrect);
   ret = CreateProperty(v_Keyword_OffsetCorrect, "Enable", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> OffsetCorrectValues;
   OffsetCorrectValues.push_back("Enable");
   OffsetCorrectValues.push_back("Disable"); 
   ret = SetAllowedValues(v_Keyword_OffsetCorrect, OffsetCorrectValues);
   assert(ret == DEVICE_OK);
   //GainCorrect
   const char* const v_Keyword_GainCorrect    = "GainCorrect";
   pAct = new CPropertyAction (this, &VarianFPD::OnGainCorrect);
   ret = CreateProperty(v_Keyword_GainCorrect, "Enable", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> GainCorrectValues;
   GainCorrectValues.push_back("Enable");
   GainCorrectValues.push_back("Disable"); 
   ret = SetAllowedValues(v_Keyword_GainCorrect, GainCorrectValues);
   assert(ret == DEVICE_OK);

   //DefectCorrect
   const char* const v_Keyword_DefectCorrect    = "DefectCorrect";
   pAct = new CPropertyAction (this, &VarianFPD::OnDefectCorrect);
   ret = CreateProperty(v_Keyword_DefectCorrect, "Enable", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> DefectCorrectValues;
   DefectCorrectValues.push_back("Enable");
   DefectCorrectValues.push_back("Disable"); 
   ret = SetAllowedValues(v_Keyword_DefectCorrect, DefectCorrectValues);
   assert(ret == DEVICE_OK);
   
   //Rotate90
   const char* const v_Keyword_Rotate90    = "Rotate90";
   pAct = new CPropertyAction (this, &VarianFPD::OnRotate90);
   ret = CreateProperty(v_Keyword_Rotate90, "Enable", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> Rotate90Values;
   Rotate90Values.push_back("Enable");
   Rotate90Values.push_back("Disable"); 
   ret = SetAllowedValues(v_Keyword_Rotate90, Rotate90Values);
   assert(ret == DEVICE_OK);

   //FlipX
   const char* const v_Keyword_FlipX    = "FlipX";
   pAct = new CPropertyAction (this, &VarianFPD::OnFlipX);
   ret = CreateProperty(v_Keyword_FlipX, "Enable", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> FlipXValues;
   FlipXValues.push_back("Enable");
   FlipXValues.push_back("Disable"); 
   ret = SetAllowedValues(v_Keyword_FlipX, FlipXValues);
   assert(ret == DEVICE_OK);

   //FlipY
   const char* const v_Keyword_FlipY    = "FlipY";
   pAct = new CPropertyAction (this, &VarianFPD::OnFlipY);
   ret = CreateProperty(v_Keyword_FlipY, "Enable", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> FlipYValues;
   FlipYValues.push_back("Enable");
   FlipYValues.push_back("Disable"); 
   ret = SetAllowedValues(v_Keyword_FlipY, FlipYValues);
   assert(ret == DEVICE_OK);

	ofstVcpSetting_.assign("Enable");
	gainVcpSetting_.assign("Enable");
	dfctVcpSetting_.assign("Enable");
	rotaVcpSetting_.assign("Enable");
	flpXVcpSetting_.assign("Disable");
	flpYVcpSetting_.assign("Disable");

   //CalibrationProgress
   const char* const v_Keyword_CalibrationProgress    = "CalibrationProgress";
   pAct = new CPropertyAction (this, &VarianFPD::OnCalibrationProgress);
   ret = CreateProperty(v_Keyword_CalibrationProgress, "-", MM::String, false,pAct);
   assert(ret == DEVICE_OK);

   //OffsetCalibration
   const char* const v_Keyword_OffsetCalibration    = "OffsetCalibration";
   pAct = new CPropertyAction (this, &VarianFPD::OnOffsetCalibration);
   ret = CreateProperty(v_Keyword_OffsetCalibration, "No", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> OffsetCalibrationValues;
   OffsetCalibrationValues.push_back("Yes");
   OffsetCalibrationValues.push_back("No"); 
   OffsetCalibrationValues.push_back("OK"); 
   ret = SetAllowedValues(v_Keyword_OffsetCalibration, OffsetCalibrationValues);
   assert(ret == DEVICE_OK);
   //GainCalibrationStep1
   const char* const v_Keyword_GainCalibrationStep1    = "GainCalibrationStep1";
   pAct = new CPropertyAction (this, &VarianFPD::OnGainCalibrationStep1);
   ret = CreateProperty(v_Keyword_GainCalibrationStep1, "No", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> GainCalibrationStep1Values;
   GainCalibrationStep1Values.push_back("Yes");
   GainCalibrationStep1Values.push_back("No"); 
   GainCalibrationStep1Values.push_back("OK"); 
   ret = SetAllowedValues(v_Keyword_GainCalibrationStep1, GainCalibrationStep1Values);
   assert(ret == DEVICE_OK);
   //GainCalibrationStep2
   const char* const v_Keyword_GainCalibrationStep2    = "GainCalibrationStep2";
   pAct = new CPropertyAction (this, &VarianFPD::OnGainCalibrationStep2);
   ret = CreateProperty(v_Keyword_GainCalibrationStep2, "No", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> GainCalibrationStep2Values;
   GainCalibrationStep2Values.push_back("Yes");
   GainCalibrationStep2Values.push_back("No"); 
   GainCalibrationStep2Values.push_back("OK"); 
   ret = SetAllowedValues(v_Keyword_GainCalibrationStep2, GainCalibrationStep2Values);
   assert(ret == DEVICE_OK);
   //GainCalibrationStep3
   const char* const v_Keyword_GainCalibrationStep3    = "GainCalibrationStep3";
   pAct = new CPropertyAction (this, &VarianFPD::OnGainCalibrationStep3);
   ret = CreateProperty(v_Keyword_GainCalibrationStep3, "No", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> GainCalibrationStep3Values;
   GainCalibrationStep3Values.push_back("Yes");
   GainCalibrationStep3Values.push_back("No"); 
   GainCalibrationStep3Values.push_back("OK"); 
   ret = SetAllowedValues(v_Keyword_GainCalibrationStep3, GainCalibrationStep3Values);
   assert(ret == DEVICE_OK);
   //GainCalibrationStep4
   const char* const v_Keyword_GainCalibrationStep4    = "GainCalibrationStep4";
   pAct = new CPropertyAction (this, &VarianFPD::OnGainCalibrationStep4);
   ret = CreateProperty(v_Keyword_GainCalibrationStep4, "No", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> GainCalibrationStep4Values;
   GainCalibrationStep4Values.push_back("Yes");
   GainCalibrationStep4Values.push_back("No"); 
   GainCalibrationStep4Values.push_back("OK"); 
   ret = SetAllowedValues(v_Keyword_GainCalibrationStep4, GainCalibrationStep4Values);
   assert(ret == DEVICE_OK);
   //ResetState
   const char* const v_Keyword_ResetState    = "ResetState";
   pAct = new CPropertyAction (this, &VarianFPD::OnResetState);
   ret = CreateProperty(v_Keyword_ResetState, "No", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> ResetStateValues;
   ResetStateValues.push_back("Yes");
   ResetStateValues.push_back("No"); 
   ResetStateValues.push_back("OK"); 
   ret = SetAllowedValues(v_Keyword_ResetState, ResetStateValues);
   assert(ret == DEVICE_OK);
   //ResetLink
   const char* const v_Keyword_ResetLink    = "ResetLink";
   pAct = new CPropertyAction (this, &VarianFPD::OnResetLink);
   ret = CreateProperty(v_Keyword_ResetLink, "No", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> ResetLinkValues;
   ResetLinkValues.push_back("Yes");
   ResetLinkValues.push_back("No"); 
   ResetLinkValues.push_back("OK"); 
   ret = SetAllowedValues(v_Keyword_ResetLink, ResetLinkValues);
   assert(ret == DEVICE_OK);

//--------------------ReadOnly Properties --------------------------
   //AcquisitionType
   const char* const v_Keyword_AcquisitionType    = "AcquisitionType";
   pAct = new CPropertyAction (this, &VarianFPD::OnAcquisitionType);
   ret = CreateProperty(v_Keyword_AcquisitionType, "Fluoro", MM::String, true,pAct);
   assert(ret == DEVICE_OK);
   vector<string> acqTypeValues;
   acqTypeValues.push_back("Fluoro");
   acqTypeValues.push_back("RAD"); 
   acqTypeValues.push_back("Demo");
   ret = SetAllowedValues(v_Keyword_AcquisitionType, acqTypeValues);
   assert(ret == DEVICE_OK);
   acquisitionType_.assign("Demo");
   // pixel type
   pAct = new CPropertyAction (this, &VarianFPD::OnPixelType);
   ret = CreateProperty(MM::g_Keyword_PixelType, g_PixelType_16bit, MM::String, true, pAct);
   assert(ret == DEVICE_OK);
   vector<string> pixelTypeValues;
   pixelTypeValues.push_back(g_PixelType_8bit);
   pixelTypeValues.push_back(g_PixelType_16bit); 
   ret = SetAllowedValues(MM::g_Keyword_PixelType, pixelTypeValues);
   assert(ret == DEVICE_OK);
   pixelType_.assign(g_PixelType_16bit);
   //Resolution
   const char* const v_Keyword_Resolution    = "Resolution";
   pAct = new CPropertyAction (this, &VarianFPD::OnResolution);
   ret = CreateProperty(v_Keyword_Resolution, "1536*1920", MM::String, true,pAct);//read only
   assert(ret == DEVICE_OK);
   resolution_.assign("1536*1920");
   //MaxResolution
   const char* const v_Keyword_MaxResolution    = "MaxResolution";
   pAct = new CPropertyAction (this, &VarianFPD::OnMaxResolution);
   ret = CreateProperty(v_Keyword_MaxResolution, "1536*1920", MM::String, true,pAct);//read only
   assert(ret == DEVICE_OK);
   maxResolution_.assign("1536*1920");
	//NumModes
	 const char* const v_Keyword_NumModes    = "NumModes";
	pAct = new CPropertyAction (this, &VarianFPD::OnNumModes);
	 ret = CreateProperty(v_Keyword_NumModes, "0", MM::Integer, true,pAct);
	 assert(ret == DEVICE_OK);
	 numModes_ = 0;
	//ReceptorType--A numerical value corresponding to the type of receptor
	const char* const v_Keyword_ReceptorType    = "ReceptorType";
	pAct = new CPropertyAction (this, &VarianFPD::OnReceptorType);
	ret = CreateProperty(v_Keyword_ReceptorType, "0", MM::Integer, true,pAct);
	assert(ret == DEVICE_OK);
	receptorType_=0;
	//MaxPixelValue--The maximum value of a pixel. The 2520E has 12-bit A/D conversion, so this value will be 4095.
    const char* const v_Keyword_MaxPixelValue    = "MaxPixelValue";
    pAct = new CPropertyAction (this, &VarianFPD::OnMaxPixelValue);
    ret = CreateProperty(v_Keyword_MaxPixelValue, "4095", MM::Integer, true,pAct);
    assert(ret == DEVICE_OK);
	maxPixelValue_=4095;
    //SystemDescription
    const char* const v_Keyword_SystemDescription    = "SystemDescription";
    pAct = new CPropertyAction (this, &VarianFPD::OnSystemDescription);
    ret = CreateProperty(v_Keyword_SystemDescription, "PaxScan", MM::String, true,pAct);
    assert(ret == DEVICE_OK);
	systemDescription_.assign("PaxScan");
    //TubeVoltage
    const char* const v_Keyword_TubeVoltage    = "TubeVoltage";
    pAct = new CPropertyAction (this, &VarianFPD::onTubeVoltage);
    ret = CreateProperty(v_Keyword_TubeVoltage, "-kV", MM::String, false,pAct);
    assert(ret == DEVICE_OK);
	tubeVoltage_.assign("-kV");

    //TubeCurrent
    const char* const v_Keyword_TubeCurrent    = "TubeCurrent";
    pAct = new CPropertyAction (this, &VarianFPD::onTubeCurrent);
    ret = CreateProperty(v_Keyword_TubeCurrent, "-mA", MM::String, false,pAct);
    assert(ret == DEVICE_OK);
	tubeCurrent_.assign("-mA");
    //Operator
    const char* const v_Keyword_Operator    = "Operator";
    pAct = new CPropertyAction (this, &VarianFPD::onOperator);
    ret = CreateProperty(v_Keyword_Operator, "-", MM::String, false,pAct);
    assert(ret == DEVICE_OK);
	operator_.assign("-");
    //SID
    const char* const v_Keyword_SID    = "SID";
    pAct = new CPropertyAction (this, &VarianFPD::onSID);
    ret = CreateProperty(v_Keyword_SID, "-", MM::String, false,pAct);
    assert(ret == DEVICE_OK);
	sid_.assign("-");
	//SOD
    const char* const v_Keyword_SOD    = "SOD";
    pAct = new CPropertyAction (this, &VarianFPD::onSOD);
    ret = CreateProperty(v_Keyword_SOD, "-", MM::String, false,pAct);
    assert(ret == DEVICE_OK);
	sod_.assign("-");
	//Tag
    const char* const v_Keyword_Tag    = "Tag";
    pAct = new CPropertyAction (this, &VarianFPD::onTag);
    ret = CreateProperty(v_Keyword_Tag, "-", MM::String, false,pAct);
    assert(ret == DEVICE_OK);
	tag_.assign("-");

	// initialize the varriables
	//-------------------------------------------
	imageWidth_ = 1536;
	imageHeight_ = 1920;
	maxBitDepth_ = 14;

	ret = openLink();
	//assert(ret == DEVICE_OK);
	if(ret != DEVICE_OK)
	{
		virtualCPLink_.assign("OffLine");
		LogMessage("Open Link Fail!");
	}
	else
	{
		virtualCPLink_.assign("OnLine");
		ret = getSysInfo();
		if(ret != DEVICE_OK)
			LogMessage("Get System Information Fail!");
	}
	ret=vip_reset_state();//TODO:handle error

	// synchronize all properties
    // --------------------------
   ret = UpdateStatus();
   if (ret != DEVICE_OK)
      return ret;

   // setup the buffer
   // ----------------
   ret = ResizeImageBuffer();
   if (ret != DEVICE_OK)
      return ret;
   SetProperty(v_Keyword_Mode,"High-Res Fluoro");
   initialized_ = true;
   return DEVICE_OK;
}

/**
* Shuts down (unloads) the device.
* Ideally this method will completely unload the device and release all resources.
* Shutdown() may be called multiple times in a row.
* Required by the MM::Device API.
*/
int VarianFPD::Shutdown()
{
   initialized_ = false;
   closeLink();
   return DEVICE_OK;
}

/**
* Performs exposure and grabs a single image.
* This function should block during the actual exposure and return immediately afterwards 
* (i.e., before readout).  This behavior is needed for proper synchronization with the shutter.
* Required by the MM::Camera API.
*/
int VarianFPD::SnapImage()
{
	int ret=DEVICE_ERR;

	if(acquisitionType_.compare("Demo") ==0)
	{
		GenerateSyntheticImage(img_,GetExposure());
		return DEVICE_OK;
	}
	else if(acquisitionType_.compare("Fluoro")==0)
	{
		ret = fluoro_acqImage();
	}
	else// if(acquisitionType_.compare("RAD")==0)
	{
		ret = rad_acqImage();
	}
	if(ret != DEVICE_OK)//may be disconnected
	{	
		ret =checkLink();
		if(ret != DEVICE_OK)
			ret = openLink();
		if(ret == DEVICE_OK)
		{
			//and , let's do it again...
			if(acquisitionType_.compare("Demo") ==0)
			{
				GenerateSyntheticImage(img_,GetExposure());
				return DEVICE_OK;
			}
			else if(acquisitionType_.compare("Fluoro")==0)
			{
				ret = fluoro_acqImage();
				ret = InsertImage();
			}
			else //if(acquisitionType_.compare("RAD")==0)
			{
				ret = rad_acqImage();
				ret = InsertImage();
			}
		}
	}
	return ret;
}

/**
* Returns pixel data.
* Required by the MM::Camera API.
* The calling program will assume the size of the buffer based on the values
* obtained from GetImageBufferSize(), which in turn should be consistent with
* values returned by GetImageWidth(), GetImageHight() and GetImageBytesPerPixel().
* The calling program allso assumes that camera never changes the size of
* the pixel buffer on its own. In other words, the buffer can change only if
* appropriate properties are set (such as binning, pixel type, etc.)
*/
const unsigned char* VarianFPD::GetImageBuffer()
{
   return const_cast<unsigned char*>(img_.GetPixels());
}

/**
* Returns image buffer X-size in pixels.
* Required by the MM::Camera API.
*/
unsigned VarianFPD::GetImageWidth() const
{
   return img_.Width();
}

/**
* Returns image buffer Y-size in pixels.
* Required by the MM::Camera API.
*/
unsigned VarianFPD::GetImageHeight() const
{
   return img_.Height();
}

/**
* Returns image buffer pixel depth in bytes.
* Required by the MM::Camera API.
*/
unsigned VarianFPD::GetImageBytesPerPixel() const
{
   return img_.Depth();
} 

/**
* Returns the bit depth (dynamic range) of the pixel.
* This does not affect the buffer size, it just gives the client application
* a guideline on how to interpret pixel values.
* Required by the MM::Camera API.
*/
unsigned VarianFPD::GetBitDepth() const
{
   return maxBitDepth_;
}

/**
* Returns the size in bytes of the image buffer.
* Required by the MM::Camera API.
*/
long VarianFPD::GetImageBufferSize() const
{
   return img_.Width() * img_.Height() * GetImageBytesPerPixel();
}

/**
* Sets the camera Region Of Interest.
* Required by the MM::Camera API.
* This command will change the dimensions of the image.
* Depending on the hardware capabilities the camera may not be able to configure the
* exact dimensions requested - but should try do as close as possible.
* If the hardware does not have this capability the software should simulate the ROI by
* appropriately cropping each frame.
* This demo implementation ignores the position coordinates and just crops the buffer.
* @param x - top-left corner coordinate
* @param y - top-left corner coordinate
* @param xSize - width
* @param ySize - height
*/
int VarianFPD::SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize)
{
   if (xSize == 0 && ySize == 0)
   {
      // effectively clear ROI
      ResizeImageBuffer();
      roiX_ = 0;
      roiY_ = 0;
   }
   else
   {
      // apply ROI
      img_.Resize(xSize, ySize);
      roiX_ = x;
      roiY_ = y;
   }
   return DEVICE_OK;
}

/**
* Returns the actual dimensions of the current ROI.
* Required by the MM::Camera API.
*/
int VarianFPD::GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize)
{
   x = roiX_;
   y = roiY_;

   xSize = img_.Width();
   ySize = img_.Height();

   return DEVICE_OK;
}

/**
* Resets the Region of Interest to full frame.
* Required by the MM::Camera API.
*/
int VarianFPD::ClearROI()
{
   ResizeImageBuffer();
   roiX_ = 0;
   roiY_ = 0;
      
   return DEVICE_OK;
}

/**
* Returns the current exposure setting in milliseconds.
* Required by the MM::Camera API.
*/
double VarianFPD::GetExposure() const
{
   return exposure_;
}

/**
* Sets exposure in milliseconds.
* Required by the MM::Camera API.
*/
void VarianFPD::SetExposure(double exp)
{
    double p= exp;
	SetProperty("Exposure",CDeviceUtils::ConvertToString(p));
}

/**
* Returns the current binning factor.
* Required by the MM::Camera API.
*/
int VarianFPD::GetBinning() const
{
   return binning_;
}

/**
* Sets binning factor.
* Required by the MM::Camera API.
*/
int VarianFPD::SetBinning(int binF)
{
   return SetProperty(MM::g_Keyword_Binning, CDeviceUtils::ConvertToString(binF));
}

int VarianFPD::PrepareSequenceAcqusition()
{
   if (IsCapturing())
      return DEVICE_CAMERA_BUSY_ACQUIRING;

   int ret = GetCoreCallback()->PrepareForAcq(this);
   if (ret != DEVICE_OK)
      return ret;

   return DEVICE_OK;
}


/**
 * Required by the MM::Camera API
 * Please implement this yourself and do not rely on the base class implementation
 * The Base class implementation is deprecated and will be removed shortly
 */
int VarianFPD::StartSequenceAcquisition(double interval) {

	//Exit current progress
	int ret=vip_reset_state();
	if(ret!= HCP_NO_ERR)
		ret = checkLink();
	if(ret == DEVICE_OK )
		return StartSequenceAcquisition(LONG_MAX, interval, false); 
	else 
		return ret;
}

/**                                                                       
* Stop and wait for the Sequence thread finished                                   
*/                                                                      
int VarianFPD::StopSequenceAcquisition()                                     
{   int result=VIP_NO_ERR;                                                                    
   if (!thd_->IsStopped()) {
      thd_->Stop();                                                       
      thd_->wait();                                                       
   }
   if(acquisitionType_.compare("Fluoro") == 0)
   {
	   result = vip_fluoro_record_stop();
	  
	   result = vip_fluoro_grabber_stop();
	    if(result != VIP_NO_ERR)
			result = vip_reset_state();
   }
   //
   if(result == VIP_NO_ERR)
		return DEVICE_OK;       
   else
	   return DEVICE_ERR;
} 

/**
* Simple implementation of Sequence Acquisition
* A sequence acquisition should run on its own thread and transport new images
* coming of the camera into the MMCore circular buffer.
*/
int VarianFPD::StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow)
{
	int ret = DEVICE_ERR;
     if (IsCapturing())
          return DEVICE_CAMERA_BUSY_ACQUIRING;
	 ret = GetCoreCallback()->PrepareForAcq(this);
	 if (ret != DEVICE_OK)
		 return ret;
      if(calibrationMode_.compare("Normal")==0)
	  {
		  if(acquisitionType_.compare("Fluoro") ==0)
		  {
			    ret = vip_fluoro_record_start(); 
		  }else if(acquisitionType_.compare("RAD") ==0)
		  {
			  //return DEVICE_ERR;		   
		  }
	  }
	if(ret != DEVICE_OK)
		return ret;
	sequenceStartTime_ = GetCurrentMMTime();
	imageCounter_ = 0;
   thd_->Start(numImages,interval_ms);
   stopOnOverflow_ = stopOnOverflow;
   return DEVICE_OK;
}

/*
 * Inserts Image and MetaData into MMCore circular Buffer
 */
int VarianFPD::InsertImage()
{
	MM::MMTime timeStamp = this->GetCurrentMMTime();
	char label[MM::MaxStrLength];
	this->GetLabel(label);

	// Important:  metadata about the image are generated here:
	Metadata md;
	// Copy the metadata inserted by other processes:
	std::vector<std::string> keys = metadata_.GetKeys();
	for (unsigned int i= 0; i < keys.size(); i++) {
		MetadataSingleTag mst = metadata_.GetSingleTag(keys[i].c_str());
		md.PutTag(mst.GetName(), mst.GetDevice(), mst.GetValue());
	}

	// Add our own metadata
	md.put("Camera", label);
	md.put(MM::g_Keyword_Metadata_StartTime, CDeviceUtils::ConvertToString(sequenceStartTime_.getMsec()));
	md.put(MM::g_Keyword_Elapsed_Time_ms, CDeviceUtils::ConvertToString((timeStamp - sequenceStartTime_).getMsec()));
	md.put(MM::g_Keyword_Metadata_ImageNumber, CDeviceUtils::ConvertToString(imageCounter_));
	md.put(MM::g_Keyword_Metadata_ROI_X, CDeviceUtils::ConvertToString( (long) roiX_)); 
	md.put(MM::g_Keyword_Metadata_ROI_Y, CDeviceUtils::ConvertToString( (long) roiY_)); 

	imageCounter_++;

	char buf[MM::MaxStrLength];
	GetProperty(MM::g_Keyword_Binning, buf);
	md.put(MM::g_Keyword_Binning, buf);

	MMThreadGuard g(imgPixelsLock_);

	const unsigned char* pI = GetImageBuffer();
	unsigned int w = GetImageWidth();
	unsigned int h = GetImageHeight();
	unsigned int b = GetImageBytesPerPixel();

	int ret = GetCoreCallback()->InsertImage(this, pI, w, h, b, md.Serialize().c_str());
	if (!stopOnOverflow_ && ret == DEVICE_BUFFER_OVERFLOW)
	{
		// do not stop on overflow - just reset the buffer
		GetCoreCallback()->ClearImageBuffer(this);
		// don't process this same image again...
		return GetCoreCallback()->InsertImage(this, pI, w, h, b, md.Serialize().c_str(), false);
	} else
		return ret;
}

/*
 * Do actual capturing
 * Called from inside the thread  
 */
 int VarianFPD::ThreadRun(void)
{
  static int lastCnt=-1;
  int ret=VIP_NO_ERR;
  if(calibrationMode_.compare("OffsetCalibration")==0)
  {
	  // wait for the calibration to complete
	  offset_calibration(crntModeSelect,8);

  }
  else if(calibrationMode_.compare("GainCalibration")==0)
  {
	  // wait for the calibration to complete
	  if(acquisitionType_.compare("Fluoro")==0)
		  gain_calibration(crntModeSelect,8);
	  else//RAD
		  rad_gain_calibration(crntModeSelect,8);

  }
  else
  {
	   if(acquisitionType_.compare("Fluoro")==0)
	   {
		   //fluoro_acqImage();
			for(int i=0;i<3;i++)
			{
					if(WaitForSingleObject(thd_->hFrameEvent, 1000) != WAIT_TIMEOUT)
					{
						int GNumFrames = GLiveParams->NumFrames;
						if(GNumFrames && GNumFrames != lastCnt)
						{
							WORD* imPtr = (WORD*)GLiveParams->BufPtr;
							//long offset= GLiveParams->BufIndex * bytesPerPixel_* imageWidth_ * imageHeight_;
							img_.SetPixels((const void *)(imPtr));//get_image();//FIXME use the imPtr pointor to get the image;
							lastCnt = GNumFrames;
							printf("frameNum:%d\n",GNumFrames);
							ret = DEVICE_OK;
							ret = InsertImage();
							break;
						}

					}
					else
						ret = DEVICE_ERR;
			}
		
	   }
	   else if(acquisitionType_.compare("RAD") ==0)
	   {
		   ret = SnapImage();
		   if (ret != DEVICE_OK)
		   {
			   return ret;
		   }
		   ret = InsertImage();
	   }
	   else//DEMO
	   {
		   GenerateSyntheticImage(img_,GetExposure());
		   ret = InsertImage();
		   if (ret != DEVICE_OK)
		   {
			   return ret;
		   }
	   }
  }
   return ret;
};



bool VarianFPD::IsCapturing() {
   return !thd_->IsStopped();
}


///////////////////////////////////////////////////////////////////////////////
// Private VarianFPD methods
///////////////////////////////////////////////////////////////////////////////

/**
* Sync internal image buffer size to the chosen property values.
*/
int VarianFPD::ResizeImageBuffer()
{
   img_.Resize(imageWidth_/binning_, imageHeight_/binning_, bytesPerPixel_);

   return DEVICE_OK;
}

/**
 * Generate an image with fixed value for all pixels
 */
void VarianFPD::GenerateImage()
{
   const int maxValue = (1 << maxBitDepth_) - 1; // max for the 12 bit camera
   unsigned char* pBuf = const_cast<unsigned char*>(img_.GetPixels());
   memset(pBuf, (unsigned int)100 , img_.Height()*img_.Width()*img_.Depth());
}
///////////////////////////////////////////////////////////////////////////////
// VarianFPD Action handlers
///////////////////////////////////////////////////////////////////////////////

/**
* Handles "AcquisitionType" property.
*/
int VarianFPD::OnAcquisitionType(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(acquisitionType_);

	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(acquisitionType_.c_str());
	}
	return DEVICE_OK;
}

/**
* Handles "Binning" property.
*/
int VarianFPD::OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		long binSize;
		pProp->Get(binSize);
		binning_ = (int)binSize;
		return ResizeImageBuffer();
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((long)binning_);
	}

	return DEVICE_OK;
}

/**
* Handles "PixelType" property.
*/
int VarianFPD::OnPixelType(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		string val;
		pProp->Get(val);
		if (val.compare(g_PixelType_8bit) == 0)
			bytesPerPixel_ = 1;
		else if (val.compare(g_PixelType_16bit) == 0)
			bytesPerPixel_ = 2;
		else
			assert(false);
		pixelType_ = val;
		return ResizeImageBuffer();
	}
	else if (eAct == MM::BeforeGet)
	{
		if (bytesPerPixel_ == 1)
			pProp->Set(g_PixelType_8bit);
		else if (bytesPerPixel_ == 2)
			pProp->Set(g_PixelType_16bit);
		else
			assert(false); // this should never happen
	}

	return DEVICE_OK;
}

/**
* Handles "Gain" property.
*/
int VarianFPD::OnGain(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(gain_);
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(gain_);
	}
	return DEVICE_OK;
}
int VarianFPD::OnFrameRate( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		double fr;
		pProp->Get(fr);
		if(crntModeSelect == -1 )
		{
			frameRate_ = (float)fr;
			exposure_ = 1000.0/fr;
			return DEVICE_OK;
		}
		else
		{
			if(vip_set_frame_rate(crntModeSelect,fr) == VIP_NO_ERR)
			{
				frameRate_ = (float)fr;
				return DEVICE_OK;
			}
			else
				return DEVICE_ERR;
		}
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((double)frameRate_);
	}
	return DEVICE_OK;
}
int VarianFPD::OnExposure( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		double fr;
		pProp->Get(fr);
		if(crntModeSelect == -1 )
		{
			exposure_ = (float)fr;
			return DEVICE_OK;
		}
		else
		{
			
			if(vip_set_frame_rate(crntModeSelect,1000.0/fr) == VIP_NO_ERR)
			{
				exposure_ = (float)fr;
				frameRate_ = 1000.0/(float)fr;
				return DEVICE_OK;
			}
			else
				return DEVICE_ERR;
		}

	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((double)exposure_);
	}
	return DEVICE_OK;
}
int VarianFPD::OnAnalogGain( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		double ag;
		pProp->Get(ag);
		analogGain_ = (float)ag;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((double)analogGain_);
	}
	return DEVICE_OK;
}
int VarianFPD::OnCalibrationMode( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		std::string cm;
		pProp->Get(cm);
		calibrationMode_ = cm;
					if(calibrationMode_.compare("OffsetCalibration")==0 || calibrationMode_.compare("GainCalibration")==0  )
		{
						//offset_calibration(crntModeSelect,8);
		  	imageWidth_ = 512;
			imageHeight_ =32;
			ResizeImageBuffer();
		}
		else
		{
			imageWidth_ = modeInfo.ColsPerFrame;
			imageHeight_ = modeInfo.LinesPerFrame;
			ResizeImageBuffer();
		}
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(calibrationMode_.c_str());
	}
	return DEVICE_OK;
}

int VarianFPD::OnAutoRectify( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		autoRectify_ = ar;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(autoRectify_.c_str());
	}
	return DEVICE_OK;
}
int VarianFPD::OnCalibrationProgress(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		//calibrationProgress_ = ar;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		calibrationProgress_ = getProgress();
		pProp->Set(calibrationProgress_.c_str());
		//if(calibrationProgress_.compare("Error")==0)
		//	return DEVICE_ERR; 
	}
	return DEVICE_OK;
}
int VarianFPD::OnOffsetCalibration(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		if(ar.compare("Yes")==0)
		{
			if(acquisitionType_.compare("RAD")==0)
			{
				if(rad_getReady4OffsetCal(crntModeSelect,frameNum4Cal_)!=DEVICE_OK)
					return DEVICE_ERR;
			}
			else
			{
				if(fluoro_getReady4OffsetCal(crntModeSelect,frameNum4Cal_)!=DEVICE_OK)
					return DEVICE_ERR;
			}
			OffsetCalibration_.assign("OK");
		}
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(OffsetCalibration_.c_str());
	}
	return DEVICE_OK;
}
int VarianFPD::OnGainCalibrationStep1(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		if(ar.compare("Yes")==0)
		{
			if(acquisitionType_.compare("RAD")==0)
			{
				if(rad_getReady4GainCal(crntModeSelect,frameNum4Cal_)!=DEVICE_OK)
					return DEVICE_ERR;
			}
			else
			{
				if(fluoro_getReady4GainCal(crntModeSelect,frameNum4Cal_)!=DEVICE_OK)
					return DEVICE_ERR;
			}
			gainCalibrationStep1_.assign("OK");
		}

		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(gainCalibrationStep1_.c_str());
	}
	return DEVICE_OK;
}
int VarianFPD::OnGainCalibrationStep2(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		if(ar.compare("Yes")==0)
		{
			if(acquisitionType_.compare("RAD")==0)
			{
				if(rad_startDarkFeild4GainCal()!=DEVICE_OK)
					return DEVICE_ERR;
			}
			else
			{
				if(fluoro_startFlatFeild4GainCal()!=DEVICE_OK)
					return DEVICE_ERR;
			}
			gainCalibrationStep2_.assign("OK");
		}
		
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(gainCalibrationStep2_.c_str());
	}
	return DEVICE_OK;
}
int VarianFPD::OnResetLink(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		if(ar.compare("Yes")==0)
		{
			if(vip_reset_state() == VIP_NO_ERR)
			{
				int r1=closeLink();
				int r2 = openLink();
				if(r1==DEVICE_OK && r2 ==DEVICE_OK)
				{
					resetLink_.assign("OK");
					return DEVICE_OK;
				}
				else
					resetLink_.assign("No");
			}
			else
				resetLink_.assign("No");
			return DEVICE_ERR;
		}
		else
			resetLink_.assign( "No");
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(resetLink_.c_str());
	}
	return DEVICE_OK;
}

int VarianFPD::OnResetState(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		if(ar.compare("Yes")==0)
		{
			if(vip_reset_state() == VIP_NO_ERR)
				resetState_.assign("OK");
			else
				resetState_.assign("No");
		}
		else
			resetState_.assign( "No");
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(resetState_.c_str());
	}
	return DEVICE_OK;
}
int VarianFPD::OnGainCalibrationStep3(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		if(ar.compare("Yes")==0)
		{
			if(acquisitionType_.compare("RAD")==0)
			{
				if(rad_startFlatFeild4GainCal()!=DEVICE_OK)
					return DEVICE_ERR;
			}
			else
			{
				if(fluoro_startDarkFeild4GainCal()!=DEVICE_OK)
					return DEVICE_ERR;
			}
			gainCalibrationStep3_.assign("OK");
		}
		
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(gainCalibrationStep3_.c_str());
	}
	return DEVICE_OK;
}
int VarianFPD::OnGainCalibrationStep4(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		if(ar.compare("Yes")==0)
		{
			if(acquisitionType_.compare("RAD")==0)
			{
				if(rad_endGainCal()!=DEVICE_OK)
					return DEVICE_ERR;
			}
			else
			{
				if(fluoro_endGainCal()!=DEVICE_OK)
					return DEVICE_ERR; 
			}
			gainCalibrationStep4_.assign("OK");
		}

		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(gainCalibrationStep4_.c_str());
	}
	return DEVICE_OK;
}
int VarianFPD::OnResolution( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(resolution_);
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(resolution_.c_str());
	}
	return DEVICE_OK;
}

int VarianFPD::OnMaxResolution( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(maxResolution_);
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(maxResolution_.c_str());
	}
	return DEVICE_OK;
}

int VarianFPD::OnNumModes( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		long nm;
		pProp->Get(nm);
		numModes_ = (int)nm;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((long)numModes_);
	}
	return DEVICE_OK;
}

int VarianFPD::OnReceptorType( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		long rt;
		pProp->Get(rt);
		receptorType_ = (int)rt;
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((long)receptorType_);
	}
	return DEVICE_OK;
}

int VarianFPD::OnMaxPixelValue( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		long mv;
		pProp->Get(mv);
		maxPixelValue_ = (int)mv;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((long)maxPixelValue_);
	}
	return DEVICE_OK;
}
int VarianFPD::OnFrameNum4Cal(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		long fn;
		pProp->Get(fn);
		frameNum4Cal_ = (int)fn;
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((long)frameNum4Cal_);
	}
	return DEVICE_OK;
}
int VarianFPD::OnFrameNum4Acq( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		long fn;
		pProp->Get(fn);
		if(vip_set_num_acq_frames(crntModeSelect,fn)==0)
		{
			frameNum4Acq_ = (int)fn;
		}
		else
			return DEVICE_ERR;
	}
	else if (eAct == MM::BeforeGet)
	{
		int fn;
		if(vip_get_num_acq_frames(crntModeSelect,&fn)==0)
		{
			frameNum4Acq_ = fn;
			pProp->Set((long)frameNum4Acq_);
		}
		else
			pProp->Set((long)-1);
		
	}
	return DEVICE_OK;
}
int VarianFPD::OnSystemDescription( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(systemDescription_);
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(systemDescription_.c_str());
	}
	return DEVICE_OK;
}

int VarianFPD::OnConfigurationPath( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(configurationPath_);
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(configurationPath_.c_str());
	}
	return(DEVICE_OK);
}
int VarianFPD::onTubeVoltage( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(tubeVoltage_);
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(tubeVoltage_.c_str());
	}
	return(DEVICE_OK);
}
int VarianFPD::onTubeCurrent( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(tubeCurrent_);
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(tubeCurrent_.c_str());
	}
	return(DEVICE_OK);
}
int VarianFPD::onOperator( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(operator_);
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(operator_.c_str());
	}
	return(DEVICE_OK);
}
int VarianFPD::onSID( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(sid_);
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(sid_.c_str());
	}
	return(DEVICE_OK);
}
int VarianFPD::onSOD( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(sod_);
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(sod_.c_str());
	}
	return(DEVICE_OK);
}
int VarianFPD::onTag( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(tag_);
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(tag_.c_str());
	}
	return(DEVICE_OK);
}
int VarianFPD::OnVirtualCPLink( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		int ret = checkLink();
		std::string vcpl;
		pProp->Get(vcpl);
		if(ret != DEVICE_OK)
		{
			virtualCPLink_.assign("OffLine");
			return DEVICE_NOT_CONNECTED;
		}
		if(vcpl.compare("OnLine") == 0 && ret != DEVICE_OK)
		{
			int ret = openLink();
			if(ret == DEVICE_OK )
			{
				virtualCPLink_.assign("OnLine");
			}
			else
				return DEVICE_NOT_CONNECTED;
		}
		else if(vcpl.compare("OffLine") == 0 &&  ret == DEVICE_OK)
		{
			int ret = closeLink();
			if(ret == DEVICE_OK)
				virtualCPLink_.assign("OffLine");
			else
				return DEVICE_ERR;
		}
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(virtualCPLink_.c_str());
	}
	return(DEVICE_OK);
}

int VarianFPD::OnMode( MM::PropertyBase* pProp, MM::ActionType eAct )
{
		if (eAct == MM::AfterSet)
		{
			if(IsCapturing())
				return DEVICE_CAMERA_BUSY_ACQUIRING;
			std::string md;
			pProp->Get(md);
			SetProperty("CalibrationMode","Normal");
			return switchMode(md);
		}
		else if (eAct == MM::BeforeGet)
		{
			pProp->Set(mode_.c_str());
		}
		return DEVICE_OK;
}

int VarianFPD::getSysInfo()
{
	int   result =	vip_get_sys_info(&sysInfo);
	if (result == HCP_NO_ERR)
	{
		//printf("  >> SysDescription=\"%s\"\n", sysInfo.SysDescription);
		//printf("  >> NumModes=         %5d,   DfltModeNum=   %5d\n",
		//	sysInfo.NumModes, sysInfo.DfltModeNum);
		//printf("  >> MxLinesPerFrame=  %5d,   MxColsPerFrame=%5d\n",
		//	sysInfo.MxLinesPerFrame, sysInfo.MxColsPerFrame);
		//printf("  >> MxPixelValue=     %5d,   HasVideo=      %5d\n",
		//	sysInfo.MxPixelValue, sysInfo.HasVideo);
		//printf("  >> StartUpConfig=    %5d,   NumAsics=      %5d\n",
		//	sysInfo.StartUpConfig, sysInfo.NumAsics);
		//printf("  >> ReceptorType=     %5d\n", sysInfo.ReceptorType);

		char buf[255];
		sprintf(buf,"sys:%s,Modes:%5d,MaxRes:%5d*%5d,MxPixelVal:%5d,startupCfg:%5d,ReceptorType:%5d",sysInfo.SysDescription,
			sysInfo.NumModes,sysInfo.MxLinesPerFrame, sysInfo.MxColsPerFrame,sysInfo.MxPixelValue,sysInfo.StartUpConfig,
			sysInfo.ReceptorType);
		systemDescription_.assign(buf);
		return(DEVICE_OK);
	}
	else
		return(DEVICE_ERR);
}
   int VarianFPD::OnOffsetCorrect(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		ofstVcpSetting_ = ar;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(ofstVcpSetting_.c_str());
	}
	return DEVICE_OK;
}
   int VarianFPD::OnGainCorrect(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		gainVcpSetting_ = ar;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(gainVcpSetting_.c_str());
	}
	return DEVICE_OK;
}
   int VarianFPD::OnDefectCorrect(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		dfctVcpSetting_ = ar;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(dfctVcpSetting_.c_str());
	}
	return DEVICE_OK;
}
   int VarianFPD::OnRotate90(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		rotaVcpSetting_ = ar;
		return SetCorrections();
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(rotaVcpSetting_.c_str());
	}
	return DEVICE_OK;
}
   int VarianFPD::OnFlipX(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		flpXVcpSetting_ = ar;
		return SetCorrections();
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(flpXVcpSetting_.c_str());
	}
	return DEVICE_OK;
}
   int VarianFPD::OnFlipY(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string ar;
		pProp->Get(ar);
		flpYVcpSetting_ = ar;
		return SetCorrections();
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(flpYVcpSetting_.c_str());
	}
	return DEVICE_OK;
}
int VarianFPD::openLink()
{
	int result = DEVICE_ERR;

	SOpenReceptorLink orl;
	memset(&orl, 0, sizeof(SOpenReceptorLink));
	orl.StructSize = sizeof(SOpenReceptorLink);
	strncpy(orl.RecDirPath, configurationPath_.c_str(), MAX_STR);

// if we want to turn debug on then uncomment the following line...
	orl.DebugMode = HCP_DBG_ON;
	printf("Opening link to %s\n", orl.RecDirPath);
	result = vip_open_receptor_link(&orl);

	if (result == HCP_OFST_ERR ||
		result == HCP_GAIN_ERR ||
		result == HCP_DFCT_ERR)
	{
		// this means not all correction files are available
		// here we will just turn corrections off but IN REAL APPLICATION
		// WE MUST BE SURE CORRECTIONS ARE ON AND THE RECEPTOR IS CALIBRATED
		result = SetCorrections();
		//if(result == VIP_NO_ERR) printf("\n\nCORRECTIONS ARE OFF!!");
	}
	if(result == VIP_NO_ERR)
		return DEVICE_OK;
	else
		return DEVICE_NOT_CONNECTED;
}

int VarianFPD::closeLink()
{
	int res = vip_reset_state();
	res = vip_close_link();
	if (res == HCP_NO_ERR)
		return DEVICE_OK;
	else 
		return DEVICE_ERR;
}

int VarianFPD::checkLink()
{
	SCheckLink clk;
	memset(&clk, 0, sizeof(SCheckLink));
	clk.StructSize = sizeof(SCheckLink);
	int result = vip_check_link(&clk);
	if(result == HCP_NO_ERR)
	{
		virtualCPLink_.assign("OnLine");
		return(DEVICE_OK);
	}
	else
	{
		virtualCPLink_.assign("OffLine");
		return DEVICE_NOT_CONNECTED;
	}
}

int VarianFPD::switchMode(std::string modeString)
{
	//vip_select_mode -> vip_get_correction_settings -> vip_get_mode_acq_type ->
	int N=0,result;
	int GImgSize=0;
	int acqType = -1;
	result =vip_reset_state();
	if(result!= HCP_NO_ERR)
		result = checkLink();

	if(modeString.compare("RAD")==0)//RAD
	{
		N =2 ;
		//result = vip_set_mode_acq_type(N,VIP_VALID_XRAYS_N_FRAMES,1);//TODO: Check return val
	}
	else if(modeString.compare("High-Res Fluoro")==0)
	{
		N =0;
	}
	else if(modeString.compare("Low-Res Fluoro")==0)
	{
		N = 1;
	}
	else
	{
		N = -1;
		crntModeSelect = -1;
		acquisitionType_.assign("Demo");
		return DEVICE_OK;
	}
    if(result!=VIP_NO_ERR)
		return DEVICE_ERR;//Error...
	result = vip_select_mode(N);
	if(result == VIP_NO_ERR)
	{
		// **** STEP 2 -- Get mode info --
		result = vip_get_mode_info(N, &modeInfo);
		if(result == VIP_NO_ERR)
		{
			if(modeInfo.ModeNum != N)
				return DEVICE_ERR;
			imageWidth_ = modeInfo.ColsPerFrame;
			imageHeight_ = modeInfo.LinesPerFrame;
			GImgSize = imageWidth_ * imageHeight_; //* sizeof(WORD); // image size in bytes
			ResizeImageBuffer();
			//frameRate_ = modeInfo.FrameRate;
			//int ret = SetPropertyLimits("FrameRate", LOWER_FRAME_RATE ,modeInfo.MxAllowedFrameRate);
			//assert(ret == DEVICE_OK);
			//ret = SetPropertyLimits("Exposure", 1000.0/modeInfo.MxAllowedFrameRate ,1000.0/LOWER_FRAME_RATE);
			result = SetProperty("FrameRate",CDeviceUtils::ConvertToString(modeInfo.FrameRate));
			//assert(result == DEVICE_OK);
			result = SetProperty("AcqCalibrationCount",CDeviceUtils::ConvertToString(modeInfo.CalFrmCount));
			assert(result == DEVICE_OK);
			result = SetProperty("AcqFrameCount",CDeviceUtils::ConvertToString(modeInfo.AcqFrmCount));
			assert(result == DEVICE_OK);
			result = SetProperty("ModeDescription",modeInfo.ModeDescription);
			assert(result == DEVICE_OK);
			acqType = modeInfo.AcqType & VIP_ACQ_MASK;
			char buf[20];
			sprintf(buf,"%d*%d",imageWidth_,imageHeight_);

			resolution_= buf ;

			if(acqType==VIP_ACQ_TYPE_CONTINUOUS)
				acquisitionType_.assign("Fluoro");
			else
				acquisitionType_.assign("RAD");
			crntModeSelect =N;
			mode_ = modeString;
		}
	}
	switch(N)
	{
		case 2:
			result = rad_init();
			break;
		case 0:
			result = fluoro_init(0,10);//TODO: Check return val
			fluoro_acqImage();
			break;
		case 1:
			result = fluoro_init(0,10);//TODO: Check return val
			fluoro_acqImage();
			break;
		default:
			acquisitionType_.assign("Demo");
			result = DEVICE_OK;
	}
	return result;
}
int VarianFPD::rad_init()
{
	int result =SetCorrections();
	result =GetCorrections();
	if (result == HCP_NO_ERR)
	{
		int modeType=-1, acqNum=-1;
		//vip_set_num_acq_frames(crntModeSelect,numFrames);
		result = vip_get_mode_acq_type(crntModeSelect, &modeType, &acqNum);
		if(result == HCP_NO_ERR)
		{
			if(modeType != VIP_VALID_XRAYS_N_FRAMES || acqNum <= 0)
			{
				printf("*** Problem with mode settings; modeType=%d; acqNum=%d",
					modeType, acqNum);
				result = HCP_OTHER_ERR;
			}
		}
	}
	//result = CheckRecLink();
	if (result == HCP_NO_ERR)
	{
		result = vip_enable_sw_handshaking(TRUE);
	}
	if (result == HCP_NO_ERR)
	{
		result = vip_set_frame_rate(crntModeSelect, frameRate_ ); // 10 second integration
	}
	
	if (result ==HCP_NO_ERR)
		return DEVICE_OK;
	else
		return DEVICE_ERR;
}
int VarianFPD::rad_acqImage()
{
	//ret =checkLink();
	//if(ret == DEVICE_OK)
	//if(ret == HCP_NO_ERR)
	//{

		int result = vip_sw_handshaking(VIP_SW_PREPARE, TRUE);
		if (result == HCP_NO_ERR)
		{
			crntStatus.qpi.ReadyForPulse = FALSE;
			int runCount=0;
			while (result == HCP_NO_ERR)
			{
				result =checkLink();
				if(result ==DEVICE_ERR)
					return DEVICE_ERR;
				memset(&crntStatus, 0, sizeof(SQueryProgInfo));
				crntStatus.qpi.StructSize = sizeof(SQueryProgInfo);
				result = vip_query_prog_info(HCP_U_QPI, &crntStatus);
				if(crntStatus.qpi.ReadyForPulse == TRUE) break;
				Sleep(5);
				
				if(runCount++>100)
					return DEVICE_ERR;
			}
		}	
		if (result == HCP_NO_ERR)
		{
			result = vip_sw_handshaking(VIP_SW_VALID_XRAYS, TRUE);
		}
		if (result == HCP_NO_ERR)
		{

			crntStatus.qpi.Complete = FALSE;
			printf("Waiting for Complete == TRUE...\n");
			int runCount=0;
			while (result == HCP_NO_ERR)
			{
				//result =checkLink();
				//if(result !=DEVICE_OK)
				//	return DEVICE_ERR;
				memset(&crntStatus, 0, sizeof(SQueryProgInfo));
				crntStatus.qpi.StructSize = sizeof(SQueryProgInfo);
				int result = vip_query_prog_info(HCP_U_QPI, &crntStatus);
				if(crntStatus.qpi.Complete == TRUE) break;
				Sleep(30);
				
				if(runCount++>100)
					return DEVICE_ERR;
			}
		}
		if (result == HCP_NO_ERR)
		{
			result = vip_sw_handshaking(VIP_SW_PREPARE, FALSE);
		}

		if (result == HCP_NO_ERR)
		{
			get_image();
		}
		// no need to do this if not using Hw handshaking at all
		//vip_enable_sw_handshaking(FALSE);
		if(result == HCP_NO_ERR)
			return DEVICE_OK;
		else
			return DEVICE_ERR;
	//}
	//else
	//	return DEVICE_ERR;

}
int VarianFPD::fluoro_acqImage()
{
	int	ret = DEVICE_ERR;
	//if(ret ==DEVICE_OK)
	//{
		ret = vip_fluoro_record_start();//TODO:maybe we can put it to the fluoro initial 
		if( thd_->hFrameEvent && GLiveParams)
		{
			int lastCnt=-1;
			for(int i=0;i<3;i++)
			{
					if(WaitForSingleObject(thd_->hFrameEvent, 1000) != WAIT_TIMEOUT)
					{
						int GNumFrames = GLiveParams->NumFrames-1;
						if(GNumFrames>1 && GNumFrames != lastCnt)
						{

							//int offset= bufIndex * bytesPerPixel_* imageWidth_ * imageHeight_;
							img_.SetPixels((const void *)GLiveParams->BufPtr);//get_image();//FIXME use the imPtr pointor to get the image;
							lastCnt = GNumFrames;
							ret = DEVICE_OK;
							//ret = InsertImage();
							break;
						}

					}
					else
						ret = DEVICE_ERR;
			}

		}
		ret = vip_fluoro_record_stop();
		if(ret == HCP_NO_ERR)
			return DEVICE_OK;
		else
			return DEVICE_ERR;
	//}
	//else
	//	return DEVICE_ERR;
}
int VarianFPD::fluoro_init(int numFrm ,int numBuf )
{	int acqType=-1;
	int result = DEVICE_ERR;
	///////////////////////////////////////////////////////////////////////////
 //   // check the receptor link is OK
	//result = checkLink();
	///////////////////////////////////////////////////////////////////////////
	//if(result == DEVICE_OK)
	//{
		int i=0;
		SSeqPrms seqPrms;
		memset(&seqPrms, 0, sizeof(SSeqPrms));
		seqPrms.StructSize = sizeof(SSeqPrms);
		seqPrms.NumBuffers = numBuf;
		seqPrms.StopAfterN = numFrm; // zero is interpreted as acquire continuous
		// (writing to buffers in circular fashion)
		result = vip_fluoro_set_prms(HCP_FLU_SEQ_PRMS, &seqPrms);
		numBuf = seqPrms.NumBuffers;
		if(numFrm > numBuf) numFrm = numBuf;
		//////////////////////////////////////////////////////////////////////////
		if(result == VIP_NO_ERR)
		{
			SAcqPrms acqPrms;
			memset(&acqPrms, 0, sizeof(SAcqPrms));
			acqPrms.StructSize = sizeof(SAcqPrms);
			acqPrms.MarkPixels = 0;
			acqPrms.StartUp = 0;
			acqPrms.CorrType = HCP_CORR_STD; // This will be the normal setting. 
			acqPrms.CorrFuncPtr = NULL; // This should essentially always be set to NULL. 
			acqPrms.ReqType = 0;		// ReqType is for internal use only. 
			result =GetCorrections();
			if(result == HCP_NO_ERR)
			{
				result = vip_fluoro_grabber_start(&acqPrms);
				GLiveParams = (SLivePrms*)acqPrms.LivePrmsPtr;	

			}
			if(result == HCP_NO_ERR)
			{
				result = vip_fluoro_get_event_name(HCP_FG_FRM_TO_DISP, GFrameReadyName);
			}
			thd_->hFrameEvent = CreateEvent(NULL, FALSE, FALSE, GFrameReadyName);
		}
		if(result == VIP_NO_ERR)
			return DEVICE_OK;
		else
			return DEVICE_ERR;
	//}
	//else
	//	return DEVICE_ERR;
}
/**
* Generate a spatial sine wave.
*/
void VarianFPD::GenerateSyntheticImage(ImgBuffer& img, double exp)
{ 
	exp = 100;
	// constants for naming pixel types (allowed values of the "PixelType" property)
	const char* g_PixelType_16bit = "16bit";
	double fractionOfPixelsToDropOrSaturate_=0.002;
	bool dropPixels_=false;
	bool saturatePixels_=false;
	MMThreadGuard g(imgPixelsLock_);
	double g_IntensityFactor_ = 1.0;
	int bitDepth_=16;
	//std::string pixelType;
	char buf[MM::MaxStrLength];
	GetProperty(MM::g_Keyword_PixelType, buf);
	std::string pixelType(buf);
	static double dPhase_=0;
	if (img.Height() == 0 || img.Width() == 0 || img.Depth() == 0)
		return;

	const double cPi = 3.14159265358979;
	long lPeriod = img.Width()/2;
	double dLinePhase = 0.0;
	const double dAmp = exp;
	const double cLinePhaseInc = 2.0 * cPi / 4.0 / img.Height();

	static bool debugRGB = false;
#ifdef TIFFDEMO
	debugRGB = true;
#endif
	static  unsigned char* pDebug  = NULL;
	static unsigned long dbgBufferSize = 0;
	static long iseq = 1;
	// for integer images: bitDepth_ is 8, 10, 12, 16 i.e. it is depth per component
	long maxValue = (1L << bitDepth_)-1;

	long pixelsToDrop = 0;
	if( dropPixels_)
		pixelsToDrop = (long)(0.5 + fractionOfPixelsToDropOrSaturate_*img.Height()*img.Width());
	long pixelsToSaturate = 0;
	if( saturatePixels_)
		pixelsToSaturate = (long)(0.5 + fractionOfPixelsToDropOrSaturate_*img.Height()*img.Width());

	 unsigned j, k;
	 if (pixelType.compare(g_PixelType_16bit) == 0)
	{
		double pedestal = maxValue/2 * exp / 100.0 * GetBinning() * GetBinning();
		double dAmp16 = dAmp * maxValue/255.0; // scale to behave like 8-bit
		unsigned short* pBuf = (unsigned short*) const_cast<unsigned char*>(img.GetPixels());
		for (j=0; j<img.Height(); j++)
		{
			for (k=0; k<img.Width(); k++)
			{
				long lIndex = img.Width()*j + k;
				*(pBuf + lIndex) = (unsigned short) (g_IntensityFactor_ * min((double)maxValue, pedestal + dAmp16 * sin(dPhase_ + dLinePhase + (2.0 * cPi * k) / lPeriod)));
			}
			dLinePhase += cLinePhaseInc;
		}         
		for(int snoise = 0; snoise < pixelsToSaturate; ++snoise)
		{
			j = (unsigned)(0.5 + (double)img.Height()*(double)rand()/(double)RAND_MAX);
			k = (unsigned)(0.5 + (double)img.Width()*(double)rand()/(double)RAND_MAX);
			*(pBuf + img.Width()*j + k) = (unsigned short)maxValue;
		}
		int pnoise;
		for(pnoise = 0; pnoise < pixelsToDrop; ++pnoise)
		{
			j = (unsigned)(0.5 + (double)img.Height()*(double)rand()/(double)RAND_MAX);
			k = (unsigned)(0.5 + (double)img.Width()*(double)rand()/(double)RAND_MAX);
			*(pBuf + img.Width()*j + k) = 0;
		}

	}

	dPhase_ += cPi / 4.;
}
int VarianFPD::get_image()
{
	int x_size = modeInfo.ColsPerFrame;
	int y_size = modeInfo.LinesPerFrame;
	int npixels = x_size * y_size;

	//USHORT *image_ptr = (USHORT *)malloc(npixels * sizeof(USHORT));
	USHORT* image_ptr =(USHORT*)const_cast<unsigned char*>(img_.GetPixels()); //FIXME may be some error in the convertion
	int result = vip_get_image(crntModeSelect, VIP_CURRENT_IMAGE, x_size, y_size, image_ptr);
	return result;
}
int VarianFPD::fluoro_getReady4GainCal(int mode,int numCalFrmSet)
{
	int result = DEVICE_ERR;
	result=vip_reset_state();
	if(result == VIP_NO_ERR)
	{
		if(numCalFrmSet<0)
			 result = vip_get_num_cal_frames(mode, &numCalFrmSet);
		else
			 result = vip_set_num_cal_frames(mode, numCalFrmSet);
	}
	frameNum4Cal_ = numCalFrmSet;
	// tell the system to prepare for a gain calibration
	if(result == VIP_NO_ERR)
	{
		result = vip_gain_cal_prepare(mode);
	}
	else
		return DEVICE_ERR;
	// send prepare = true
	if(result == VIP_NO_ERR)
	{
		result = vip_sw_handshaking(VIP_SW_PREPARE, TRUE);
	}
	else
	return DEVICE_ERR;
	memset(&qpi, 0, sizeof(SQueryProgInfo));
	qpi.StructSize = sizeof(SQueryProgInfo);
	uqpi = (UQueryProgInfo*)&qpi;

	// wait for readyForPulse
	while(!qpi.ReadyForPulse && result == VIP_NO_ERR)
	{
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		Sleep(100);
	}
	fflush(stdin);

	return result;
}
int VarianFPD::fluoro_startFlatFeild4GainCal()
{
	int result = DEVICE_ERR;
	printf("\n**Hit any key when x-rays are ON and ready for flat-field");
	//while(!_kbhit()) Sleep (100);

	// send xrays = true - this signals the START of the FLAT-FIELD ACQUISITION
	result = vip_sw_handshaking(VIP_SW_VALID_XRAYS, TRUE);
	qpi.NumFrames = 0;
	return result;
}
int VarianFPD::fluoro_startDarkFeild4GainCal()
{
	int result = DEVICE_ERR;
	printf("\n**Hit any key when x-rays are OFF and ready for dark-field");
	//_getch();
	//while(!_kbhit()) Sleep (100);
	Sleep (100);
	// wait for readyForPulse
	while(!qpi.ReadyForPulse && result == VIP_NO_ERR)
	{
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		Sleep(100);
	}

	// send xrays = false - this signals the START of the DARK-FIELD ACQUISITION
	//if(result == VIP_NO_ERR)
	{
		result = vip_sw_handshaking(VIP_SW_VALID_XRAYS, 0);
	}
	// wait for the calibration to complete
	qpi.Complete = FALSE;
	//while (!qpi.Complete && result == VIP_NO_ERR)
	//{
	//	result = vip_query_prog_info(HCP_U_QPI, uqpi);
	//	if(qpi.NumFrames < frameNum4Cal_)
	//	{
	//		printf("\nDark-field frames accumulated = %d", qpi.NumFrames);
	//	}
	//	Sleep(100);
	//}
	return result;
}
int  VarianFPD::fluoro_endGainCal()
{
	int result = DEVICE_ERR;
	//printf("Setting PREPARE=0\n");
	//try{
	//result = vip_sw_handshaking(VIP_SW_PREPARE, 0);
	//}
	//catch (...){
	//	vip_reset_state();
	//}
	return DEVICE_OK;
}
std::string VarianFPD::getProgress()
{
	std::string retStr;
	memset(&crntStatus, 0, sizeof(SQueryProgInfo));
	crntStatus.qpi.StructSize = sizeof(SQueryProgInfo);
	int result = vip_query_prog_info(HCP_U_QPI, &crntStatus);
	char buf[255];
	if (result == HCP_NO_ERR)
	{
		sprintf(buf,"%d,%d,%d,%d",//"f=%d c=%d p=%d r=%d",//"frames=%d complete=%d pulses=%d ready=%d",
		crntStatus.qpi.NumFrames,
		crntStatus.qpi.Complete,
		crntStatus.qpi.NumPulses,
		crntStatus.qpi.ReadyForPulse);

		prevStatus.qpi = crntStatus.qpi;
		retStr.assign(buf);
		printf("%s\n",buf);
	}
	else
		retStr.assign("Error");
	return retStr;
}
int VarianFPD::gain_calibration(int mode,int numCalFrmSet)
{
	int result=vip_reset_state();
		if(result == VIP_NO_ERR)
	{
		if(numCalFrmSet<0)
			 result = vip_get_num_cal_frames(mode, &numCalFrmSet);
		else
			 result = vip_set_num_cal_frames(mode, numCalFrmSet);
	}
	// tell the system to prepare for a gain calibration
	if(result == VIP_NO_ERR)
	{
		result = vip_gain_cal_prepare(mode);
	}

	// send prepare = true
	if(result == VIP_NO_ERR)
	{
		result = vip_sw_handshaking(VIP_SW_PREPARE, TRUE);
	}

	SQueryProgInfo qpi;
	memset(&qpi, 0, sizeof(SQueryProgInfo));
	qpi.StructSize = sizeof(SQueryProgInfo);
	UQueryProgInfo* uqpi = (UQueryProgInfo*)&qpi;

	// wait for readyForPulse
	while(!qpi.ReadyForPulse && result == VIP_NO_ERR)
	{
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		Sleep(100);
	}
	fflush(stdin);
	printf("\n**Hit any key when x-rays are ON and ready for flat-field");
	//while(!_kbhit()) Sleep (100);

	// send xrays = true - this signals the START of the FLAT-FIELD ACQUISITION
	if(result == VIP_NO_ERR)
	{
		result = vip_sw_handshaking(VIP_SW_VALID_XRAYS, TRUE);
	}

	qpi.NumFrames = 0;
	int maxFrms=0;
	while(qpi.NumFrames < numCalFrmSet && result == VIP_NO_ERR)
	{
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		printf("\nFlat-field frames accumulated = %d", qpi.NumFrames);
		GenerateSyntheticImage(img_,10);
		result = InsertImage();
		Sleep(100);

		// just in case the number of frames resets to zero before we see the 
		// limit reached
		if(maxFrms > qpi.NumFrames) break;
		maxFrms = qpi.NumFrames;
	}
	
	printf("\n**Hit any key when x-rays are OFF and ready for dark-field");
	//_getch();
	//while(!_kbhit()) Sleep (100);

	// wait for readyForPulse
	while(!qpi.ReadyForPulse && result == VIP_NO_ERR)
	{
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		Sleep(100);
	}

	// send xrays = false - this signals the START of the DARK-FIELD ACQUISITION
	if(result == VIP_NO_ERR)
	{
		result = vip_sw_handshaking(VIP_SW_VALID_XRAYS, 0);
	}

	// wait for the calibration to complete
	qpi.Complete = FALSE;
	while (!qpi.Complete && result == VIP_NO_ERR)
	{
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		if(qpi.NumFrames < numCalFrmSet)
		{
			printf("\nDark-field frames accumulated = %d", qpi.NumFrames);
			GenerateSyntheticImage(img_,10);
			result = InsertImage();
		}
		Sleep(100);
	}

	if(result == VIP_NO_ERR)
	{
		printf("\n\nGain calibration completed successfully");
		StopSequenceAcquisition();
	}
	else
	{
		printf("\n\nError in gain calibration");
	}
	if(result == VIP_NO_ERR)
	{
		return DEVICE_OK;
	}
	else
	{
		return DEVICE_ERR;
	}
}
int VarianFPD::fluoro_getReady4OffsetCal(int mode,int numCalFrmSet)
{
	int result = DEVICE_ERR;
	fflush(stdin);
	printf("\n**Hit any key when x-rays are OFF and ready for offset cal");
	//while(!_kbhit()) Sleep (100);
	int ret=vip_reset_state();
	// find how many calibration frames we have set
	//int numCalFrmSet=0;

	if(numCalFrmSet<0)
		 result = vip_get_num_cal_frames(mode, &numCalFrmSet);
	else
		 result = vip_set_num_cal_frames(mode, numCalFrmSet);

	frameNum4Cal_ = numCalFrmSet;

	if(result == VIP_NO_ERR)
	{
		// start the offset cal
		result = vip_offset_cal(mode);
	}
	memset(&qpi, 0, sizeof(SQueryProgInfo));
	qpi.StructSize = sizeof(SQueryProgInfo);
	uqpi = (UQueryProgInfo*)&qpi;
	return ret;
}
int VarianFPD::offset_calibration(int mode,int numCalFrmSet)
{
	int result = DEVICE_ERR;
	fflush(stdin);
	printf("\n**Hit any key when x-rays are OFF and ready for offset cal");
	//while(!_kbhit()) Sleep (100);
	int ret=vip_reset_state();
	// find how many calibration frames we have set
	//int numCalFrmSet=0;
	if(numCalFrmSet<0)
		 result = vip_get_num_cal_frames(mode, &numCalFrmSet);
	else
		 result = vip_set_num_cal_frames(mode, numCalFrmSet);

	if(result == VIP_NO_ERR)
	{
		// start the offset cal
		result = vip_offset_cal(mode);
	}

	SQueryProgInfo qpi;
	memset(&qpi, 0, sizeof(SQueryProgInfo));
	qpi.StructSize = sizeof(SQueryProgInfo);
	UQueryProgInfo* uqpi = (UQueryProgInfo*)&qpi;

	// wait for the calibration to complete
	while (!qpi.Complete && result == VIP_NO_ERR)
	{
		int lastNum = 0;
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		if(qpi.NumFrames <= numCalFrmSet)
		{
			if(lastNum != qpi.NumFrames)
			{
				printf("\nOffset frames accumulated = %d; Complete = %d",
						qpi.NumFrames, qpi.Complete);
				lastNum = qpi.NumFrames;
				GenerateSyntheticImage(img_,10);
				ret = InsertImage();
			}
		}
		Sleep(100);
	}

	if(result == VIP_NO_ERR)
	{
		printf("\n\nOffset calibration completed successfully");
		StopSequenceAcquisition();
	}
	else
	{
		printf("\n\nError in offset calibration");
	}
	return result;
}
int VarianFPD::queryProgress()
{
	bool showAll = false;
	memset(&crntStatus, 0, sizeof(SQueryProgInfo));
	crntStatus.qpi.StructSize = sizeof(SQueryProgInfo);
	int result = vip_query_prog_info(HCP_U_QPI, &crntStatus);

	if (result == HCP_NO_ERR)
	{
		if (showAll
		|| (prevStatus.qpi.NumFrames != crntStatus.qpi.NumFrames)
		|| (prevStatus.qpi.Complete != crntStatus.qpi.Complete)
		|| (prevStatus.qpi.NumPulses != crntStatus.qpi.NumPulses)
		|| (prevStatus.qpi.ReadyForPulse != crntStatus.qpi.ReadyForPulse))
		{
			printf("vip_query_prog_info: frames=%d complete=%d pulses=%d ready=%d\n",
				crntStatus.qpi.NumFrames,
				crntStatus.qpi.Complete,
				crntStatus.qpi.NumPulses,
				crntStatus.qpi.ReadyForPulse);

			prevStatus.qpi = crntStatus.qpi;
		}
	}
	else
		printf("**** vip_query_prog_info returns error %d\n", result);

	return result;
}
int  VarianFPD::rad_getReady4OffsetCal(int mode,int numCalFrmSet)
{
	return fluoro_getReady4OffsetCal(mode,numCalFrmSet);
}
int  VarianFPD::rad_getReady4GainCal(int mode,int numCalFrmSet)
{
	printf("Gain calibration\n");
	int  result = vip_reset_state();

// NOTE: for simplicity some error checking left out in the following

	if(result == VIP_NO_ERR)
	{
		if(numCalFrmSet<0)
			 result = vip_get_num_cal_frames(mode, &numCalFrmSet);
		else
			 result = vip_set_num_cal_frames(mode, numCalFrmSet);
	}
	if(result == VIP_NO_ERR)
	{
		result = vip_enable_sw_handshaking(TRUE); //TODO:oeway added, not test yet
	}
	if(result == VIP_NO_ERR)
	{
		result = vip_gain_cal_prepare(crntModeSelect, false);
	}

	if (result != HCP_NO_ERR)
	{
		printf("*** vip_gain_cal_prepare returns error %d\n", result);

	}



	return result;
}
int  VarianFPD::rad_startDarkFeild4GainCal()
{
	int result = DEVICE_ERR;
	result = vip_sw_handshaking(VIP_SW_PREPARE, 1);
	if (result != HCP_NO_ERR)
	{
		printf("*** vip_sw_handshaking(VIP_SW_PREPARE, 1) returns error %d\n", result);
		return result;
	}
	printf("Performing initial offset calibration prior to gain calibration\n");
	printf("Press Esc to exit without modifying calibration files\n");

	crntStatus.qpi.ReadyForPulse = FALSE;

	//while(!crntStatus.qpi.ReadyForPulse)
	//{
	//	result = queryProgress();
	//	if(result != HCP_NO_ERR) 
	//	return result;
	//	if(crntStatus.qpi.ReadyForPulse) break;
	//	SleepEx(100, FALSE);
	//}


	return result;
}

int  VarianFPD::rad_startFlatFeild4GainCal()
{	int result = DEVICE_ERR;
		vip_enable_sw_handshaking(TRUE);
		crntStatus.qpi.ReadyForPulse = FALSE;
		while(!crntStatus.qpi.ReadyForPulse)
		{
			result = queryProgress();
			if(result != HCP_NO_ERR) return result;
			if(crntStatus.qpi.ReadyForPulse) break;
			SleepEx(100, FALSE);
		}
		printf("Ready for X-rays, EXPOSE NOW\n");
		result = vip_sw_handshaking(VIP_SW_VALID_XRAYS, TRUE);
		Sleep(300);
		return result;
}
int  VarianFPD::rad_endGainCal()
{
		printf("Setting PREPARE=0\n");
	int result = vip_sw_handshaking(VIP_SW_PREPARE, 0);
	int i=0;
	while (!crntStatus.qpi.Complete && i<30)
	{
		SleepEx(500, FALSE);
		queryProgress();
		i++;
	}
	return result;
}
int  VarianFPD::rad_gain_calibration(int mode,int numCalFrmSet)
{
	printf("Gain calibration\n");
	int  result = vip_reset_state();

// NOTE: for simplicity some error checking left out in the following
	int numOfstCal=2;
	vip_get_num_cal_frames(crntModeSelect, &numOfstCal);

	result = vip_gain_cal_prepare(crntModeSelect, false);
	if (result != HCP_NO_ERR)
	{
		printf("*** vip_gain_cal_prepare returns error %d\n", result);
		return result;
	}
	result = vip_sw_handshaking(VIP_SW_PREPARE, 1);
	if (result != HCP_NO_ERR)
	{
		printf("*** vip_sw_handshaking(VIP_SW_PREPARE, 1) returns error %d\n", result);
		return result;
	}

	printf("Performing initial offset calibration prior to gain calibration\n");
	printf("Press Esc to exit without modifying calibration files\n");

	do
	{
		result = queryProgress();
		if (result != HCP_NO_ERR)
			return result;

		SleepEx(50, FALSE);
		//if (_kbhit())
		//{
		//	printf("Key pressed - exiting calibration\n");
		//	vip_reset_state();
		//	return -1;
		//}
	} while (crntStatus.qpi.NumFrames < numOfstCal);
	printf("Initial offset calibration complete\n\n");

	int numPulses=0;
	
	vip_enable_sw_handshaking(TRUE);

	printf("\nPress any key to begin flat field acquisition\n");	
	//while(!_kbhit()) Sleep (100);

	while (1)
	{
		crntStatus.qpi.ReadyForPulse = FALSE;

		while(!crntStatus.qpi.ReadyForPulse)
		{
			result = queryProgress();
			if(result != HCP_NO_ERR) 
			return result;
			if(crntStatus.qpi.ReadyForPulse) break;
			SleepEx(100, FALSE);
		}


		printf("Ready for X-rays, EXPOSE NOW\n");
		vip_sw_handshaking(VIP_SW_VALID_XRAYS, TRUE);
		Sleep(300);

		crntStatus.qpi.NumPulses = numPulses;
		while(crntStatus.qpi.NumPulses == numPulses)
		{
			result = queryProgress();
			if(result != HCP_NO_ERR) 
			return result;
			if(crntStatus.qpi.NumPulses != numPulses) break;
			SleepEx(100, FALSE);
		}
		numPulses = crntStatus.qpi.NumPulses;

		printf("Number of pulses=%d", numPulses);
		if(numPulses == numCalFrmSet) break;
	}
	
	printf("Setting PREPARE=0\n");
	result = vip_sw_handshaking(VIP_SW_PREPARE, 0);

	while (!crntStatus.qpi.Complete)
	{
		SleepEx(500, FALSE);
		queryProgress();
		//if (_kbhit())
		{
			//keyCode = _getch();
			//if (keyCode == ESC_KEY)
			//{
			//	printf("Esc key pressed - forcing exit from calibration\n");
			//	vip_reset_state();
			//	return -1;
			//}
		}
	}

	printf("Gain calibration complete\n");
	StopSequenceAcquisition();
	// no need to do this if not using Hw handshaking at all
	//vip_enable_sw_handshaking(FALSE);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//  SetCorrections
//////////////////////////////////////////////////////////////////////////////
int	 VarianFPD::SetCorrections()
{
	// set up the SCorrections structure
	SCorrections corr;
	memset(&corr, 0, sizeof(SCorrections));
	corr.StructSize = sizeof(SCorrections);
	

		corr.Ofst = (ofstVcpSetting_.compare("Enable")==0);
		corr.Gain = (gainVcpSetting_.compare("Enable")==0);
		corr.Dfct = (dfctVcpSetting_.compare("Enable")==0);
		corr.Rotate90 = (rotaVcpSetting_.compare("Enable")==0);
		corr.FlipX    = (flpXVcpSetting_.compare("Enable")==0);
		corr.FlipY    = (flpYVcpSetting_.compare("Enable")==0);


	return(vip_set_correction_settings(&corr));
}
int	 VarianFPD::GetCorrections()
{

	SCorrections corr;
	memset(&corr, 0, sizeof(SCorrections));
	corr.StructSize = sizeof(SCorrections);
	int result = vip_get_correction_settings(&corr);
	if(result== HCP_NO_ERR)
	{
		if(corr.Ofst) ofstVcpSetting_.assign("Enable"); else ofstVcpSetting_.assign("Disable"); 
		if(corr.Gain) gainVcpSetting_.assign("Enable"); else gainVcpSetting_.assign("Disable"); 
		if(corr.Dfct) dfctVcpSetting_.assign("Enable"); else dfctVcpSetting_.assign("Disable"); 
		if(corr.Rotate90) rotaVcpSetting_.assign("Enable"); else rotaVcpSetting_.assign("Disable"); 
		if(corr.FlipX)  flpXVcpSetting_.assign("Enable"); else flpXVcpSetting_.assign("Disable"); 
		if(corr.FlipY)  flpYVcpSetting_.assign("Enable"); else flpYVcpSetting_.assign("Disable"); 
			
	}
	return result;
}
