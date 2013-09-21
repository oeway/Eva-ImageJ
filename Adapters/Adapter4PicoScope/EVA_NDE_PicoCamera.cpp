///////////////////////////////////////////////////////////////////////////////
// FILE:          EVA_NDE_PicoCamera.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   The example implementation of the EVA_NDE_Pico camera.
//                Simulates generic digital camera and associated automated
//                microscope devices and enables testing of the rest of the
//                system without the need to connect to the actual hardware. 
//                
// AUTHOR:        Nenad Amodaj, nenad@amodaj.com, 06/08/2005
//
// COPYRIGHT:     University of California, San Francisco, 2006
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
// CVS:           $Id: EVA_NDE_PicoCamera.cpp 10842 2013-04-24 01:21:05Z mark $
//

#include "EVA_NDE_PicoCamera.h"
#include <cstdio>
#include <string>
#include <math.h>
#include "../../MMDevice/ModuleInterface.h"
#include "../../MMCore/Error.h"
#include <sstream>
#include <algorithm>
#include "WriteCompactTiffRGB.h"
#include <iostream>
#include "pico\PS3000Acon.c"
#define MAX_SAMPLE_LENGTH 3000000
#define PICO_RUM_TIME_OUT 5000
#define WAIT_FOR_TRIGGER_FAIL_COUNT 2
using namespace std;
const double CEVA_NDE_PicoCamera::nominalPixelSizeUm_ = 1.0;
double g_IntensityFactor_ = 1.0;

// External names used used by the rest of the system
// to load particular device from the "EVA_NDE_PicoCamera.dll" library
const char* g_CameraDeviceName = "PicoCam";
const char* g_HubDeviceName = "PicoHub";
const char* g_DADeviceName = "PicoScopeGen";

// constants for naming pixel types (allowed values of the "PixelType" property)
const char* g_PixelType_8bit = "8bit";
const char* g_PixelType_16bit = "16bit";

// TODO: linux entry code

// windows DLL entry code
#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE /*hModule*/, 
                      DWORD  ul_reason_for_call, 
                      LPVOID /*lpReserved*/
                      )
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
 * List all suppoerted hardware devices here
 * Do not discover devices at runtime.  To avoid warnings about missing DLLs, Micro-Manager
 * maintains a list of supported device (MMDeviceList.txt).  This list is generated using 
 * information supplied by this function, so runtime discovery will create problems.
 */
