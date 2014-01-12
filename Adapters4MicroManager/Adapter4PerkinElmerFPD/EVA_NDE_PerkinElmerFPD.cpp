///////////////////////////////////////////////////////////////////////////////
// FILE:          EVA_NDE_PerkinElmerFPD.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   The example implementation of the EVA_NDE_PerkinElmer camera.
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
// CVS:           $Id: EVA_NDE_PerkinElmerFPD.cpp 10842 2013-04-24 01:21:05Z mark $
//

#include "EVA_NDE_PerkinElmerFPD.h"
#include <cstdio>
#include <string>
#include <math.h>
#include "../../MMDevice/ModuleInterface.h"
#include "../../MMCore/Error.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include "XIS_func.c"
#define MAX_SAMPLE_LENGTH 3000000
#define WAIT_FOR_TRIGGER_FAIL_COUNT 2
using namespace std;
const double CEVA_NDE_PerkinElmerFPD::nominalPixelSizeUm_ = 1.0;
double g_IntensityFactor_ = 1.0;

// External names used used by the rest of the system
// 
const char* g_CameraDeviceName = "PerkinElmerFPD";
const char* g_HubDeviceName = "PerkinElmerHub";

// constants for naming pixel types (allowed values of the "PixelType" property)
const char* g_PixelType_8bit = "8bit";
const char* g_PixelType_16bit = "16bit";

// constants for naming acquisition mode
const char* g_acquisitionMode_0 = "Free Running";
const char* g_acquisitionMode_1 = "External Triggered";
const char* g_acquisitionMode_2 = "Internal Triggered";
const char* g_acquisitionMode_3 = "Soft Triggered";

const char* g_FrameTime_TIMING_0 = "<=134ms";
const char* g_FrameTime_TIMING_1 = "200ms";
const char* g_FrameTime_TIMING_2 = "400ms";
const char* g_FrameTime_TIMING_3 = "800ms";
const char* g_FrameTime_TIMING_4 = "1600ms";
const char* g_FrameTime_TIMING_5 = "3200ms";
const char* g_FrameTime_TIMING_6 = "6400ms";
const char* g_FrameTime_TIMING_7 = "12800ms";

const char* g_TriggerMode_0 = "DataDeliveredOnDemand";
const char* g_TriggerMode_1 = "DataDeliveredOnDemandNoClearance";
const char* g_TriggerMode_2 = "Linewise";
const char* g_TriggerMode_3 = "Framewise";

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
   AddAvailableDeviceName(g_CameraDeviceName, "EVA_NDE_PerkinElmer Camera");
   AddAvailableDeviceName("TransposeProcessor", "TransposeProcessor");
   AddAvailableDeviceName("ImageFlipX", "ImageFlipX");
   AddAvailableDeviceName("ImageFlipY", "ImageFlipY");
   AddAvailableDeviceName("MedianFilter", "MedianFilter");
   AddAvailableDeviceName(g_HubDeviceName, "EVA_NDE_PerkinElmer Hub");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return NULL;

   // decide which device class to create based on the deviceName parameter
   if (strcmp(deviceName, g_CameraDeviceName) == 0)
   {
      // create camera
      return new CEVA_NDE_PerkinElmerFPD();
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
	  return new EVA_NDE_PerkinElmerHub();
   }

   // ...supplied name not recognized
   return NULL;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// CEVA_NDE_PerkinElmerFPD implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
* CEVA_NDE_PerkinElmerFPD constructor.
* Setup default all variables and create device properties required to exist
* before intialization. In this case, no such properties were required. All
* properties will be created in the Initialize() method.
*
* As a general guideline Micro-Manager devices do not access hardware in the
* the constructor. We should do as little as possible in the constructor and
* perform most of the initialization in the Initialize() method.
*/
CEVA_NDE_PerkinElmerFPD::CEVA_NDE_PerkinElmerFPD() :
   CCameraBase<CEVA_NDE_PerkinElmerFPD> (),
   initialized_(false),
   acquisitionMode_(0),
   bitDepth_(16),
   roiX_(0),
   roiY_(0),
   sequenceStartTime_(0),
   isSequenceable_(false),
   sequenceMaxLength_(100),
   sequenceRunning_(false),
   sequenceIndex_(0),
   binMode_(1),
   exposureMs_(10),
   triggerMode_(0),
   byteDepth_(2),
   frameTiming_(0),
   image_width(512),
   image_height(512),
   pEVA_NDE_PerkinElmerResourceLock_(0),
   triggerDevice_(""),
   stopOnOverflow_(false),
   timeout_(5000),
   hAcqDesc(NULL)
{
   // call the base class method to set-up default error codes/messages
   InitializeDefaultErrorMessages();
   pEVA_NDE_PerkinElmerResourceLock_ = new MMThreadLock();
   thd_ = new MySequenceThread(this);

   // parent ID display
   CreateHubIDProperty();
}

