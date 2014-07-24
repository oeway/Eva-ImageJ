#include "../../MMCore/MMCore.h"
#include "../Common/PropertyTypes.h"
#include "../Common/DeviceTypes.h"
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <sstream>
#include <conio.h>
using namespace std;
int main(int argc, char* argv[])
{
   // get module and device names
   if (argc < 3)
   {
      cout << "Error. Module and/or device name not specified!" << endl;
      cout << "ModuleTest <module_name> <device_name>" << endl;
      return 1;
   }
   else if (argc > 3)
   {
      cout << "Error. Too many parameters!" << endl;
      cout << "ModuleTest <module_name> <device_name>" << endl;
      return 1;
   }
   string moduleName(argv[1]);
   string deviceName(argv[2]);

      CMMCore core;
   core.enableStderrLog(false);
      string label("Device");
   try
   {
      // Initialize the device
      // ---------------------
      cout << "Loading " << deviceName << " from library " << moduleName << "..." << endl;
      core.loadDevice(label.c_str(), moduleName.c_str(), deviceName.c_str());
      cout << "Done." << endl;

      cout << "Initializing..." << endl;
      core.initializeAllDevices();
      cout << "Done." << endl;

      // Obtain device properties
      // ------------------------
      vector<string> props(core.getDevicePropertyNames(label.c_str()));
      for (unsigned i=0; i < props.size(); i++)
      {
         cout << props[i] << " (" << ::getPropertyTypeVerbose(core.getPropertyType(label.c_str(), props[i].c_str())) << ") = "
                          << core.getProperty(label.c_str(), props[i].c_str()) << endl;
      }
		 std::string val;
      // additional testing
      MM::DeviceType type = core.getDeviceType(label.c_str());
	  core.setTimeoutMs(20000);
      if (type == MM::XYStageDevice)
	  {
		  //core.setProperty(label.c_str(), "Set position XY axis (mm) (X= Y=)", "X=1 Y=1");
		  std::string val144121 = core.getProperty(label.c_str(), "Set position XY axis (mm) (X= Y=)");
		  std::string val111 = core.getProperty(label.c_str(), "VelocityX");
		  std::string val22 = core.getProperty(label.c_str(), "VelocityY");
		  //core.setProperty(label.c_str(), "VelocityX", "8.5");
		  std::string val112 = core.getProperty(label.c_str(), "VelocityX");
		  std::string val1 = core.getProperty(label.c_str(), "PositionX");
		  core.setProperty(label.c_str(), "Set position XY axis (mm) (X= Y=)", "X=0 Y=100");
		  core.waitForDevice(label.c_str());
		  std::string val1121 = core.getProperty(label.c_str(), "Set position XY axis (mm) (X= Y=)");
		  //assert(atof(val11.c_str())==1.0);
		  //assert(atof(val22.c_str())==1.0);
		  core.setProperty(label.c_str(), "PositionX", "2.0");
		  core.setProperty(label.c_str(), "PositionY", "3.0");

		  std::string  val2 = core.getProperty(label.c_str(), "PositionY");
		  core.setProperty(label.c_str(), "Set position XY axis (mm) (X= Y=)", "X=3 Y=2");
		  std::string  val3 = core.getProperty(label.c_str(), "PositionX");

		  std::string val5 = core.getProperty(label.c_str(), "Set position XY axis (mm) (X= Y=)");
		  std::string val6 = core.getProperty(label.c_str(), "VelocityX");
		  std::string val7 = core.getProperty(label.c_str(), "VelocityY");
		  //core.setProperty(label.c_str(), "VelocityX", "1.0");
		  //core.setProperty(label.c_str(), "VelocityY", "1.0");
	  }
      if (type == MM::CameraDevice)
      {		  	vector<string> tokenInput;
				char* pEnd;
				//"frames=%d complete=%d pulses=%d ready=%d",
   //      cout << "Testing camera specific functions:" << endl;
   //      core.setExposure(10.0);
		 //core.setProperty(label.c_str(), "ResetLink", "Yes");
			//	 cout << "Snap image in fluoro mode (high)  "<<endl;
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //core.snapImage();
			//	 cout << "Snap image in fluoro mode (high)  "<<endl;
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //core.snapImage();
			//	 cout << "Snap image in fluoro mode (high)  "<<endl;
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //core.snapImage();
//------------------------------rad gain calibration------------------------------------------
#ifdef RAD_GAIN_CAL
		 core.setProperty(label.c_str(), "Mode", "RAD");
		 core.setProperty(label.c_str(), "GainCalibrationStep1", "Yes");
		 //while(1) 
		 //{
			val = core.getProperty(label.c_str(), "CalibrationProgress");
			////break;
		 //}
		 cout << "Please turn off the X ray source"<<endl;
		 getch();
		 //while(!_kbhit()) Sleep (100);
		 //dark feild
		 core.setProperty(label.c_str(), "GainCalibrationStep2", "Yes");
		 while(1) 
		 {
			val =core.getProperty(label.c_str(), "CalibrationProgress");
			if(val.c_str()[val.size()-1]=='1')
			break;
		 }
		 cout << "Please turn on the X ray source"<<endl;
		 getch();
		 while(!_kbhit()) Sleep (100);
		 //flat Feild
		 core.setProperty(label.c_str(), "GainCalibrationStep3", "Yes");
		 while(1) 
		 {
			 val = core.getProperty(label.c_str(), "CalibrationProgress");
			//tokenInput.clear();
			//CDeviceUtils::Tokenize(val, tokenInput, "f=");
			//int frames = strtod(tokenInput[0].c_str(), &pEnd);
			//int compelete = strtod(tokenInput[1].c_str(), &pEnd);
			//int pulses = strtod(tokenInput[2].c_str(), &pEnd);
			//int ready = strtod(tokenInput[3].c_str(), &pEnd);
			//if(ready ==1)
			if(val.c_str()[val.size()-1]=='1')
				break;
		 }
		 //End
		 core.setProperty(label.c_str(), "GainCalibrationStep4", "Yes");
		val = core.getProperty(label.c_str(), "GainCalibrationStep4");
#endif
		#ifdef FLUORO_GAIN_CAL
//------------------------------fluoro gain calibration------------------------------------------
		 core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 core.setProperty(label.c_str(), "GainCalibrationStep1", "Yes");
		 //while(1) 
		 //{
			val = core.getProperty(label.c_str(), "CalibrationProgress");
			////break;
		 //}
		 cout << "Please turn on the X ray source"<<endl;
		 getch();
		 //flat Feild
		 core.setProperty(label.c_str(), "GainCalibrationStep2", "Yes");
		 val = core.getProperty(label.c_str(), "GainCalibrationStep2");
		 while(1) 
		 {
			val =core.getProperty(label.c_str(), "CalibrationProgress");
			//break;
			Sleep(100);
			if(val.c_str()[val.size()-1]=='1')
				break;
		 }
		 cout << "Please turn off the X ray source"<<endl;
		 getch();
		 //flat Feild
		 core.setProperty(label.c_str(), "GainCalibrationStep3", "Yes");
		 while(1) 
		 {
			 val = core.getProperty(label.c_str(), "CalibrationProgress");
			if(val.c_str()[val.size()-1]=='1')
				break;
		 }
		 //End
		 core.setProperty(label.c_str(), "GainCalibrationStep4", "Yes");
		val = core.getProperty(label.c_str(), "GainCalibrationStep4");
#endif


		  if(true)
		  {
		  		 ////test Rad mode
		 cout << "RAD mode framerate 1 "<<endl;
		 core.setProperty(label.c_str(), "FrameRate", "0.8");
		 core.setProperty(label.c_str(), "Mode", "RAD");
		 core.snapImage();
		 core.getImage();
		 cout << "RAD mode OK! "<<endl;


		 cout << "Snap image in fluoro mode (high)  "<<endl;
		 core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 core.snapImage();
		 //core.getImage();
		 cout << "OK! "<<endl;

		 //test Fluoro mode
		 cout << "Live mode (high) "<<endl;
		 core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 val = core.getProperty(label.c_str(), "Mode");
		 core.startContinuousSequenceAcquisition(10);
		 cout << "Press any key to stop "<<endl;
		 while(!_kbhit()) Sleep (100);
		 core.stopSequenceAcquisition();

		 cout << "Snap image in fluoro mode (low) "<<endl;
		 core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 core.snapImage();
		 //core.getImage();
		 cout << "OK! "<<endl;

		 //test Fluoro mode
		 cout << "Live mode (low) "<<endl;
		 core.setProperty(label.c_str(), "Mode", "Low-Res Fluoro");
		 val = core.getProperty(label.c_str(), "Mode");
		 core.startContinuousSequenceAcquisition(10);
		 cout << "Press any key to stop "<<endl;
		 while(!_kbhit()) Sleep (100);
		 core.stopSequenceAcquisition();

		 ////test Fluoro mode
		 //cout << "Live mode "<<endl;
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //core.setProperty(label.c_str(), "Mode", "RAD");
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //val = core.getProperty(label.c_str(), "Mode");
		 //cout << "snap iamge "<<endl;
		 //core.snapImage();
		 //core.getImage();

		 //cout << "Live... "<<endl;
		 //core.startContinuousSequenceAcquisition(10);
		 //cout << "Press any key to stop "<<endl;
		 //while(!_kbhit()) Sleep (100);
		 //core.stopSequenceAcquisition();
		 //test Fluoro mode
//--------------------------------------------------------------------------------------------
		 //-----------------Calibration------------------------------------------------------------------------------------
		 cout << "High-Res Fluoro GainCalibration "<<endl;
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //core.setProperty(label.c_str(), "Mode", "RAD");

		 core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");

		 core.setProperty(label.c_str(), "CalibrationMode", "GainCalibration");

		 val = core.getProperty(label.c_str(), "Mode");
		 core.startContinuousSequenceAcquisition(10);
		 cout << "Press any key to stop "<<endl;
		 while(!_kbhit()) Sleep (100);
		 core.stopSequenceAcquisition();


		 cout << "Low-Res Fluoro GainCalibration "<<endl;
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //core.setProperty(label.c_str(), "Mode", "RAD");

		 core.setProperty(label.c_str(), "Mode", "Low-Res Fluoro");

		 core.setProperty(label.c_str(), "CalibrationMode", "GainCalibration");

		 val = core.getProperty(label.c_str(), "Mode");
		 core.startContinuousSequenceAcquisition(10);
		 cout << "Press any key to stop "<<endl;
		 while(!_kbhit()) Sleep (100);
		 core.stopSequenceAcquisition();

		 cout << "RAD GainCalibration "<<endl;
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //core.setProperty(label.c_str(), "Mode", "RAD");

		 core.setProperty(label.c_str(), "Mode", "RAD");

		 core.setProperty(label.c_str(), "CalibrationMode", "GainCalibration");

		 val = core.getProperty(label.c_str(), "Mode");
		 core.startContinuousSequenceAcquisition(10);
		 cout << "Press any key to stop "<<endl;
		 while(!_kbhit()) Sleep (100);
		 core.stopSequenceAcquisition();

		 		 cout << "High-Res Fluoro OffsetCalibration "<<endl;
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //core.setProperty(label.c_str(), "Mode", "RAD");

		 core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");

		 core.setProperty(label.c_str(), "CalibrationMode", "OffsetCalibration");

		 val = core.getProperty(label.c_str(), "Mode");
		 core.startContinuousSequenceAcquisition(10);
		 cout << "Press any key to stop "<<endl;
		 while(!_kbhit()) Sleep (100);
		 core.stopSequenceAcquisition();


		 cout << "Low-Res Fluoro OffsetCalibration "<<endl;
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //core.setProperty(label.c_str(), "Mode", "RAD");

		 core.setProperty(label.c_str(), "Mode", "Low-Res Fluoro");

		 core.setProperty(label.c_str(), "CalibrationMode", "OffsetCalibration");

		 val = core.getProperty(label.c_str(), "Mode");
		 core.startContinuousSequenceAcquisition(10);
		 cout << "Press any key to stop "<<endl;
		 while(!_kbhit()) Sleep (100);
		 core.stopSequenceAcquisition();

		 cout << "RAD OffsetCalibration "<<endl;
		 //core.setProperty(label.c_str(), "Mode", "High-Res Fluoro");
		 //core.setProperty(label.c_str(), "Mode", "RAD");

		 core.setProperty(label.c_str(), "Mode", "RAD");

		 core.setProperty(label.c_str(), "CalibrationMode", "OffsetCalibration");

		 val = core.getProperty(label.c_str(), "Mode");
		 core.startContinuousSequenceAcquisition(10);
		 cout << "Press any key to stop "<<endl;
		 while(!_kbhit()) Sleep (100);
		 core.stopSequenceAcquisition();
		 }


      }
      else if (type == MM::StateDevice)
      {
         cout << "Testing State Device specific functions:" << endl;
      }
   
      // unload the device
      // -----------------
      core.unloadAllDevices();
   }
   catch (CMMError& err)
   {
      cout << err.getMsg();
      return 1;
   }

   // declare success
   // ---------------
   cout << "Device " + deviceName + " PASSED" << endl;
	return 0;
}
