#ifndef _HH__PAXSCAN__HH__
#define _HH__PAXSCAN__HH__

extern int queryProgress(bool showAll = false);
extern int ioQueryStatus(bool showAll = false);
extern int checkAcqStatus();
extern int  performOffsetCalibration();
extern int  performHwGainCalibration();
extern int  performSwGainCalibration();
extern void getModeInfo();
extern void getSysInfo();
extern void showImageStatistics(int npixels, USHORT *image_ptr);
extern void writeImageToFile(void);
extern int  CheckRecLink();
extern int  performHwRadAcquisition();
extern int  performSwRadAcquisition();
extern void showMenu(char* currShake);
extern int run(int argc, char* argv[]);


#endif