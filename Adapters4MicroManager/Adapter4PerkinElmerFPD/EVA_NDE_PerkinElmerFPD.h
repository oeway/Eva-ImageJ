///////////////////////////////////////////////////////////////////////////////
// FILE:          EVA_NDE_PerkinElmerFPD.h
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
//                Karl Hoover (stuff such as programmable CCD size  & the various image processors)
//                Arther Edelstein ( equipment error simulation)
//
// COPYRIGHT:     University of California, San Francisco, 2006
//                100X Imaging Inc, 2008
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
// CVS:           $Id: EVA_NDE_PerkinElmerFPD.h 10842 2013-04-24 01:21:05Z mark $
//

#ifndef _EVA_NDE_PerkinElmerFPD_H_
#define _EVA_NDE_PerkinElmerFPD_H_

#include "../../MMDevice/DeviceBase.h"
#include "../../MMDevice/ImgBuffer.h"
#include "../../MMDevice/DeviceThreads.h"
#include <string>
#include <map>
#include <algorithm>
#include "Acq.h"
//////////////////////////////////////////////////////////////////////////////
// Error codes
//
#define ERR_UNKNOWN_MODE         102
#define ERR_IN_SEQUENCE          104
#define ERR_SEQUENCE_INACTIVE    105
#define HUB_UNKNOWN_ERR        107
#define HUB_NOT_AVAILABLE        107

const char* NoHubError = "Parent Hub not defined.";

////////////////////////
// EVA_NDE_PerkinElmerHub
//////////////////////

class EVA_NDE_PerkinElmerHub : public HubBase<EVA_NDE_PerkinElmerHub>
{
public:
   EVA_NDE_PerkinElmerHub():initialized_(false), busy_(false), errorRate_(0.0), divideOneByMe_(1) {} ;
   ~EVA_NDE_PerkinElmerHub() {};

   // Device API
   // ---------
   int Initialize();
   int Shutdown() {return DEVICE_OK;};
   void GetName(char* pName) const; 
   bool Busy() { return busy_;} ;
   bool GenerateRandomError();

   // HUB api
   int DetectInstalledDevices();
   MM::Device* CreatePeripheralDevice(const char* adapterName);

   // action interface
   int OnErrorRate(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnDivideOneByMe(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   void GetPeripheralInventory();

   bool busy_;
   bool initialized_;
   std::vector<std::string> peripherals_;
   double errorRate_;
   long divideOneByMe_;
};


//////////////////////////////////////////////////////////////////////////////
// CEVA_NDE_PerkinElmerFPD class
// Simulation of the Camera device
//////////////////////////////////////////////////////////////////////////////

class MySequenceThread;

class CEVA_NDE_PerkinElmerFPD : public CCameraBase<CEVA_NDE_PerkinElmerFPD>  
{
public:
   CEVA_NDE_PerkinElmerFPD();
   ~CEVA_NDE_PerkinElmerFPD();
  
   // MMDevice API
   // ------------
   int Initialize();
   int Shutdown();
  
   void GetName(char* name) const;      
   
   // MMCamera API
   // ------------
   int SnapImage();
   const unsigned char* GetImageBuffer();
   unsigned GetImageWidth() const;
   unsigned GetImageHeight() const;
   unsigned GetImageBytesPerPixel() const;
   unsigned GetBitDepth() const;
   long GetImageBufferSize() const;
   double GetExposure() const;
   void SetExposure(double exp);
   int SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize); 
   int GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize); 
   int ClearROI();
   int PrepareSequenceAcqusition()
   {
      //EVA_NDE_PerkinElmerHub* pHub = static_cast<EVA_NDE_PerkinElmerHub*>(GetParentHub());
      //if (!pHub)
      //   return HUB_UNKNOWN_ERR;
      return DEVICE_OK;
   }
   int StartSequenceAcquisition(double interval);
   int StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow);
   int StopSequenceAcquisition();
   int InsertImage();
   int ThreadRun(MM::MMTime startTime);
   bool IsCapturing();
   void OnThreadExiting() throw(); 
   double GetNominalPixelSizeUm() const {return nominalPixelSizeUm_;}
   double GetPixelSizeUm() const {return nominalPixelSizeUm_ * GetBinning();}
   int GetBinning() const;
   int SetBinning(int bS);
   int IsExposureSequenceable(bool& isSequenceable) const 
   {
      isSequenceable = isSequenceable_;
      return DEVICE_OK;
   }
   int GetExposureSequenceMaxLength(long& nrEvents) 
   {
      nrEvents = sequenceMaxLength_;
      return DEVICE_OK;
   }
   int StartExposureSequence() 
   {
      // may need thread lock
      sequenceRunning_ = true;
      return DEVICE_OK;
   }
   int StopExposureSequence() 
   {
      // may need thread lock
      sequenceRunning_ = false; 
      sequenceIndex_ = 0;
      return DEVICE_OK;
   }
   // Remove all values in the sequence                                   
   int ClearExposureSequence();
   // Add one value to the sequence                                       
   int AddToExposureSequence(double exposureTime_ms);
   // Signal that we are done sending sequence values so that the adapter can send the whole sequence to the device
   int SendExposureSequence() const {
      return DEVICE_OK;
   }

   unsigned  GetNumberOfComponents() const { return 1;};

   // action interface
   // ----------------
	// floating point read-only properties for testing

	//int OnSwitch(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnExposure(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAcquisitionMode(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnTriggerDevice(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnIsSequenceable(MM::PropertyBase* pProp, MM::ActionType eAct);
	int CEVA_NDE_PerkinElmerFPD::OnFrameTiming(MM::PropertyBase* pProp, MM::ActionType eAct);
	int CEVA_NDE_PerkinElmerFPD::OnTriggerMode(MM::PropertyBase* pProp, MM::ActionType eAct);