/**
* CEVA_NDE_PerkinElmerFPD destructor.
* If this device used as intended within the Micro-Manager system,
* Shutdown() will be always called before the destructor. But in any case
* we need to make sure that all resources are properly released even if
* Shutdown() was not called.
*/
CEVA_NDE_PerkinElmerFPD::~CEVA_NDE_PerkinElmerFPD()
{
   StopSequenceAcquisition();
   delete thd_;
   delete pEVA_NDE_PerkinElmerResourceLock_;
}

/**
* Obtains device name.
* Required by the MM::Device API.
*/
void CEVA_NDE_PerkinElmerFPD::GetName(char* name) const
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
int CEVA_NDE_PerkinElmerFPD::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   if (pHub)
   {
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
   nRet = CreateProperty(MM::g_Keyword_Description, "EVA_NDE_PerkinElmer Camera Device Adapter", MM::String, true);
   if (DEVICE_OK != nRet)
      return nRet;

   // CameraName
   nRet = CreateProperty(MM::g_Keyword_CameraName, g_CameraDeviceName, MM::String, true);
   assert(nRet == DEVICE_OK);

   // CameraID
   nRet = CreateProperty(MM::g_Keyword_CameraID, "V1.0", MM::String, true);
   assert(nRet == DEVICE_OK);

   // binning
   CPropertyAction *pAct = new CPropertyAction (this, &CEVA_NDE_PerkinElmerFPD::OnBinning);
   nRet = CreateProperty(MM::g_Keyword_Binning, "1", MM::Integer, false, pAct);
   assert(nRet == DEVICE_OK);

   nRet = SetAllowedBinning();
   if (nRet != DEVICE_OK)
      return nRet;

   // exposure
   pAct = new CPropertyAction (this, &CEVA_NDE_PerkinElmerFPD::OnExposure);
   nRet = CreateProperty(MM::g_Keyword_Exposure, "10.0", MM::Float, false,pAct);
   assert(nRet == DEVICE_OK);
   SetPropertyLimits(MM::g_Keyword_Exposure, 0, 5000);  //limit to 5s

	// Acquisition mode
   pAct = new CPropertyAction (this, &CEVA_NDE_PerkinElmerFPD::OnAcquisitionMode);
   nRet = CreateProperty("AcquisitionMode", g_acquisitionMode_0, MM::String, false, pAct);
   assert(nRet == DEVICE_OK);
   AddAllowedValue("AcquisitionMode",g_acquisitionMode_0);
   AddAllowedValue("AcquisitionMode",g_acquisitionMode_1);
   AddAllowedValue("AcquisitionMode",g_acquisitionMode_2);
   AddAllowedValue("AcquisitionMode",g_acquisitionMode_3);

	// Trigger mode
   pAct = new CPropertyAction (this, &CEVA_NDE_PerkinElmerFPD::OnTriggerMode);
   nRet = CreateProperty("TriggerMode", g_TriggerMode_0, MM::String, false, pAct);
   assert(nRet == DEVICE_OK);
   AddAllowedValue("TriggerMode",g_TriggerMode_0);
   AddAllowedValue("TriggerMode",g_TriggerMode_1);
   AddAllowedValue("TriggerMode",g_TriggerMode_2);
   AddAllowedValue("TriggerMode",g_TriggerMode_3);

	// Frame Timing
   pAct = new CPropertyAction (this, &CEVA_NDE_PerkinElmerFPD::OnFrameTiming);
   nRet = CreateProperty("FrameTiming", g_FrameTime_TIMING_0, MM::String, false, pAct);
   assert(nRet == DEVICE_OK);
   AddAllowedValue("FrameTiming",g_FrameTime_TIMING_0);
   AddAllowedValue("FrameTiming",g_FrameTime_TIMING_1);
   AddAllowedValue("FrameTiming",g_FrameTime_TIMING_2);
   AddAllowedValue("FrameTiming",g_FrameTime_TIMING_3);
   AddAllowedValue("FrameTiming",g_FrameTime_TIMING_4);
   AddAllowedValue("FrameTiming",g_FrameTime_TIMING_5);
   AddAllowedValue("FrameTiming",g_FrameTime_TIMING_6);
   AddAllowedValue("FrameTiming",g_FrameTime_TIMING_7);

   // camera gain
   nRet = CreateProperty(MM::g_Keyword_Gain, "0", MM::Integer, false);
   assert(nRet == DEVICE_OK);
   SetPropertyLimits(MM::g_Keyword_Gain, 0, 5);

   // camera offset
   nRet = CreateProperty(MM::g_Keyword_Offset, "0", MM::Integer, false);
   assert(nRet == DEVICE_OK);

   // Whether or not to use exposure time sequencing
   pAct = new CPropertyAction (this, &CEVA_NDE_PerkinElmerFPD::OnIsSequenceable);
   std::string propName = "UseExposureSequences";
   CreateProperty(propName.c_str(), "No", MM::String, false, pAct);
   AddAllowedValue(propName.c_str(), "Yes");
   AddAllowedValue(propName.c_str(), "No");

   // Camera Status
   //pAct = new CPropertyAction (this, &CEVA_NDE_PerkinElmerFPD::OnStatus);
   std::string statusPropName = "Status";
   CreateProperty(statusPropName.c_str(), "Idle", MM::String, false);


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

   // initialize image buffer
   GenerateEmptyImage(img_);

   nRet = init(hAcqDesc);
   if(nRet != HIS_ALL_OK)
		return DEVICE_ERR;

   nRet = openDevice(hAcqDesc);
   if(nRet != HIS_ALL_OK)
		return DEVICE_ERR;
  
  image_width = dwColumns;
  image_height = dwRows;
  ResizeImageBuffer();

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
int CEVA_NDE_PerkinElmerFPD::Shutdown()
{
   ////EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   closeDevice();

   initialized_ = false;
   return DEVICE_OK;
}

/**
* Performs exposure and grabs a single image.
* This function should block during the actual exposure and return immediately afterwards 
* (i.e., before readout).  This behavior is needed for proper synchronization with the shutter.
* Required by the MM::Camera API.
*/
int CEVA_NDE_PerkinElmerFPD::SnapImage()
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;
   int ret = DEVICE_ERR;
	static int callCounter = 0;
	++callCounter;

    MMThreadGuard g(imgPixelsLock_);
	
    unsigned short* pBuf = (unsigned short*)GetImageBuffer();
	ret = acquireImage(hAcqDesc,pBuf,1,exposureMs_);
	if(ret != HIS_ALL_OK)
		return DEVICE_ERR;
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
const unsigned char* CEVA_NDE_PerkinElmerFPD::GetImageBuffer()
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   MMThreadGuard g(imgPixelsLock_);
   unsigned char *pB = (unsigned char*)(img_.GetPixels());
   return pB;
}