MODULE_API void InitializeModuleData()
{
   AddAvailableDeviceName(g_CameraDeviceName, "EVA_NDE_Pico camera");
   AddAvailableDeviceName("TransposeProcessor", "TransposeProcessor");
   AddAvailableDeviceName("ImageFlipX", "ImageFlipX");
   AddAvailableDeviceName("ImageFlipY", "ImageFlipY");
   AddAvailableDeviceName("MedianFilter", "MedianFilter");
   AddAvailableDeviceName(g_DADeviceName, "EVA_NDE_Pico DA");
   AddAvailableDeviceName(g_HubDeviceName, "DHub");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return 0;

   // decide which device class to create based on the deviceName parameter
   if (strcmp(deviceName, g_CameraDeviceName) == 0)
   {
      // create camera
      return new CEVA_NDE_PicoCamera();
   }
   else if (strcmp(deviceName, g_DADeviceName) == 0)
   {
      // create DA
      return new EVA_NDE_PicoDA();
   }
   else if(strcmp(deviceName, "TransposeProcessor") == 0)
   {
      return new TransposeProcessor();
   }
   else if(strcmp(deviceName, "ImageFlipX") == 0)
   {
      return new ImageFlipX();
   }
   else if(strcmp(deviceName, "ImageFlipY") == 0)
   {
      return new ImageFlipY();
   }
   else if(strcmp(deviceName, "MedianFilter") == 0)
   {
      return new MedianFilter();
   }
   else if (strcmp(deviceName, g_HubDeviceName) == 0)
   {
	  return new EVA_NDE_PicoHub();
   }

   // ...supplied name not recognized
   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// CEVA_NDE_PicoCamera implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
* CEVA_NDE_PicoCamera constructor.
* Setup default all variables and create device properties required to exist
* before intialization. In this case, no such properties were required. All
* properties will be created in the Initialize() method.
*
* As a general guideline Micro-Manager devices do not access hardware in the
* the constructor. We should do as little as possible in the constructor and
* perform most of the initialization in the Initialize() method.
*/
CEVA_NDE_PicoCamera::CEVA_NDE_PicoCamera() :
   CCameraBase<CEVA_NDE_PicoCamera> (),
   dPhase_(0),
   initialized_(false),
   readoutUs_(0.0),
   scanMode_(1),
   bitDepth_(16),
   roiX_(0),
   roiY_(0),
   sequenceStartTime_(0),
   isSequenceable_(false),
   sequenceMaxLength_(100),
   sequenceRunning_(false),
   sequenceIndex_(0),
	binSize_(1),
	cameraCCDXSize_(512),//BUFFER_SIZE/2),
	cameraCCDYSize_(1),
   ccdT_ (0.0),
   nComponents_(1),
   pEVA_NDE_PicoResourceLock_(0),
   triggerDevice_(""),
	dropPixels_(false),
   fastImage_(false),
   saturatePixels_(false),
	fractionOfPixelsToDropOrSaturate_(0.002),
   stopOnOverflow_(false),
   sampleOffset_(0)
{

   // call the base class method to set-up default error codes/messages
   InitializeDefaultErrorMessages();
   readoutStartTime_ = GetCurrentMMTime();
   pEVA_NDE_PicoResourceLock_ = new MMThreadLock();
   thd_ = new MySequenceThread(this);

   // parent ID display
   CreateHubIDProperty();
}

/**
* CEVA_NDE_PicoCamera destructor.
* If this device used as intended within the Micro-Manager system,
* Shutdown() will be always called before the destructor. But in any case
* we need to make sure that all resources are properly released even if
* Shutdown() was not called.
*/
CEVA_NDE_PicoCamera::~CEVA_NDE_PicoCamera()
{
   StopSequenceAcquisition();
   delete thd_;
   delete pEVA_NDE_PicoResourceLock_;
}

/**
* Obtains device name.
* Required by the MM::Device API.
*/
void CEVA_NDE_PicoCamera::GetName(char* name) const
{
   // Return the name used to referr to this device adapte
   CDeviceUtils::CopyLimitedString(name, g_CameraDeviceName);
}

/**
* Intializes the hardware.
* Required by the MM::Device API.
* Typically we access and initialize hardware at this point.
* Device properties are typically created here as well, except
* the ones we need to use for defining initialization parameters.
* Such pre-initialization properties are created in the constructor.
* (This device does not have any pre-initialization properties)
*/
int CEVA_NDE_PicoCamera::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub)
   {
      if (pHub->GenerateRandomError())
         return SIMULATED_ERROR;
      char hubLabel[MM::MaxStrLength];
      pHub->GetLabel(hubLabel);
      SetParentID(hubLabel); // for backward comp.
   }
   else
      LogMessage(NoHubError);

   // set property list
   // -----------------

   // Name
   int nRet = CreateProperty(MM::g_Keyword_Name, g_CameraDeviceName, MM::String, true);
   if (DEVICE_OK != nRet)
      return nRet;

   // Description
   nRet = CreateProperty(MM::g_Keyword_Description, "EVA_NDE_Pico Camera Device Adapter", MM::String, true);
   if (DEVICE_OK != nRet)
      return nRet;

   // CameraName
   nRet = CreateProperty(MM::g_Keyword_CameraName, "EVA_NDE_PicoCamera-MultiMode", MM::String, true);
   assert(nRet == DEVICE_OK);

   // CameraID
   nRet = CreateProperty(MM::g_Keyword_CameraID, "V1.0", MM::String, true);
   assert(nRet == DEVICE_OK);

   // binning
   CPropertyAction *pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnBinning);
   nRet = CreateProperty(MM::g_Keyword_Binning, "1", MM::Integer, false, pAct);
   assert(nRet == DEVICE_OK);

   //RowCount
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnRowCount);
   nRet = CreateProperty("RowCount", "1", MM::Integer, false, pAct);
   assert(nRet == DEVICE_OK);

   nRet = SetAllowedBinning();
   if (nRet != DEVICE_OK)
      return nRet;

   // pixel type
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnPixelType);
   nRet = CreateProperty(MM::g_Keyword_PixelType, g_PixelType_16bit, MM::String, false, pAct);
   assert(nRet == DEVICE_OK);

   vector<string> pixelTypeValues;
   pixelTypeValues.push_back(g_PixelType_8bit);
   pixelTypeValues.push_back(g_PixelType_16bit); 
   //pixelTypeValues.push_back(::g_PixelType_32bit);

   	SetProperty(MM::g_Keyword_PixelType, g_PixelType_16bit);

   nRet = SetAllowedValues(MM::g_Keyword_PixelType, pixelTypeValues);
   if (nRet != DEVICE_OK)
      return nRet;

   // Bit depth
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnBitDepth);
   nRet = CreateProperty("BitDepth", "8", MM::Integer, false, pAct);
   assert(nRet == DEVICE_OK);

   vector<string> bitDepths;
   bitDepths.push_back("8");
   bitDepths.push_back("10");
   bitDepths.push_back("12");
   bitDepths.push_back("14");
   bitDepths.push_back("16");
   nRet = SetAllowedValues("BitDepth", bitDepths);
   if (nRet != DEVICE_OK)
      return nRet;

   // exposure
   nRet = CreateProperty(MM::g_Keyword_Exposure, "10.0", MM::Float, false);
   assert(nRet == DEVICE_OK);
   SetPropertyLimits(MM::g_Keyword_Exposure, 0, 10000);
      // sample offset
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnSampleOffset);
   nRet = CreateProperty("SampleOffset", "0.0", MM::Integer, false, pAct);
   assert(nRet == DEVICE_OK);
   //SetPropertyLimits("SampleOffset", 0, MAX_SAMPLE_LENGTH-1000);

      //sample length
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnSampleLength);
   nRet = CreateProperty("SampleLength", "10.0", MM::Integer, false, pAct);
   assert(nRet == DEVICE_OK);
   //SetPropertyLimits("SampleLength", 0, MAX_SAMPLE_LENGTH);

      //timebase
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnTimebase);
   nRet = CreateProperty("Timebase", "10", MM::Integer, false, pAct);
   assert(nRet == DEVICE_OK);

    //timeInterval
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnTimeInterval);
   nRet = CreateProperty("TimeIntervalNs", "0", MM::Integer, true, pAct);
   assert(nRet == DEVICE_OK);


   //SetPropertyLimits("SampleLength", 0, MAX_SAMPLE_LENGTH);
	CPropertyActionEx *pActX = 0;
	// create an extended (i.e. array) properties 1 through 4

   //pAct = new CPropertyAction(this, &CEVA_NDE_PicoCamera::OnSwitch);
   //nRet = CreateProperty("Switch", "0", MM::Integer, false, pAct);
   //SetPropertyLimits("Switch", 8, 1004);
	
	
	// scan mode
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnScanMode);
   nRet = CreateProperty("ScanMode", "1", MM::Integer, false, pAct);
   assert(nRet == DEVICE_OK);
   AddAllowedValue("ScanMode","1");
   AddAllowedValue("ScanMode","2");
   AddAllowedValue("ScanMode","3");

   // camera gain
   nRet = CreateProperty(MM::g_Keyword_Gain, "0", MM::Integer, false);
   assert(nRet == DEVICE_OK);
   SetPropertyLimits(MM::g_Keyword_Gain, -5, 8);

   // camera offset
   nRet = CreateProperty(MM::g_Keyword_Offset, "0", MM::Integer, false);
   assert(nRet == DEVICE_OK);

   // camera temperature
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnCCDTemp);
   nRet = CreateProperty(MM::g_Keyword_CCDTemperature, "0", MM::Float, false, pAct);
   assert(nRet == DEVICE_OK);
   SetPropertyLimits(MM::g_Keyword_CCDTemperature, -100, 10);

   // camera temperature RO
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnCCDTemp);
   nRet = CreateProperty("CCDTemperature RO", "0", MM::Float, true, pAct);
   assert(nRet == DEVICE_OK);

   // readout time
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnReadoutTime);
   nRet = CreateProperty(MM::g_Keyword_ReadoutTime, "0", MM::Float, false, pAct);
   assert(nRet == DEVICE_OK);

   // CCD size of the camera we are modeling
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnCameraCCDXSize);
   CreateProperty("OnCameraCCDXSize", "512", MM::Integer, false, pAct);
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnCameraCCDYSize);
   CreateProperty("OnCameraCCDYSize", "512", MM::Integer, false, pAct);

   // Trigger device
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnTriggerDevice);
   CreateProperty("TriggerDevice","", MM::String, false, pAct);

   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnDropPixels);
	CreateProperty("DropPixels", "0", MM::Integer, false, pAct);
   AddAllowedValue("DropPixels", "0");
   AddAllowedValue("DropPixels", "1");

	pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnSaturatePixels);
	CreateProperty("SaturatePixels", "0", MM::Integer, false, pAct);
   AddAllowedValue("SaturatePixels", "0");
   AddAllowedValue("SaturatePixels", "1");

   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnFastImage);
	CreateProperty("FastImage", "0", MM::Integer, false, pAct);
   AddAllowedValue("FastImage", "0");
   AddAllowedValue("FastImage", "1");

   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnFractionOfPixelsToDropOrSaturate);
	CreateProperty("FractionOfPixelsToDropOrSaturate", "0.002", MM::Float, false, pAct);
	SetPropertyLimits("FractionOfPixelsToDropOrSaturate", 0., 0.1);

   // Whether or not to use exposure time sequencing
   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnIsSequenceable);
   std::string propName = "UseExposureSequences";
   CreateProperty(propName.c_str(), "No", MM::String, false, pAct);
   AddAllowedValue(propName.c_str(), "Yes");
   AddAllowedValue(propName.c_str(), "No");




   // synchronize all properties
   // --------------------------
   nRet = UpdateStatus();
   if (nRet != DEVICE_OK)
      return nRet;


   // setup the buffer
   // ----------------
   nRet = ResizeImageBuffer();
   if (nRet != DEVICE_OK)
      return nRet;

#ifdef TESTRESOURCELOCKING
   TestResourceLocking(true);
   LogMessage("TestResourceLocking OK",true);
#endif

   	status = OpenDevice(&unit);
	if(PICO_OK != status)
	return DEVICE_NOT_CONNECTED;
	picoInitBlock(&unit);
   // initialize image buffer
   GenerateEmptyImage(img_);


   pAct = new CPropertyAction (this, &CEVA_NDE_PicoCamera::OnInputRange);
   CreateProperty("InputRange", "0", MM::Integer, false, pAct);
   //add input range
   	for (int i = unit.firstRange; i <= unit.lastRange; i++) 
	{
		AddAllowedValue("InputRange",  CDeviceUtils::ConvertToString(inputRanges[i]));
	}







  initialized_ = true;
   return DEVICE_OK;
}

/**
* Shuts down (unloads) the device.
* Required by the MM::Device API.
* Ideally this method will completely unload the device and release all resources.
* Shutdown() may be called multiple times in a row.
* After Shutdown() we should be allowed to call Initialize() again to load the device
* without causing problems.
*/
int CEVA_NDE_PicoCamera::Shutdown()
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   CloseDevice(&unit);

   initialized_ = false;
   return DEVICE_OK;
}