private:
   int SetAllowedBinning();
   void TestResourceLocking(const bool);
   void GenerateEmptyImage(ImgBuffer& img);
   int ResizeImageBuffer();
   static const double nominalPixelSizeUm_;

   unsigned int	image_width;			// image width
   unsigned int	image_height;			// image height
   ImgBuffer img_;
   bool busy_;
   bool stopOnOverFlow_;
   bool initialized_;
   long timeout_;
   double exposureMs_;
   long acquisitionMode_;
   long frameTiming_;
   int bitDepth_;
   unsigned roiX_;
   unsigned roiY_;
   MM::MMTime sequenceStartTime_;
   bool isSequenceable_;
   long sequenceMaxLength_;
   bool sequenceRunning_;
   unsigned long sequenceIndex_;
   double GetSequenceExposure();
   std::vector<double> exposureSequence_;
   long imageCounter_;
	long binMode_;
	long byteDepth_;
	std::string triggerDevice_;
	long triggerMode_;

   bool stopOnOverflow_;

   HACQDESC hAcqDesc;
   MMThreadLock* pEVA_NDE_PerkinElmerResourceLock_;
   MMThreadLock imgPixelsLock_;
   friend class MySequenceThread;
   MySequenceThread * thd_;

};

class MySequenceThread : public MMDeviceThreadBase
{
   friend class CEVA_NDE_PerkinElmerFPD;
   enum { default_numImages=1, default_intervalMS = 100 };
   public:
      MySequenceThread(CEVA_NDE_PerkinElmerFPD* pCam);
      ~MySequenceThread();
      void Stop();
      void Start(long numImages, double intervalMs);
      bool IsStopped();
      void Suspend();
      bool IsSuspended();
      void Resume();
      double GetIntervalMs(){return intervalMs_;}                               
      void SetLength(long images) {numImages_ = images;}                        
      long GetLength() const {return numImages_;}
      long GetImageCounter(){return imageCounter_;}                             
      MM::MMTime GetStartTime(){return startTime_;}                             
      MM::MMTime GetActualDuration(){return actualDuration_;}
   private:                                                                     
      int svc(void) throw();
      CEVA_NDE_PerkinElmerFPD* camera_;                                                     
      bool stop_;                                                               
      bool suspend_;                                                            
      long numImages_;                                                          
      long imageCounter_;                                                       
      double intervalMs_;                                                       
      MM::MMTime startTime_;                                                    
      MM::MMTime actualDuration_;                                               
      MM::MMTime lastFrameTime_;                                                
      MMThreadLock stopLock_;                                                   
      MMThreadLock suspendLock_;                                                
}; 

//////////////////////////////////////////////////////////////////////////////
// TransposeProcessor class
// transpose an image
// K.H.
//////////////////////////////////////////////////////////////////////////////
class TransposeProcessor : public CImageProcessorBase<TransposeProcessor>
{
public:
   TransposeProcessor () : inPlace_ (false), pTemp_(NULL), tempSize_(0), busy_(false)
   {
      // parent ID display
      CreateHubIDProperty();
   }
   ~TransposeProcessor () {if( NULL!= pTemp_) free(pTemp_); tempSize_=0;  }

