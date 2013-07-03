
#define boolean bool

#ifndef TARGET_H
  #define TARGET_H

#define NOT_SET 9999

// Toward North or South Poles the Sun may not rise or set every day
typedef enum
{ DAYTYPE_NORMAL      = 0
, DAYTYPE_POLAR_DAY   = 1 // AKA midnight sun
, DAYTYPE_POLAR_NIGHT = 2 
} DayType;

// What is this program supposed to do
typedef enum
{ FUNCTION_WAIT                // Wait for sun to pass specified twilight angle
, FUNCTION_POLL                // Indicate immediately if sun is up or down, relative to specified twilight angle
, FUNCTION_LIST                // List the specified number of days times for sunrise and sunset of specified twiligh
, FUNCTION_USAGE               // List the command line usage instructions
, FUNCTION_VERSION             // List this programs version
, FUNCTION_NOT_SET = NOT_SET 
} Function;

// Looking out for sun-up or sun-down or either
typedef enum 
{ UPDOWN_SUNRISE
, UPDOWN_SUNSET
, UPDOWN_NOT_SET = NOT_SET
} UpDown;

typedef enum
{ ONOFF_ON
, ONOFF_OFF
} OnOff;

typedef struct
{ 
  double latitude;            // Degrees N
  double longitude;           // Degrees E
  double nowTime;             // Unit: hours
  unsigned int nowYear;       // Normal Calendar Year - eg 2013
  unsigned int nowMonth;      // Normal Month, January = 1 to December = 12
  unsigned int nowDayOfMonth; // 1 to 31
  double hourOffset;          // Unit: Hours
  double twilightAngle;    // Degrees, -ve = below horizon
  double riseTime;         // Sunrise    - time of, Unit: hours, GMT
  double noonTime;         // Solar noon - time of, Unit: hours, GMT
  double setTime;          // Sunset     - time of, Unit: hours, GMT
  unsigned int year;       // Normal Calendar Year - eg 2013
  unsigned int month;      // Normal Month, January = 1 to December = 12
  unsigned int dayOfMonth; // 1 to 31
  unsigned int daysSince2000;
  DayType  dayType;
  Function function;       // What is this program meant to do?
  OnOff    report;         // Is a report required
  OnOff    debug;          // Is debug output required
  OnOff    exitReport;     // Return text exit: "DAY", "NIGHT", "ERROR", "OK"
  UpDown   upDown;         // Look for sun rising, setting or either
  unsigned int list;       // How many days should sunrise/set be listed for
} targetStruct;

double getOffsetRiseTime (targetStruct *pTarget);
double getOffsetSetTime  (targetStruct *pTarget);

#define EXIT_OK    0
#define EXIT_ERROR 1
#define EXIT_DAY   2
#define EXIT_NIGHT 3

int poll (targetStruct *pTarget);
int wait (targetStruct *pTarget);

#endif