/**
* Performs exposure and grabs a single image.
* This function should block during the actual exposure and return immediately afterwards 
* (i.e., before readout).  This behavior is needed for proper synchronization with the shutter.
* Required by the MM::Camera API.
*/
int CEVA_NDE_PicoCamera::SnapImage()
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

	static int callCounter = 0;
	++callCounter;

   MM::MMTime startTime = GetCurrentMMTime();

   //picoInitBlock(&unit);
  // CollectBlockImmediate(&unit); 
   unsigned short* pBuf = (unsigned short*) const_cast<unsigned char*>(img_.GetPixels());
	for (unsigned long k=0; k<img_.Height(); k++)
{ 		
	unsigned long j=0;
     try
	{
		int retryCount =0;
		int ret = DEVICE_ERR;
		unsigned long sampleCaptured=0;
  		while(ret != DEVICE_OK && retryCount++ < WAIT_FOR_TRIGGER_FAIL_COUNT){
			
			ret = picoRunBlock(&unit,sampleOffset_,cameraCCDXSize_,PICO_RUM_TIME_OUT, &sampleCaptured);
			//
		}
		if(ret != DEVICE_OK || sampleCaptured<0)
			return DEVICE_SNAP_IMAGE_FAILED;

	   //void* pBuf = const_cast<void*>((void*)buffers);
	   //img_.SetPixels(pBuf);
	     MMThreadGuard g(imgPixelsLock_);
		  for (j=0; j<sampleCaptured; j++)
		  {
				long lIndex = img_.Width()*k + j;
				*(pBuf + lIndex) = (unsigned short)(32768+buffers[0][j]);
				//printf("%ld:%d,",lIndex,buffers[0][j]);
				// 
		  }
		  //printf("\n");
	  }	
	 catch(...)
	{
		 LogMessage("memory overflow!", false);
		 return DEVICE_OUT_OF_MEMORY;
	}
   //GenerateSyntheticImage(img_, exp);
 }

   readoutStartTime_ = GetCurrentMMTime();

   return DEVICE_OK;
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
const unsigned char* CEVA_NDE_PicoCamera::GetImageBuffer()
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return 0;

   MMThreadGuard g(imgPixelsLock_);
   MM::MMTime readoutTime(readoutUs_);
   while (readoutTime > (GetCurrentMMTime() - readoutStartTime_)) {}		
   unsigned char *pB = (unsigned char*)(img_.GetPixels());
   return pB;
}

/**
* Returns image buffer X-size in pixels.
* Required by the MM::Camera API.
*/
unsigned CEVA_NDE_PicoCamera::GetImageWidth() const
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return 0;

   return img_.Width();
}

/**
* Returns image buffer Y-size in pixels.
* Required by the MM::Camera API.
*/
unsigned CEVA_NDE_PicoCamera::GetImageHeight() const
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return 0;

   return img_.Height();
}

/**
* Returns image buffer pixel depth in bytes.
* Required by the MM::Camera API.
*/
unsigned CEVA_NDE_PicoCamera::GetImageBytesPerPixel() const
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return 0;

   return img_.Depth();
} 

/**
* Returns the bit depth (dynamic range) of the pixel.
* This does not affect the buffer size, it just gives the client application
* a guideline on how to interpret pixel values.
* Required by the MM::Camera API.
*/
unsigned CEVA_NDE_PicoCamera::GetBitDepth() const
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return 0;

   return bitDepth_;
}

/**
* Returns the size in bytes of the image buffer.
* Required by the MM::Camera API.
*/
long CEVA_NDE_PicoCamera::GetImageBufferSize() const
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return 0;

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
* This EVA_NDE_Pico implementation ignores the position coordinates and just crops the buffer.
* @param x - top-left corner coordinate
* @param y - top-left corner coordinate
* @param xSize - width
* @param ySize - height
*/
int CEVA_NDE_PicoCamera::SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

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
int CEVA_NDE_PicoCamera::GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

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
int CEVA_NDE_PicoCamera::ClearROI()
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   ResizeImageBuffer();
   roiX_ = 0;
   roiY_ = 0;
      
   return DEVICE_OK;
}

/**
* Returns the current exposure setting in milliseconds.
* Required by the MM::Camera API.
*/
double CEVA_NDE_PicoCamera::GetExposure() const
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   char buf[MM::MaxStrLength];
   int ret = GetProperty(MM::g_Keyword_Exposure, buf);
   if (ret != DEVICE_OK)
      return 0.0;
   return atof(buf);
}

/**
 * Returns the current exposure from a sequence and increases the sequence counter
 * Used for exposure sequences
 */
double CEVA_NDE_PicoCamera::GetSequenceExposure() 
{
   if (exposureSequence_.size() == 0) 
      return this->GetExposure();

   double exposure = exposureSequence_[sequenceIndex_];

   sequenceIndex_++;
   if (sequenceIndex_ >= exposureSequence_.size())
      sequenceIndex_ = 0;

   return exposure;
}

/**
* Sets exposure in milliseconds.
* Required by the MM::Camera API.
*/
void CEVA_NDE_PicoCamera::SetExposure(double exp)
{
   SetProperty(MM::g_Keyword_Exposure, CDeviceUtils::ConvertToString(exp));
   GetCoreCallback()->OnExposureChanged(this, exp);;
}

/**
* Returns the current binning factor.
* Required by the MM::Camera API.
*/
int CEVA_NDE_PicoCamera::GetBinning() const
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   char buf[MM::MaxStrLength];
   int ret = GetProperty(MM::g_Keyword_Binning, buf);
   if (ret != DEVICE_OK)
      return 1;
   return atoi(buf);
}

/**
* Sets binning factor.
* Required by the MM::Camera API.
*/
int CEVA_NDE_PicoCamera::SetBinning(int binF)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   return SetProperty(MM::g_Keyword_Binning, CDeviceUtils::ConvertToString(binF));
}

/**
 * Clears the list of exposures used in sequences
 */
int CEVA_NDE_PicoCamera::ClearExposureSequence()
{
   exposureSequence_.clear();
   return DEVICE_OK;
}

/**
 * Adds an exposure to a list of exposures used in sequences
 */
int CEVA_NDE_PicoCamera::AddToExposureSequence(double exposureTime_ms) 
{
   exposureSequence_.push_back(exposureTime_ms);
   return DEVICE_OK;
}

int CEVA_NDE_PicoCamera::SetAllowedBinning() 
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   vector<string> binValues;
   binValues.push_back("1");
   binValues.push_back("2");
   if (scanMode_ < 3)
      binValues.push_back("4");
   if (scanMode_ < 2)
      binValues.push_back("8");
   if (binSize_ == 8 && scanMode_ == 3) {
      SetProperty(MM::g_Keyword_Binning, "2");
   } else if (binSize_ == 8 && scanMode_ == 2) {
      SetProperty(MM::g_Keyword_Binning, "4");
   } else if (binSize_ == 4 && scanMode_ == 3) {
      SetProperty(MM::g_Keyword_Binning, "2");
   }
      
   LogMessage("Setting Allowed Binning settings", true);
   return SetAllowedValues(MM::g_Keyword_Binning, binValues);
}


/**
 * Required by the MM::Camera API
 * Please implement this yourself and do not rely on the base class implementation
 * The Base class implementation is deprecated and will be removed shortly
 */
int CEVA_NDE_PicoCamera::StartSequenceAcquisition(double interval) {
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   return StartSequenceAcquisition(LONG_MAX, interval, false);            
}