   int Shutdown() {return DEVICE_OK;}
   void GetName(char* name) const {strcpy(name,"TransposeProcessor");}

   int Initialize();

   bool Busy(void) { return busy_;};

    // really primative image transpose algorithm which will work fine for non-square images... 
   template <typename PixelType> int TransposeRectangleOutOfPlace( PixelType* pI, unsigned int width, unsigned int height)
   {
      int ret = DEVICE_OK;
      unsigned long tsize = width*height*sizeof(PixelType);
      if( this->tempSize_ != tsize)
      {
         if( NULL != this->pTemp_)
         {
            free(pTemp_);
            pTemp_ = NULL;
         }
         pTemp_ = (PixelType *)malloc(tsize);
      }
      if( NULL != pTemp_)
      {
         PixelType* pTmpImage = (PixelType *) pTemp_;
         tempSize_ = tsize;
         for( unsigned long ix = 0; ix < width; ++ix)
         {
            for( unsigned long iy = 0; iy < height; ++iy)
            {
               pTmpImage[iy + ix*width] = pI[ ix + iy*height];
            }
         }
         memcpy( pI, pTmpImage, tsize);
      }
      else
      {
         ret = DEVICE_ERR;
      }

      return ret;
   }

   
   template <typename PixelType> void TransposeSquareInPlace( PixelType* pI, unsigned int dim) 
   { 
      PixelType tmp;
      for( unsigned long ix = 0; ix < dim; ++ix)
      {
         for( unsigned long iy = ix; iy < dim; ++iy)
         {
            tmp = pI[iy*dim + ix];
            pI[iy*dim +ix] = pI[ix*dim + iy];
            pI[ix*dim +iy] = tmp; 
         }
      }

      return;
   }

   int Process(unsigned char* buffer, unsigned width, unsigned height, unsigned byteDepth);

   // action interface
   // ----------------
   int OnInPlaceAlgorithm(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   bool inPlace_;
   void* pTemp_;
   unsigned long tempSize_;
   bool busy_;
};



//////////////////////////////////////////////////////////////////////////////
// ImageFlipX class
// flip an image
// K.H.
//////////////////////////////////////////////////////////////////////////////
class ImageFlipX : public CImageProcessorBase<ImageFlipX>
{
public:
   ImageFlipX () :  busy_(false) {}
   ~ImageFlipX () {  }

   int Shutdown() {return DEVICE_OK;}
   void GetName(char* name) const {strcpy(name,"ImageFlipX");}

   int Initialize();
   bool Busy(void) { return busy_;};

    // 
   template <typename PixelType> int Flip( PixelType* pI, unsigned int width, unsigned int height)
   {
      PixelType tmp;
      int ret = DEVICE_OK;
      for( unsigned long iy = 0; iy < height; ++iy)
      {
         for( unsigned long ix = 0; ix <  (width>>1) ; ++ix)
         {
            tmp = pI[ ix + iy*width];
            pI[ ix + iy*width] = pI[ width - 1 - ix + iy*width];
            pI[ width -1 - ix + iy*width] = tmp;
         }
      }
      return ret;
   }

   int Process(unsigned char* buffer, unsigned width, unsigned height, unsigned byteDepth);

   int OnPerformanceTiming(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   bool busy_;
   MM::MMTime performanceTiming_;
};


//////////////////////////////////////////////////////////////////////////////
// ImageFlipY class
// flip an image
// K.H.
//////////////////////////////////////////////////////////////////////////////
class ImageFlipY : public CImageProcessorBase<ImageFlipY>
{
public:
   ImageFlipY () : busy_(false), performanceTiming_(0.) {}
   ~ImageFlipY () {  }

   int Shutdown() {return DEVICE_OK;}
   void GetName(char* name) const {strcpy(name,"ImageFlipY");}

   int Initialize();
   bool Busy(void) { return busy_;};