/**
* Returns image buffer X-size in pixels.
* Required by the MM::Camera API.
*/
unsigned CEVA_NDE_PerkinElmerFPD::GetImageWidth() const
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   return img_.Width();
}

/**
* Returns image buffer Y-size in pixels.
* Required by the MM::Camera API.
*/
unsigned CEVA_NDE_PerkinElmerFPD::GetImageHeight() const
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   return img_.Height();
}

/**
* Returns image buffer pixel depth in bytes.
* Required by the MM::Camera API.
*/
unsigned CEVA_NDE_PerkinElmerFPD::GetImageBytesPerPixel() const
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   return img_.Depth();
} 

/**
* Returns the bit depth (dynamic range) of the pixel.
* This does not affect the buffer size, it just gives the client application
* a guideline on how to interpret pixel values.
* Required by the MM::Camera API.
*/
unsigned CEVA_NDE_PerkinElmerFPD::GetBitDepth() const
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   return bitDepth_;
}

/**
* Returns the size in bytes of the image buffer.
* Required by the MM::Camera API.
*/
long CEVA_NDE_PerkinElmerFPD::GetImageBufferSize() const
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

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
* This EVA_NDE_PerkinElmer implementation ignores the position coordinates and just crops the buffer.
* @param x - top-left corner coordinate
* @param y - top-left corner coordinate
* @param xSize - width
* @param ySize - height
*/
int CEVA_NDE_PerkinElmerFPD::SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize)
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
    //  return HUB_UNKNOWN_ERR;

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
int CEVA_NDE_PerkinElmerFPD::GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize)
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

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
int CEVA_NDE_PerkinElmerFPD::ClearROI()
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   ResizeImageBuffer();
   roiX_ = 0;
   roiY_ = 0;
      
   return DEVICE_OK;
}