/**                                                                       
* Stop and wait for the Sequence thread finished                                   
*/                                                                        
int CEVA_NDE_PicoCamera::StopSequenceAcquisition()                                     
{
   if (IsCallbackRegistered())
   {
      EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
      if (pHub && pHub->GenerateRandomError())
         return SIMULATED_ERROR;
   }

   if (!thd_->IsStopped()) {
      thd_->Stop();                                                       
      thd_->wait();                                                       
   }                                                                      
                                                                          
   return DEVICE_OK;                                                      
} 

/**
* Simple implementation of Sequence Acquisition
* A sequence acquisition should run on its own thread and transport new images
* coming of the camera into the MMCore circular buffer.
*/
int CEVA_NDE_PicoCamera::StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (IsCapturing())
      return DEVICE_CAMERA_BUSY_ACQUIRING;

   int ret = GetCoreCallback()->PrepareForAcq(this);
   if (ret != DEVICE_OK)
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
int CEVA_NDE_PicoCamera::InsertImage()
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   MM::MMTime timeStamp = this->GetCurrentMMTime();
   char label[MM::MaxStrLength];
   this->GetLabel(label);
 
   // Important:  metadata about the image are generated here:
   Metadata md;
   md.put("Camera", label);
   md.put(MM::g_Keyword_Metadata_StartTime, CDeviceUtils::ConvertToString(sequenceStartTime_.getMsec()));
   md.put(MM::g_Keyword_Elapsed_Time_ms, CDeviceUtils::ConvertToString((timeStamp - sequenceStartTime_).getMsec()));
   md.put(MM::g_Keyword_Metadata_ROI_X, CDeviceUtils::ConvertToString( (long) roiX_)); 
   md.put(MM::g_Keyword_Metadata_ROI_Y, CDeviceUtils::ConvertToString( (long) roiY_)); 

   imageCounter_++;

   char buf[MM::MaxStrLength];
   GetProperty(MM::g_Keyword_Binning, buf);
   md.put(MM::g_Keyword_Binning, buf);

   MMThreadGuard g(imgPixelsLock_);

   const unsigned char* pI;
   pI = GetImageBuffer();

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
int CEVA_NDE_PicoCamera::ThreadRun (MM::MMTime startTime)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   int ret=DEVICE_ERR;
   
   ret = SnapImage();

   if (ret != DEVICE_OK)
   {
      return ret;
   }
   ret = InsertImage();

   if (ret != DEVICE_OK)
   {
      return ret;
   }
   return ret;
};

bool CEVA_NDE_PicoCamera::IsCapturing() {
   return !thd_->IsStopped();
}

/*
 * called from the thread function before exit 
 */
void CEVA_NDE_PicoCamera::OnThreadExiting() throw()
{
   try
   {
      LogMessage(g_Msg_SEQUENCE_ACQUISITION_THREAD_EXITING);
      GetCoreCallback()?GetCoreCallback()->AcqFinished(this,0):DEVICE_OK;
   }

   catch( CMMError& e){
      std::ostringstream oss;
      oss << g_Msg_EXCEPTION_IN_ON_THREAD_EXITING << " " << e.getMsg() << " " << e.getCode();
      LogMessage(oss.str().c_str(), false);
   }
   catch(...)
   {
      LogMessage(g_Msg_EXCEPTION_IN_ON_THREAD_EXITING, false);
   }
}


MySequenceThread::MySequenceThread(CEVA_NDE_PicoCamera* pCam)
   :intervalMs_(default_intervalMS)
   ,numImages_(default_numImages)
   ,imageCounter_(0)
   ,stop_(true)
   ,suspend_(false)
   ,camera_(pCam)
   ,startTime_(0)
   ,actualDuration_(0)
   ,lastFrameTime_(0)
{};

MySequenceThread::~MySequenceThread() {};

void MySequenceThread::Stop() {
   MMThreadGuard(this->stopLock_);
   stop_=true;
}

void MySequenceThread::Start(long numImages, double intervalMs)
{
   MMThreadGuard(this->stopLock_);
   MMThreadGuard(this->suspendLock_);
   numImages_=numImages;
   intervalMs_=intervalMs;
   imageCounter_=0;
   stop_ = false;
   suspend_=false;
   activate();
   actualDuration_ = 0;
   startTime_= camera_->GetCurrentMMTime();
   lastFrameTime_ = 0;
}

bool MySequenceThread::IsStopped(){
   MMThreadGuard(this->stopLock_);
   return stop_;
}

void MySequenceThread::Suspend() {
   MMThreadGuard(this->suspendLock_);
   suspend_ = true;
}

bool MySequenceThread::IsSuspended() {
   MMThreadGuard(this->suspendLock_);
   return suspend_;
}

void MySequenceThread::Resume() {
   MMThreadGuard(this->suspendLock_);
   suspend_ = false;
}

int MySequenceThread::svc(void) throw()
{
   int ret=DEVICE_ERR;
   try 
   {
      do
      {  
         ret=camera_->ThreadRun(startTime_);
      } while (DEVICE_OK == ret && !IsStopped() && imageCounter_++ < numImages_-1);
      if (IsStopped())
         camera_->LogMessage("SeqAcquisition interrupted by the user\n");

   }catch( CMMError& e){
      camera_->LogMessage(e.getMsg(), false);
      ret = e.getCode();
   }catch(...){
      camera_->LogMessage(g_Msg_EXCEPTION_IN_THREAD, false);
   }
   stop_=true;
   actualDuration_ = camera_->GetCurrentMMTime() - startTime_;
   camera_->OnThreadExiting();
   return ret;
}


///////////////////////////////////////////////////////////////////////////////
// CEVA_NDE_PicoCamera Action handlers
///////////////////////////////////////////////////////////////////////////////



//int CEVA_NDE_PicoCamera::OnSwitch(MM::PropertyBase* pProp, MM::ActionType eAct)
//{
   // use cached values
//   return DEVICE_OK;
//}

/**
* Handles "Binning" property.
*/
int CEVA_NDE_PicoCamera::OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   int ret = DEVICE_ERR;
   switch(eAct)
   {
   case MM::AfterSet:
      {
         if(IsCapturing())
            return DEVICE_CAMERA_BUSY_ACQUIRING;

         // the user just set the new value for the property, so we have to
         // apply this value to the 'hardware'.
         long binFactor;
         pProp->Get(binFactor);
			if(binFactor > 0 && binFactor < 10)
			{
				img_.Resize(cameraCCDXSize_/binFactor, cameraCCDYSize_/binFactor);
				binSize_ = binFactor;
            std::ostringstream os;
            os << binSize_;
            OnPropertyChanged("Binning", os.str().c_str());
				ret=DEVICE_OK;
			}
      }break;
   case MM::BeforeGet:
      {
         ret=DEVICE_OK;
			pProp->Set(binSize_);
      }break;
   }
   return ret; 
}
/**
* Handles "RowCount" property.
*/
int CEVA_NDE_PicoCamera::OnRowCount(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::BeforeGet)
   {
		pProp->Set(cameraCCDYSize_);
   }
   else if (eAct == MM::AfterSet)
   {
      long value;
      pProp->Get(value);
		if ( (value < 1) || (MAX_SAMPLE_LENGTH < value))
			return DEVICE_ERR;  // invalid image size
		if( value != cameraCCDYSize_)
		{
			cameraCCDYSize_ = value;
			img_.Resize(cameraCCDXSize_/binSize_, cameraCCDYSize_/binSize_);
		}
   }
	return DEVICE_OK; 
}
/**
* Handles "PixelType" property.
*/
int CEVA_NDE_PicoCamera::OnPixelType(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   int ret = DEVICE_ERR;
   switch(eAct)
   {
   case MM::AfterSet:
      {
         if(IsCapturing())
            return DEVICE_CAMERA_BUSY_ACQUIRING;

         string pixelType;
         pProp->Get(pixelType);

         if (pixelType.compare(g_PixelType_8bit) == 0)
         {
            nComponents_ = 1;
            img_.Resize(img_.Width(), img_.Height(), 1);
            bitDepth_ = 8;
            ret=DEVICE_OK;
         }
         else if (pixelType.compare(g_PixelType_16bit) == 0)
         {
            nComponents_ = 1;
            img_.Resize(img_.Width(), img_.Height(), 2);
            ret=DEVICE_OK;
         }
	
         else
         {
            // on error switch to default pixel type
            nComponents_ = 1;
            img_.Resize(img_.Width(), img_.Height(), 1);
            pProp->Set(g_PixelType_8bit);
            ret = ERR_UNKNOWN_MODE;
         }
      } break;
   case MM::BeforeGet:
      {
         long bytesPerPixel = GetImageBytesPerPixel();
         if (bytesPerPixel == 1)
         	pProp->Set(g_PixelType_8bit);
         else if (bytesPerPixel == 2)
         	pProp->Set(g_PixelType_16bit);
     		else
				pProp->Set(g_PixelType_8bit);
         ret=DEVICE_OK;
      }break;
   }
   return ret; 
}