   template <typename PixelType> int Flip( PixelType* pI, unsigned int width, unsigned int height)
   {
      PixelType tmp;
      int ret = DEVICE_OK;
      for( unsigned long ix = 0; ix < width ; ++ix)
      {
         for( unsigned long iy = 0; iy < (height>>1); ++iy)
         {
            tmp = pI[ ix + iy*width];
            pI[ ix + iy*width] = pI[ ix + (height - 1 - iy)*width];
            pI[ ix + (height - 1 - iy)*width] = tmp;
         }
      }
      return ret;
   }


   int Process(unsigned char* buffer, unsigned width, unsigned height, unsigned byteDepth);

   // action interface
   // ----------------
   int OnPerformanceTiming(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   bool busy_;
   MM::MMTime performanceTiming_;

};



//////////////////////////////////////////////////////////////////////////////
// MedianFilter class
// apply Median filter an image
// K.H.
//////////////////////////////////////////////////////////////////////////////
class MedianFilter : public CImageProcessorBase<MedianFilter>
{
public:
   MedianFilter () : busy_(false), performanceTiming_(0.),pSmoothedIm_(0), sizeOfSmoothedIm_(0)
   {
      // parent ID display
      CreateHubIDProperty();
   };
   ~MedianFilter () { if(0!=pSmoothedIm_) free(pSmoothedIm_); };

   int Shutdown() {return DEVICE_OK;}
   void GetName(char* name) const {strcpy(name,"MedianFilter");}

   int Initialize();
   bool Busy(void) { return busy_;};

   // NOTE: this utility MODIFIES the argument, make a copy yourself if you want the original data preserved
   template <class U> U FindMedian(std::vector<U>& values ) {
      std::sort(values.begin(), values.end());
      return values[(values.size())>>1];
   };


   template <typename PixelType> int Filter( PixelType* pI, unsigned int width, unsigned int height)
   {
      int ret = DEVICE_OK;
      int x[9];
      int y[9];

      const unsigned long thisSize = sizeof(*pI)*width*height;
      if( thisSize != sizeOfSmoothedIm_)
      {
         if(NULL!=pSmoothedIm_)
         {
            sizeOfSmoothedIm_ = 0;
            free(pSmoothedIm_);
         }
         // malloc is faster than new...
         pSmoothedIm_ = (PixelType*)malloc(thisSize);
         if(NULL!=pSmoothedIm_)
         {
            sizeOfSmoothedIm_ = thisSize;
         }
      }

      PixelType* pSmooth = (PixelType*) pSmoothedIm_;

      if(NULL != pSmooth)
      {
      /*Apply 3x3 median filter to reduce shot noise*/
      for (unsigned int i=0; i<width; i++) {
         for (unsigned int j=0; j<height; j++) {
            x[0]=i-1;
            y[0]=(j-1);
            x[1]=i;
            y[1]=(j-1);
            x[2]=i+1;
            y[2]=(j-1);
            x[3]=i-1;
            y[3]=(j);
            x[4]=i;
            y[4]=(j);
            x[5]=i+1;
            y[5]=(j);
            x[6]=i-1;
            y[6]=(j+1);
            x[7]=i;
            y[7]=(j+1);
            x[8]=i+1;
            y[8]=(j+1);
            // truncate the median filter window  -- duplicate edge points
            // this could be more efficient, we could fill in the interior image [1,w0-1]x[1,h0-1] then explicitly fill in the edge pixels.
            // also the temporary image could be as small as 2 rasters of the image
            for(int ij =0; ij < 9; ++ij)
            {
               if( x[ij] < 0)
                  x[ij] = 0;
               else if( int(width-1) < x[ij])
                  x[ij] = int(width-1);
               if( y[ij] < 0)
                  y[ij] = 0;
               else if( int(height-1) < y[ij])
                  y[ij] = (int)(height-1);
            }
            std::vector<PixelType> windo;
            for(int ij = 0; ij < 9; ++ij)
            {
               windo.push_back(pI[ x[ij] + width*y[ij]]);
            }
            pSmooth[i + j*width] = FindMedian(windo);
         }
      }

      memcpy( pI, pSmoothedIm_, thisSize);
      }
      else
         ret = DEVICE_ERR;

      return ret;
   }
   int Process(unsigned char* buffer, unsigned width, unsigned height, unsigned byteDepth);

   // action interface
   // ----------------
   int OnPerformanceTiming(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   bool busy_;
   MM::MMTime performanceTiming_;
   void*  pSmoothedIm_;
   unsigned long sizeOfSmoothedIm_;
   


};







#endif //_EVA_NDE_PerkinElmerFPD_H_