/**
* Returns the current exposure setting in milliseconds.
* Required by the MM::Camera API.
*/
double CEVA_NDE_PerkinElmerFPD::GetExposure() const
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;
    return exposureMs_;
}

/**
 * Returns the current exposure from a sequence and increases the sequence counter
 * Used for exposure sequences
 */
double CEVA_NDE_PerkinElmerFPD::GetSequenceExposure() 
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
void CEVA_NDE_PerkinElmerFPD::SetExposure(double exp)
{
   SetProperty(MM::g_Keyword_Exposure, CDeviceUtils::ConvertToString(exp));

}

/**
* Returns the current binning factor.
* Required by the MM::Camera API.
*/
int CEVA_NDE_PerkinElmerFPD::GetBinning() const
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

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
int CEVA_NDE_PerkinElmerFPD::SetBinning(int binF)
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   return SetProperty(MM::g_Keyword_Binning, CDeviceUtils::ConvertToString(binF));
}

/**
 * Clears the list of exposures used in sequences
 */
int CEVA_NDE_PerkinElmerFPD::ClearExposureSequence()
{
   exposureSequence_.clear();
   return DEVICE_OK;
}

/**
 * Adds an exposure to a list of exposures used in sequences
 */
int CEVA_NDE_PerkinElmerFPD::AddToExposureSequence(double exposureTime_ms) 
{
   exposureSequence_.push_back(exposureTime_ms);
   return DEVICE_OK;
}

int CEVA_NDE_PerkinElmerFPD::SetAllowedBinning() 
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   vector<string> binValues;
   binValues.push_back("1");
   binValues.push_back("2");
   binValues.push_back("3");
   binValues.push_back("4");
   binValues.push_back("5");
   LogMessage("Setting Allowed Binning settings", true);
   return SetAllowedValues(MM::g_Keyword_Binning, binValues);
}


/**
 * Required by the MM::Camera API
 * Please implement this yourself and do not rely on the base class implementation
 * The Base class implementation is deprecated and will be removed shortly
 */
int CEVA_NDE_PerkinElmerFPD::StartSequenceAcquisition(double interval) {
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   return StartSequenceAcquisition(LONG_MAX, interval, false);            
}

/**                                                                       
* Stop and wait for the Sequence thread finished                                   
*/                                                                        
int CEVA_NDE_PerkinElmerFPD::StopSequenceAcquisition()                                     
{
   if (IsCallbackRegistered())
   {
      //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
      //if (!pHub)
      //   return HUB_UNKNOWN_ERR;
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
int CEVA_NDE_PerkinElmerFPD::StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow)
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

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
int CEVA_NDE_PerkinElmerFPD::InsertImage()
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

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
int CEVA_NDE_PerkinElmerFPD::ThreadRun (MM::MMTime startTime)
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   int ret=DEVICE_ERR;

   ret = SnapImage();
   if (ret != DEVICE_OK)
   {
      return ret;
   }
   while(!_isReady) ;
   ret = InsertImage();

   if (ret != DEVICE_OK)
   {
      return ret;
   }
   return ret;
};

bool CEVA_NDE_PerkinElmerFPD::IsCapturing() {
   return !thd_->IsStopped();
}

/*
 * called from the thread function before exit 
 */
void CEVA_NDE_PerkinElmerFPD::OnThreadExiting() throw()
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


MySequenceThread::MySequenceThread(CEVA_NDE_PerkinElmerFPD* pCam)
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
// CEVA_NDE_PerkinElmerFPD Action handlers
///////////////////////////////////////////////////////////////////////////////



//int CEVA_NDE_PerkinElmerFPD::OnSwitch(MM::PropertyBase* pProp, MM::ActionType eAct)
//{
   // use cached values
//   return DEVICE_OK;
//}