/**
* Handles "BitDepth" property.
*/
int CEVA_NDE_PicoCamera::OnBitDepth(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   int ret = DEVICE_ERR;
   switch(eAct)
   {
   case MM::AfterSet:
      {
         if(IsCapturing())
            return DEVICE_CAMERA_BUSY_ACQUIRING;

         long bitDepth;
         pProp->Get(bitDepth);

			unsigned int bytesPerComponent;

         switch (bitDepth) {
            case 8:
					bytesPerComponent = 1;
               bitDepth_ = 8;
               ret=DEVICE_OK;
            break;
            case 10:
					bytesPerComponent = 2;
               bitDepth_ = 10;
               ret=DEVICE_OK;
            break;
            case 12:
					bytesPerComponent = 2;
               bitDepth_ = 12;
               ret=DEVICE_OK;
            break;
            case 14:
					bytesPerComponent = 2;
               bitDepth_ = 14;
               ret=DEVICE_OK;
            break;
            case 16:
					bytesPerComponent = 2;
               bitDepth_ = 16;
               ret=DEVICE_OK;
            break;
            default: 
               // on error switch to default pixel type
					bytesPerComponent = 1;

               pProp->Set((long)8);
               bitDepth_ = 8;
               ret = ERR_UNKNOWN_MODE;
            break;
         }
			char buf[MM::MaxStrLength];
			GetProperty(MM::g_Keyword_PixelType, buf);
			std::string pixelType(buf);
			unsigned int bytesPerPixel = 1;
			

         // automagickally change pixel type when bit depth exceeds possible value
         if (pixelType.compare(g_PixelType_8bit) == 0)
         {
				if( 2 == bytesPerComponent)
				{
					SetProperty(MM::g_Keyword_PixelType, g_PixelType_16bit);
					bytesPerPixel = 2;
				}
				else
				{
				   bytesPerPixel = 1;
				}
         }
         else if (pixelType.compare(g_PixelType_16bit) == 0)
         {
				bytesPerPixel = 2;
         }

			img_.Resize(img_.Width(), img_.Height(), bytesPerPixel);

      } break;
   case MM::BeforeGet:
      {
         pProp->Set((long)bitDepth_);
         ret=DEVICE_OK;
      }break;
   }
   return ret; 
}
/**
* Handles "ReadoutTime" property.
*/
int CEVA_NDE_PicoCamera::OnReadoutTime(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::AfterSet)
   {
      double readoutMs;
      pProp->Get(readoutMs);

      readoutUs_ = readoutMs * 1000.0;
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(readoutUs_ / 1000.0);
   }

   return DEVICE_OK;
}

int CEVA_NDE_PicoCamera::OnDropPixels(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::AfterSet)
   {
      long tvalue = 0;
      pProp->Get(tvalue);
		dropPixels_ = (0==tvalue)?false:true;
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(dropPixels_?1L:0L);
   }

   return DEVICE_OK;
}

int CEVA_NDE_PicoCamera::OnFastImage(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::AfterSet)
   {
      long tvalue = 0;
      pProp->Get(tvalue);
		fastImage_ = (0==tvalue)?false:true;
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(fastImage_?1L:0L);
   }

   return DEVICE_OK;
}

int CEVA_NDE_PicoCamera::OnSaturatePixels(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::AfterSet)
   {
      long tvalue = 0;
      pProp->Get(tvalue);
		saturatePixels_ = (0==tvalue)?false:true;
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(saturatePixels_?1L:0L);
   }

   return DEVICE_OK;
}

int CEVA_NDE_PicoCamera::OnFractionOfPixelsToDropOrSaturate(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::AfterSet)
   {
      double tvalue = 0;
      pProp->Get(tvalue);
		fractionOfPixelsToDropOrSaturate_ = tvalue;
   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(fractionOfPixelsToDropOrSaturate_);
   }

   return DEVICE_OK;
}

/*
* Handles "ScanMode" property.
* Changes allowed Binning values to test whether the UI updates properly
*/
int CEVA_NDE_PicoCamera::OnScanMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{ 
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::AfterSet) {
      pProp->Get(scanMode_);
      SetAllowedBinning();
      if (initialized_) {
         int ret = OnPropertiesChanged();
         if (ret != DEVICE_OK)
            return ret;
      }
   } else if (eAct == MM::BeforeGet) {
      LogMessage("Reading property ScanMode", true);
      pProp->Set(scanMode_);
   }
   return DEVICE_OK;
}

/**
* Handles "Input Range" property.
*/
int CEVA_NDE_PicoCamera::OnInputRange(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::AfterSet)
   {
	  long value;
      pProp->Get(value);
		if ( (value < 1) || (MAX_SAMPLE_LENGTH < value))
			return DEVICE_ERR;  // invalid image size
		int indexFound = -1;
		for (int i = unit.firstRange; i <= unit.lastRange; i++) 
		{
			 if(value==inputRanges[i])
			 {
				 indexFound = i;
			 }
		}
		if(indexFound>0)
			picoSetVoltages(&unit,indexFound);
   }
   else if (eAct == MM::BeforeGet)
   {
	   pProp->Set((long)inputRanges[unit.channelSettings[0].range]);
   }

   return DEVICE_OK;
}
/**
* Handles "SampleLength" property.
*/
int CEVA_NDE_PicoCamera::OnSampleLength(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::AfterSet)
   {
	  long value;
      pProp->Get(value);
		if ( (value < 1) || (MAX_SAMPLE_LENGTH < value))
			return DEVICE_ERR;  // invalid image size
		if( value != cameraCCDXSize_)
		{
			cameraCCDXSize_ = value;
			img_.Resize(cameraCCDXSize_/binSize_, cameraCCDYSize_/binSize_);
		}
   }
   else if (eAct == MM::BeforeGet)
   {
	pProp->Set(cameraCCDXSize_);
   }

   return DEVICE_OK;
}

/**
* Handles "Timebase" property.
*/
int CEVA_NDE_PicoCamera::OnTimebase(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::AfterSet)
   {
	  long value;
      pProp->Get(value);
	  picoSetTimebase(&unit,value);
   }
   else if (eAct == MM::BeforeGet)
   {
	pProp->Set((long)timebase);
   }

   return DEVICE_OK;
}
/**
* Handles "TimeInterval" property.
*/
int CEVA_NDE_PicoCamera::OnTimeInterval(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;
   else if (eAct == MM::BeforeGet)
   {
	pProp->Set((long)timeInterval);
   }

   return DEVICE_OK;
}
/**
* Handles "SampleOffset" property.
*/
int CEVA_NDE_PicoCamera::OnSampleOffset(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::AfterSet)
   {
	  long value;
      pProp->Get(value);
	  sampleOffset_=value;
   }
   else if (eAct == MM::BeforeGet)
   {
	pProp->Set(sampleOffset_);
   }

   return DEVICE_OK;
}
int CEVA_NDE_PicoCamera::OnCameraCCDXSize(MM::PropertyBase* pProp , MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::BeforeGet)
   {
		pProp->Set(cameraCCDXSize_);
   }
   else if (eAct == MM::AfterSet)
   {
      long value;
      pProp->Get(value);
		if ( (value < 1) || (MAX_SAMPLE_LENGTH < value))
			return DEVICE_ERR;  // invalid image size
		if( value != cameraCCDXSize_)
		{
			cameraCCDXSize_ = value;
			img_.Resize(cameraCCDXSize_/binSize_, cameraCCDYSize_/binSize_);
		}
   }
	return DEVICE_OK;

}

