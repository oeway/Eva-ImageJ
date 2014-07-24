#ifndef _IOSTATUS_H
#define _IOSTATUS_H

#define HS_STANDBY                 (0)
#define HS_ACTIVE                  (1)

// The following two codes are reserved, but not used
#define	HS_ACTIVE_NO_EXPOSE_CTRL   (2)
#define HS_ACTIVE_SW               (3)

#define HS_CANCEL                  (4)
#define HS_OFFSET_CAL              (5)

// Constants for handswitch control functions

#define IO_STANDBY                 (0)  // Normal idle state
#define IO_PREP                    (1)  // PREP pressed, waiting for READY
#define IO_READY                   (2)  // READY pressed, waiting for panel
#define IO_ACQ                     (3)  // X-ray acquisition in progress
#define IO_FETCH                   (4)  // Image is being retrieved
#define IO_DONE                    (5)  // X-ray acquisition cycle complete
#define IO_ABORT                   (6)  // PREP removed before start
#define IO_INIT                    (7)  // occurs briefly at startup
#define IO_INIT_ERROR              (8)  // occurs only on startup problem


#define EXP_STANDBY                (0) // panel performing standby cycles
#define EXP_AWAITING_PERMISSION    (1) // waiting for host OK
#define EXP_PERMITTED              (2) // host has given the go-ahead
#define EXP_REQUESTED              (3) // "expose_request" output asserted
#define EXP_CONFIRMED              (4) // "actual length of exposure" detected
#define EXP_TIMED_OUT              (5) // A.L.E. exceeded time limit
#define EXP_COMPLETED              (6) // A.L.E. (and expose_request) removed

#define NBR_OF_RESERVED   (6)
typedef struct
{
	int    structSize;
	int    structVersion;
	int    ioState;
	int    expState;
	int    windowIsOpen;
	int    windowWaitMsec;
	int    windowRemainingMsec;
	int    acqRemainingMsec;
	int    aCrntLine;
	int    bCrntLine;
	int    reserved[NBR_OF_RESERVED];
} VIP_IO_STATUS_EX;

#endif