/**
* Handles "Binning" property.
*/
int CEVA_NDE_PerkinElmerFPD::OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   int ret = DEVICE_ERR;
   switch(eAct)
   {
   case MM::AfterSet:
      {
         if(IsCapturing())
            return DEVICE_CAMERA_BUSY_ACQUIRING;

         // the user just set the new value for the property, so we have to
         // apply this value to the 'hardware'.
         pProp->Get(binMode_);
		ResizeImageBuffer();
		ret=DEVICE_OK;
      }
	  break;
   case MM::BeforeGet:
      {
         ret=DEVICE_OK;
			pProp->Set(binMode_);
      }break;
   }
   return ret; 
}
/**
* Handles "Binning" property.
*/
int CEVA_NDE_PerkinElmerFPD::OnExposure(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   int ret = DEVICE_ERR;
   switch(eAct)
   {
   case MM::AfterSet:
      {
         pProp->Get(exposureMs_);
		 GetCoreCallback()->OnExposureChanged(this, exposureMs_);
		 ret=DEVICE_OK;
      }
	  break;
   case MM::BeforeGet:
      {
         ret=DEVICE_OK;
		 pProp->Set(exposureMs_);
      }break;
   }
   return ret; 
}

/*
* Handles "AcquisitionMode" property.
* Changes allowed Binning values to test whether the UI updates properly
*/
int CEVA_NDE_PerkinElmerFPD::OnAcquisitionMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{ 
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;
   if (eAct == MM::AfterSet) {
	  std::string tmp;
      pProp->Get(tmp);
	  if(tmp.compare(g_acquisitionMode_0)==0)
		  acquisitionMode_ = 0;
	  else if(tmp.compare(g_acquisitionMode_1)==0)
		  acquisitionMode_ = 1;
	  else if(tmp.compare(g_acquisitionMode_2)==0)
		  acquisitionMode_ = 2;
	  else if(tmp.compare(g_acquisitionMode_3)==0)
		  acquisitionMode_ = 3;
	  else
		  acquisitionMode_ = 0;

      if (initialized_) {
         int ret = OnPropertiesChanged();
         if (ret != DEVICE_OK)
            return ret;
      }
   } else if (eAct == MM::BeforeGet) {
	  std::string tmp;
	  switch(acquisitionMode_)
	  {
	  case 0:
		  tmp.assign(g_acquisitionMode_0);
		  break;
	  case 1:
		  tmp.assign(g_acquisitionMode_1);
		  break;
	  case 2:
		  tmp.assign(g_acquisitionMode_2);
		  break;
	  case 3:
		  tmp.assign(g_acquisitionMode_3);
		  break;
	  default:
		  tmp.assign("UNDEFINED");
	  }
	  pProp->Set(tmp.c_str());
   }
   return DEVICE_OK;
}

/*
* Handles "TriggerMode" property.
* Changes allowed Binning values to test whether the UI updates properly
*/
int CEVA_NDE_PerkinElmerFPD::OnTriggerMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{ 
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   if (eAct == MM::AfterSet) {
	  std::string tmp;
      pProp->Get(tmp);
	  if(tmp.compare(g_TriggerMode_0)==0)
		  triggerMode_ = 0;
	  else if(tmp.compare(g_TriggerMode_1)==0)
		  triggerMode_ = 1;
	  else if(tmp.compare(g_TriggerMode_2)==0)
		  triggerMode_ = 2;
	  else if(tmp.compare(g_TriggerMode_3)==0)
		  triggerMode_ = 3;
	  else
		  triggerMode_ = 0;

   } else if (eAct == MM::BeforeGet) {
	  std::string tmp;
	  switch(triggerMode_)
	  {
	  case 0:
		  tmp.assign(g_TriggerMode_0);
		  break;
	  case 1:
		  tmp.assign(g_TriggerMode_1);
		  break;
	  case 2:
		  tmp.assign(g_TriggerMode_2);
		  break;
	  case 3:
		  tmp.assign(g_TriggerMode_3);
		  break;
	  default:
		  tmp.assign("UNDEFINED");
	  }
	  pProp->Set(tmp.c_str());
   }
   return DEVICE_OK;
}
/*
* Handles "FrameTime" property.
* Changes allowed Binning values to test whether the UI updates properly
*/
int CEVA_NDE_PerkinElmerFPD::OnFrameTiming(MM::PropertyBase* pProp, MM::ActionType eAct)
{ 
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

   if (eAct == MM::AfterSet) {
	  std::string tmp;
      pProp->Get(tmp);
	  if(tmp.compare(g_FrameTime_TIMING_0)==0)
		  frameTiming_ = 0;
	  else if(tmp.compare(g_FrameTime_TIMING_1)==0)
		  frameTiming_ = 1;
	  else if(tmp.compare(g_FrameTime_TIMING_2)==0)
		  frameTiming_ = 2;
	  else if(tmp.compare(g_FrameTime_TIMING_3)==0)
		  frameTiming_ = 3;
	  else if(tmp.compare(g_FrameTime_TIMING_4)==0)
		  frameTiming_ = 4;
	  else if(tmp.compare(g_FrameTime_TIMING_5)==0)
		  frameTiming_ = 5;
	  else if(tmp.compare(g_FrameTime_TIMING_6)==0)
		  frameTiming_ = 6;
	  else if(tmp.compare(g_FrameTime_TIMING_7)==0)
		  frameTiming_ = 7;
	  else
		  frameTiming_ = 0;

      if (initialized_) {
         int ret = OnPropertiesChanged();
         if (ret != DEVICE_OK)
            return ret;
      }
   } else if (eAct == MM::BeforeGet) {
	  std::string tmp;
	  switch(frameTiming_)
	  {
	  case 0:
		  tmp.assign(g_FrameTime_TIMING_0);
		  break;
	  case 1:
		  tmp.assign(g_FrameTime_TIMING_1);
		  break;
	  case 2:
		  tmp.assign(g_FrameTime_TIMING_2);
		  break;
	  case 3:
		  tmp.assign(g_FrameTime_TIMING_3);
		  break;
	  case 4:
		  tmp.assign(g_FrameTime_TIMING_4);
		  break;
	  case 5:
		  tmp.assign(g_FrameTime_TIMING_5);
		  break;
	  case 6:
		  tmp.assign(g_FrameTime_TIMING_6);
		  break;
	  case 7:
		  tmp.assign(g_FrameTime_TIMING_7);
		  break;
	  default:
		  tmp.assign("UNDEFINED");
	  }
	  pProp->Set(tmp.c_str());
   }
   return DEVICE_OK;
}