int CEVA_NDE_PicoCamera::OnCameraCCDYSize(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::BeforeGet)
   {
		pProp->Set(cameraCCDYSize_);
   }
   else if (eAct == MM::AfterSet)
   {
      long value;
      pProp->Get(value);
		if ( (value < 1) || (MAX_SAMPLE_LENGTH < value))
			return DEVICE_ERR;  // invalid image size
		if( value != cameraCCDYSize_)
		{
			cameraCCDYSize_ = value;
			img_.Resize(cameraCCDXSize_/binSize_, cameraCCDYSize_/binSize_);
		}
   }
	return DEVICE_OK;

}

int CEVA_NDE_PicoCamera::OnTriggerDevice(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::BeforeGet)
   {
      pProp->Set(triggerDevice_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(triggerDevice_);
   }
   return DEVICE_OK;
}


int CEVA_NDE_PicoCamera::OnCCDTemp(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(ccdT_);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(ccdT_);
   }
   return DEVICE_OK;
}

int CEVA_NDE_PicoCamera::OnIsSequenceable(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::string val = "Yes";
   if (eAct == MM::BeforeGet)
   {
      if (!isSequenceable_) 
      {
         val = "No";
      }
      pProp->Set(val.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      isSequenceable_ = false;
      pProp->Get(val);
      if (val == "Yes") 
      {
         isSequenceable_ = true;
      }
   }

   return DEVICE_OK;
}


///////////////////////////////////////////////////////////////////////////////
// Private CEVA_NDE_PicoCamera methods
///////////////////////////////////////////////////////////////////////////////

/**
* Sync internal image buffer size to the chosen property values.
*/
int CEVA_NDE_PicoCamera::ResizeImageBuffer()
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   char buf[MM::MaxStrLength];
   int ret = GetProperty(MM::g_Keyword_Binning, buf);
   if (ret != DEVICE_OK)
      return ret;
   binSize_ = atol(buf);

   ret = GetProperty(MM::g_Keyword_PixelType, buf);
   if (ret != DEVICE_OK)
      return ret;

	std::string pixelType(buf);
	int byteDepth = 0;

   if (pixelType.compare(g_PixelType_8bit) == 0)
   {
      byteDepth = 1;
   }
   else if (pixelType.compare(g_PixelType_16bit) == 0)
   {
      byteDepth = 2;
   }


   img_.Resize(cameraCCDXSize_/binSize_, cameraCCDYSize_/binSize_, byteDepth);
   return DEVICE_OK;
}

void CEVA_NDE_PicoCamera::GenerateEmptyImage(ImgBuffer& img)
{
   MMThreadGuard g(imgPixelsLock_);
   if (img.Height() == 0 || img.Width() == 0 || img.Depth() == 0)
      return;
   unsigned char* pBuf = const_cast<unsigned char*>(img.GetPixels());
   memset(pBuf, 0, img.Height()*img.Width()*img.Depth());
}



/**
* Generate a spatial sine wave.
*/
void CEVA_NDE_PicoCamera::GenerateSyntheticImage(ImgBuffer& img, double exp)
{ 
  
   MMThreadGuard g(imgPixelsLock_);

	//std::string pixelType;
	char buf[MM::MaxStrLength];
   GetProperty(MM::g_Keyword_PixelType, buf);
	std::string pixelType(buf);

	if (img.Height() == 0 || img.Width() == 0 || img.Depth() == 0)
      return;

   const double cPi = 3.14159265358979;
   long lPeriod = img.Width()/2;
   double dLinePhase = 0.0;
   const double dAmp = exp;
   const double cLinePhaseInc = 2.0 * cPi / 4.0 / img.Height();

   static bool debugRGB = false;
#ifdef TIFFEVA_NDE_Pico
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
   if (pixelType.compare(g_PixelType_8bit) == 0)
   {
      double pedestal = 127 * exp / 100.0 * GetBinning() * GetBinning();
      unsigned char* pBuf = const_cast<unsigned char*>(img.GetPixels());
      for (j=0; j<img.Height(); j++)
      {
         for (k=0; k<img.Width(); k++)
         {
            long lIndex = img.Width()*j + k;
            *(pBuf + lIndex) = (unsigned char) (g_IntensityFactor_ * min(255.0, (pedestal + dAmp * sin(dPhase_ + dLinePhase + (2.0 * cPi * k) / lPeriod))));
         }
         dLinePhase += cLinePhaseInc;
      }
	   for(int snoise = 0; snoise < pixelsToSaturate; ++snoise)
		{
			j = (unsigned)( (double)(img.Height()-1)*(double)rand()/(double)RAND_MAX);
			k = (unsigned)( (double)(img.Width()-1)*(double)rand()/(double)RAND_MAX);
			*(pBuf + img.Width()*j + k) = (unsigned char)maxValue;
		}
		int pnoise;
		for(pnoise = 0; pnoise < pixelsToDrop; ++pnoise)
		{
			j = (unsigned)( (double)(img.Height()-1)*(double)rand()/(double)RAND_MAX);
			k = (unsigned)( (double)(img.Width()-1)*(double)rand()/(double)RAND_MAX);
			*(pBuf + img.Width()*j + k) = 0;
		}

   }
   else if (pixelType.compare(g_PixelType_16bit) == 0)
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
}


void CEVA_NDE_PicoCamera::TestResourceLocking(const bool recurse)
{
   MMThreadGuard g(*pEVA_NDE_PicoResourceLock_);
   if(recurse)
      TestResourceLocking(false);
}



/****
* EVA_NDE_Pico DA device
*/

EVA_NDE_PicoDA::EVA_NDE_PicoDA () : 
volt_(0), 
gatedVolts_(0), 
open_(true),
sequenceRunning_(false),
sequenceIndex_(0),
sentSequence_(vector<double>()),
nascentSequence_(vector<double>())
{
   SetErrorText(SIMULATED_ERROR, "Random, simluated error");
   SetErrorText(ERR_SEQUENCE_INACTIVE, "Sequence triggered, but sequence is not running");

   // parent ID display
   CreateHubIDProperty();
}

EVA_NDE_PicoDA::~EVA_NDE_PicoDA() {
}

void EVA_NDE_PicoDA::GetName(char* name) const
{
   CDeviceUtils::CopyLimitedString(name, g_DADeviceName);
}

int EVA_NDE_PicoDA::Initialize()
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub)
   {
      if (pHub->GenerateRandomError())
         return SIMULATED_ERROR;
      char hubLabel[MM::MaxStrLength];
      pHub->GetLabel(hubLabel);
      SetParentID(hubLabel); // for backward comp.
   }
   else
      LogMessage(NoHubError);

   // Triggers to test sequence capabilities
   CPropertyAction* pAct = new CPropertyAction (this, &EVA_NDE_PicoDA::OnTrigger);
   CreateProperty("Trigger", "-", MM::String, false, pAct);
   AddAllowedValue("Trigger", "-");
   AddAllowedValue("Trigger", "+");

   pAct = new CPropertyAction(this, &EVA_NDE_PicoDA::OnVoltage);
   CreateProperty("Voltage", "0", MM::Float, false, pAct);
   SetPropertyLimits("Voltage", 0.0, 10.0);

   pAct = new CPropertyAction(this, &EVA_NDE_PicoDA::OnRealVoltage);
   CreateProperty("Real Voltage", "0", MM::Float, true, pAct);

   return DEVICE_OK;
}