int CEVA_NDE_PerkinElmerFPD::OnTriggerDevice(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

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


int CEVA_NDE_PerkinElmerFPD::OnIsSequenceable(MM::PropertyBase* pProp, MM::ActionType eAct)
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
// Private CEVA_NDE_PerkinElmerFPD methods
///////////////////////////////////////////////////////////////////////////////

/**
* Sync internal image buffer size to the chosen property values.
*/
int CEVA_NDE_PerkinElmerFPD::ResizeImageBuffer()
{
	long binSizeX_;
	long binSizeY_;
	switch(binMode_)
	{
		case 1:
			binSizeX_ = 1; binSizeY_ =1;
			break;
		case 2:
			binSizeX_ = 2; binSizeY_ =2;
			break;
		case 3:
			binSizeX_ = 4; binSizeY_ =4;
			break;
		case 4:
			binSizeX_ = 1; binSizeY_ =2;
			break;
		case 5:
			binSizeX_ = 1; binSizeY_ =4;
			break;
	}
	img_.Resize(image_width/binSizeX_, image_height/binSizeY_,byteDepth_);
   return DEVICE_OK;
}

void CEVA_NDE_PerkinElmerFPD::GenerateEmptyImage(ImgBuffer& img)
{
   MMThreadGuard g(imgPixelsLock_);
   if (img.Height() == 0 || img.Width() == 0 || img.Depth() == 0)
      return;
   unsigned char* pBuf = const_cast<unsigned char*>(img.GetPixels());
   memset(pBuf, 0, img.Height()*img.Width()*img.Depth());
}




void CEVA_NDE_PerkinElmerFPD::TestResourceLocking(const bool recurse)
{
   MMThreadGuard g(*pEVA_NDE_PerkinElmerResourceLock_);
   if(recurse)
      TestResourceLocking(false);
}




int TransposeProcessor::Initialize()
{
   EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   if (pHub)
   {
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
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
    //  return HUB_UNKNOWN_ERR;

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
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

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


int EVA_NDE_PerkinElmerHub::Initialize()
{
   //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
   //if (!pHub)
   //   return HUB_UNKNOWN_ERR;

  	initialized_ = true;
 
	return DEVICE_OK;
}

int EVA_NDE_PerkinElmerHub::DetectInstalledDevices()
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

MM::Device* EVA_NDE_PerkinElmerHub::CreatePeripheralDevice(const char* adapterName)
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


void EVA_NDE_PerkinElmerHub::GetName(char* pName) const
{
   CDeviceUtils::CopyLimitedString(pName, g_HubDeviceName);
}

int EVA_NDE_PerkinElmerHub::OnErrorRate(MM::PropertyBase* pProp, MM::ActionType eAct)
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

int EVA_NDE_PerkinElmerHub::OnDivideOneByMe(MM::PropertyBase* pProp, MM::ActionType eAct)
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