int EVA_NDE_PicoDA::SetGateOpen(bool open) 
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   open_ = open; 
   if (open_) 
      gatedVolts_ = volt_; 
   else 
      gatedVolts_ = 0;

   return DEVICE_OK;
}

int EVA_NDE_PicoDA::GetGateOpen(bool& open) 
{
   open = open_; 
   return DEVICE_OK;
}

int EVA_NDE_PicoDA::SetSignal(double volts) {
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   volt_ = volts; 
   if (open_)
      gatedVolts_ = volts;
   stringstream s;
   s << "Voltage set to " << volts;
   LogMessage(s.str(), false);
   return DEVICE_OK;
}

int EVA_NDE_PicoDA::GetSignal(double& volts) 
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   volts = volt_; 
   return DEVICE_OK;
}

int EVA_NDE_PicoDA::SendDASequence() 
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   (const_cast<EVA_NDE_PicoDA*> (this))->SetSentSequence();
   return DEVICE_OK;
}

// private
void EVA_NDE_PicoDA::SetSentSequence()
{
   sentSequence_ = nascentSequence_;
   nascentSequence_.clear();
}

int EVA_NDE_PicoDA::ClearDASequence()
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   nascentSequence_.clear();
   return DEVICE_OK;
}

int EVA_NDE_PicoDA::AddToDASequence(double voltage) {
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   nascentSequence_.push_back(voltage);
   return DEVICE_OK;
}

int EVA_NDE_PicoDA::OnTrigger(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::BeforeGet)
   {
      pProp->Set("-");
   } else if (eAct == MM::AfterSet) {
      if (!sequenceRunning_)
         return ERR_SEQUENCE_INACTIVE;
      std::string tr;
      pProp->Get(tr);
      if (tr == "+") {
         if (sequenceIndex_ < sentSequence_.size()) {
            double voltage = sentSequence_[sequenceIndex_];
            int ret = SetSignal(voltage);
            if (ret != DEVICE_OK)
               return ERR_IN_SEQUENCE;
            sequenceIndex_++;
            if (sequenceIndex_ >= sentSequence_.size()) {
               sequenceIndex_ = 0;
            }
         } else
         {
            return ERR_IN_SEQUENCE;
         }
      }
   }
   return DEVICE_OK;
}

int EVA_NDE_PicoDA::OnVoltage(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      double volts = 0.0;
      GetSignal(volts);
      pProp->Set(volts);
   }
   else if (eAct == MM::AfterSet)
   {
      double volts = 0.0;
      pProp->Get(volts);
      SetSignal(volts);
   }
   return DEVICE_OK;
}

int EVA_NDE_PicoDA::OnRealVoltage(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(gatedVolts_);
   }
   return DEVICE_OK;
}



int TransposeProcessor::Initialize()
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub)
   {
      if (pHub->GenerateRandomError())
         return SIMULATED_ERROR;
      char hubLabel[MM::MaxStrLength];
      pHub->GetLabel(hubLabel);
      SetParentID(hubLabel); // for backward comp.
   }
   else
      LogMessage(NoHubError);

   if( NULL != this->pTemp_)
   {
      free(pTemp_);
      pTemp_ = NULL;
      this->tempSize_ = 0;
   }
    CPropertyAction* pAct = new CPropertyAction (this, &TransposeProcessor::OnInPlaceAlgorithm);
   (void)CreateProperty("InPlaceAlgorithm", "0", MM::Integer, false, pAct); 
   return DEVICE_OK;
}

   // action interface
   // ----------------
int TransposeProcessor::OnInPlaceAlgorithm(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   if (eAct == MM::BeforeGet)
   {
      pProp->Set(this->inPlace_?1L:0L);
   }
   else if (eAct == MM::AfterSet)
   {
      long ltmp;
      pProp->Get(ltmp);
      inPlace_ = (0==ltmp?false:true);
   }

   return DEVICE_OK;
}


int TransposeProcessor::Process(unsigned char *pBuffer, unsigned int width, unsigned int height, unsigned int byteDepth)
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   int ret = DEVICE_OK;
   // 
   if( width != height)
      return DEVICE_NOT_SUPPORTED; // problem with tranposing non-square images is that the image buffer
   // will need to be modified by the image processor.
   if(busy_)
      return DEVICE_ERR;
 
   busy_ = true;

   if( inPlace_)
   {
      if(  sizeof(unsigned char) == byteDepth)
      {
         TransposeSquareInPlace( (unsigned char*)pBuffer, width);
      }
      else if( sizeof(unsigned short) == byteDepth)
      {
         TransposeSquareInPlace( (unsigned short*)pBuffer, width);
      }
      else if( sizeof(unsigned long) == byteDepth)
      {
         TransposeSquareInPlace( (unsigned long*)pBuffer, width);
      }
      else if( sizeof(unsigned long long) == byteDepth)
      {
         TransposeSquareInPlace( (unsigned long long*)pBuffer, width);
      }
      else 
      {
         ret = DEVICE_NOT_SUPPORTED;
      }
   }
   else
   {
      if( sizeof(unsigned char) == byteDepth)
      {
         ret = TransposeRectangleOutOfPlace( (unsigned char*)pBuffer, width, height);
      }
      else if( sizeof(unsigned short) == byteDepth)
      {
         ret = TransposeRectangleOutOfPlace( (unsigned short*)pBuffer, width, height);
      }
      else if( sizeof(unsigned long) == byteDepth)
      {
         ret = TransposeRectangleOutOfPlace( (unsigned long*)pBuffer, width, height);
      }
      else if( sizeof(unsigned long long) == byteDepth)
      {
         ret =  TransposeRectangleOutOfPlace( (unsigned long long*)pBuffer, width, height);
      }
      else
      {
         ret =  DEVICE_NOT_SUPPORTED;
      }
   }
   busy_ = false;

   return ret;
}




int ImageFlipY::Initialize()
{
    CPropertyAction* pAct = new CPropertyAction (this, &ImageFlipY::OnPerformanceTiming);
    (void)CreateProperty("PeformanceTiming (microseconds)", "0", MM::Float, true, pAct); 
   return DEVICE_OK;
}

   // action interface
   // ----------------
int ImageFlipY::OnPerformanceTiming(MM::PropertyBase* pProp, MM::ActionType eAct)
{

   if (eAct == MM::BeforeGet)
   {
      pProp->Set( performanceTiming_.getUsec());
   }
   else if (eAct == MM::AfterSet)
   {
      // -- it's ready only!
   }

   return DEVICE_OK;
}


int ImageFlipY::Process(unsigned char *pBuffer, unsigned int width, unsigned int height, unsigned int byteDepth)
{
   if(busy_)
      return DEVICE_ERR;

   int ret = DEVICE_OK;
 
   busy_ = true;
   performanceTiming_ = MM::MMTime(0.);
   MM::MMTime  s0 = GetCurrentMMTime();


   if( sizeof(unsigned char) == byteDepth)
   {
      ret = Flip( (unsigned char*)pBuffer, width, height);
   }
   else if( sizeof(unsigned short) == byteDepth)
   {
      ret = Flip( (unsigned short*)pBuffer, width, height);
   }
   else if( sizeof(unsigned long) == byteDepth)
   {
      ret = Flip( (unsigned long*)pBuffer, width, height);
   }
   else if( sizeof(unsigned long long) == byteDepth)
   {
      ret =  Flip( (unsigned long long*)pBuffer, width, height);
   }
   else
   {
      ret =  DEVICE_NOT_SUPPORTED;
   }

   performanceTiming_ = GetCurrentMMTime() - s0;
   busy_ = false;

   return ret;
}







///
int ImageFlipX::Initialize()
{
    CPropertyAction* pAct = new CPropertyAction (this, &ImageFlipX::OnPerformanceTiming);
    (void)CreateProperty("PeformanceTiming (microseconds)", "0", MM::Float, true, pAct); 
   return DEVICE_OK;
}

   // action interface
   // ----------------
int ImageFlipX::OnPerformanceTiming(MM::PropertyBase* pProp, MM::ActionType eAct)
{

   if (eAct == MM::BeforeGet)
   {
      pProp->Set( performanceTiming_.getUsec());
   }
   else if (eAct == MM::AfterSet)
   {
      // -- it's ready only!
   }

   return DEVICE_OK;
}


int ImageFlipX::Process(unsigned char *pBuffer, unsigned int width, unsigned int height, unsigned int byteDepth)
{
   if(busy_)
      return DEVICE_ERR;

   int ret = DEVICE_OK;
 
   busy_ = true;
   performanceTiming_ = MM::MMTime(0.);
   MM::MMTime  s0 = GetCurrentMMTime();


   if( sizeof(unsigned char) == byteDepth)
   {
      ret = Flip( (unsigned char*)pBuffer, width, height);
   }
   else if( sizeof(unsigned short) == byteDepth)
   {
      ret = Flip( (unsigned short*)pBuffer, width, height);
   }
   else if( sizeof(unsigned long) == byteDepth)
   {
      ret = Flip( (unsigned long*)pBuffer, width, height);
   }
   else if( sizeof(unsigned long long) == byteDepth)
   {
      ret =  Flip( (unsigned long long*)pBuffer, width, height);
   }
   else
   {
      ret =  DEVICE_NOT_SUPPORTED;
   }

   performanceTiming_ = GetCurrentMMTime() - s0;
   busy_ = false;

   return ret;
}

///
int MedianFilter::Initialize()
{
    CPropertyAction* pAct = new CPropertyAction (this, &MedianFilter::OnPerformanceTiming);
    (void)CreateProperty("PeformanceTiming (microseconds)", "0", MM::Float, true, pAct); 
    (void)CreateProperty("BEWARE", "THIS FILTER MODIFIES DATA, EACH PIXEL IS REPLACED BY 3X3 NEIGHBORHOOD MEDIAN", MM::String, true); 
   return DEVICE_OK;
}

   // action interface
   // ----------------
int MedianFilter::OnPerformanceTiming(MM::PropertyBase* pProp, MM::ActionType eAct)
{

   if (eAct == MM::BeforeGet)
   {
      pProp->Set( performanceTiming_.getUsec());
   }
   else if (eAct == MM::AfterSet)
   {
      // -- it's ready only!
   }

   return DEVICE_OK;
}


int MedianFilter::Process(unsigned char *pBuffer, unsigned int width, unsigned int height, unsigned int byteDepth)
{
   if(busy_)
      return DEVICE_ERR;

   int ret = DEVICE_OK;
 
   busy_ = true;
   performanceTiming_ = MM::MMTime(0.);
   MM::MMTime  s0 = GetCurrentMMTime();


   if( sizeof(unsigned char) == byteDepth)
   {
      ret = Filter( (unsigned char*)pBuffer, width, height);
   }
   else if( sizeof(unsigned short) == byteDepth)
   {
      ret = Filter( (unsigned short*)pBuffer, width, height);
   }
   else if( sizeof(unsigned long) == byteDepth)
   {
      ret = Filter( (unsigned long*)pBuffer, width, height);
   }
   else if( sizeof(unsigned long long) == byteDepth)
   {
      ret =  Filter( (unsigned long long*)pBuffer, width, height);
   }
   else
   {
      ret =  DEVICE_NOT_SUPPORTED;
   }

   performanceTiming_ = GetCurrentMMTime() - s0;
   busy_ = false;

   return ret;
}


int EVA_NDE_PicoHub::Initialize()
{
   EVA_NDE_PicoHub* pHub = static_cast<EVA_NDE_PicoHub*>(GetParentHub());
   if (pHub && pHub->GenerateRandomError())
      return SIMULATED_ERROR;

   SetErrorText(SIMULATED_ERROR, "Simulated Error");
	CPropertyAction *pAct = new CPropertyAction (this, &EVA_NDE_PicoHub::OnErrorRate);
	CreateProperty("SimulatedErrorRate", "0.0", MM::Float, false, pAct);
	AddAllowedValue("SimulatedErrorRate", "0.0000");
	AddAllowedValue("SimulatedErrorRate", "0.0001");
   AddAllowedValue("SimulatedErrorRate", "0.0010");
	AddAllowedValue("SimulatedErrorRate", "0.0100");
	AddAllowedValue("SimulatedErrorRate", "0.1000");
	AddAllowedValue("SimulatedErrorRate", "0.2000");
	AddAllowedValue("SimulatedErrorRate", "0.5000");
	AddAllowedValue("SimulatedErrorRate", "1.0000");

   pAct = new CPropertyAction (this, &EVA_NDE_PicoHub::OnDivideOneByMe);
   std::ostringstream os;
   os<<this->divideOneByMe_;
   CreateProperty("DivideOneByMe", os.str().c_str(), MM::Integer, false, pAct);

  	initialized_ = true;
 
	return DEVICE_OK;
}

int EVA_NDE_PicoHub::DetectInstalledDevices()
{  
   ClearInstalledDevices();

   // make sure this method is called before we look for available devices
   InitializeModuleData();

   char hubName[MM::MaxStrLength];
   GetName(hubName); // this device name
   for (unsigned i=0; i<GetNumberOfDevices(); i++)
   { 
      char deviceName[MM::MaxStrLength];
      bool success = GetDeviceName(i, deviceName, MM::MaxStrLength);
      if (success && (strcmp(hubName, deviceName) != 0))
      {
         MM::Device* pDev = CreateDevice(deviceName);
         AddInstalledDevice(pDev);
      }
   }
   return DEVICE_OK; 
}

MM::Device* EVA_NDE_PicoHub::CreatePeripheralDevice(const char* adapterName)
{
   for (unsigned i=0; i<GetNumberOfInstalledDevices(); i++)
   {
      MM::Device* d = GetInstalledDevice(i);
      char name[MM::MaxStrLength];
      d->GetName(name);
      if (strcmp(adapterName, name) == 0)
         return CreateDevice(adapterName);

   }
   return 0; // adapter name not found
}


void EVA_NDE_PicoHub::GetName(char* pName) const
{
   CDeviceUtils::CopyLimitedString(pName, g_HubDeviceName);
}

bool EVA_NDE_PicoHub::GenerateRandomError()
{
   if (errorRate_ == 0.0)
      return false;

	return (0 == (rand() % ((int) (1.0 / errorRate_))));
}

int EVA_NDE_PicoHub::OnErrorRate(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   // Don't simulate an error here!!!!

   if (eAct == MM::AfterSet)
   {
      pProp->Get(errorRate_);

   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(errorRate_);
   }
   return DEVICE_OK;
}

int EVA_NDE_PicoHub::OnDivideOneByMe(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   // Don't simulate an error here!!!!

   if (eAct == MM::AfterSet)
   {
      pProp->Get(divideOneByMe_);
      static long result = 0;
      bool crashtest = CDeviceUtils::CheckEnvironment("MICROMANAGERCRASHTEST");
      if((0 != divideOneByMe_) || crashtest)
         result = 1/divideOneByMe_;
      result = result;

   }
   else if (eAct == MM::BeforeGet)
   {
      pProp->Set(divideOneByMe_);
   }
   return DEVICE_OK;
}


