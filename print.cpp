#include <stdio.h>
#include <iostream>
#include <cmath> 
#include "sunwait.h"
#include "sunriset.h"
#include "print.h"

static const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

double myDayLength (targetStruct *pTarget)
{ switch (pTarget->dayType)
  {
  case DAYTYPE_NORMAL:      return pTarget->setTime - pTarget->riseTime; break;
  case DAYTYPE_POLAR_DAY:   return 24.0; break;
  case DAYTYPE_POLAR_NIGHT: return 0.0; break;
  }
  return 0.0;
}

void print_situation 
( DayType pDayType
, const char* pTitle
, double  pRiseTime
, double  pSetTime
) 
{ if (pDayType == DAYTYPE_NORMAL)
  { printf 
    ( "%s %2.2d:%2.2d GMT, %s %2.2d:%2.2d GMT\n"
    , pTitle
    , hours (pRiseTime), minutes (pRiseTime)
    , "sets:"
    , hours (pSetTime),  minutes (pSetTime)
    );
  }
  else if (pDayType == DAYTYPE_POLAR_DAY)
  { printf ("%s --:-- GMT, sets: --:-- GMT (Never darker)\n", pTitle);
  }
  else if (pDayType == DAYTYPE_POLAR_NIGHT)
  { printf ("%s --:-- GMT, sets: --:-- GMT (Never lighter)\n", pTitle);
  }
} 

void print_twilight
( const char* pTitle
, double  pDayLength
, double  pTwilightLength
) 
{ printf 
  ( "%s %2.2d:%2.2d hours, twilight: %2.2d:%2.2d hours\n"
  , pTitle
  , hours (pDayLength),      minutes (pDayLength)
  , hours (pTwilightLength), minutes (pTwilightLength)
  );
} 

void generate_report (targetStruct *pTarget)
{
  /*
  ** Generate and save sunrise and sunset times for target 
  */

  sunriset (pTarget);
  double twilightAngleTarget   = pTarget->twilightAngle;
  double riseTimeTarget        = pTarget->riseTime;
  double setTimeTarget         = pTarget->setTime;
//double daylengthTarget       = myDayLength (pTarget);
  DayType dayTypeTarget        = pTarget->dayType;
  double offsetRiseTimeTarget  = getOffsetRiseTime (pTarget);
  double offsetSetTimeTarget   = getOffsetSetTime  (pTarget);

  /*
  ** Generate times for different types of twilight 
  */

  pTarget-> twilightAngle = TWILIGHT_ANGLE_DAYLIGHT;
  sunriset (pTarget);
  double riseTimeDaylight      = pTarget->riseTime;
  double setTimeDaylight       = pTarget->setTime;
  double daylengthDaylight     = myDayLength (pTarget);
  DayType dayTypeDaylight      = pTarget->dayType;

  pTarget-> twilightAngle = TWILIGHT_ANGLE_CIVIL;
  sunriset (pTarget);
  double riseTimeCivil         = pTarget->riseTime;
  double setTimeCivil          = pTarget->setTime;
  double daylengthCivil        = myDayLength (pTarget);
  DayType dayTypeCivil         = pTarget->dayType;

  pTarget-> twilightAngle = TWILIGHT_ANGLE_NAUTICAL;
  sunriset (pTarget);
  double riseTimeNautical      = pTarget->riseTime;
  double setTimeNautical       = pTarget->setTime;
  double daylengthNautical     = myDayLength (pTarget);
  DayType dayTypeNautical      = pTarget->dayType;

  pTarget-> twilightAngle = TWILIGHT_ANGLE_ASTRONOMICAL;
  sunriset (pTarget);
  double riseTimeAstronomical  = pTarget->riseTime;
  double setTimeAstronomical   = pTarget->setTime;
  double daylengthAstronomical = myDayLength (pTarget);
  DayType dayTypeAstonomical   = pTarget->dayType;

  /* Restore target settings */
  pTarget->riseTime      = riseTimeTarget;
  pTarget->setTime       = setTimeTarget;
  pTarget->twilightAngle = twilightAngleTarget;
  pTarget->dayType       = dayTypeTarget;


  /*
  ** Now generate the report 
  */

  printf ("\n");
  
  printf 
  ("        Current Date and Time: %2.2d-%s-%4.4d, %2.2d:%2.2d:%2.2d GMT\n"
  , pTarget->nowDayOfMonth
  , months[pTarget->nowMonth-1]
  , pTarget->nowYear
	, hours (pTarget->nowTime)
  , minutes (pTarget->nowTime)
	, seconds (pTarget->nowTime)
	);

  printf ("                     Function: ");
       if (pTarget->function == FUNCTION_WAIT)    printf ("Wait\n");
  else if (pTarget->function == FUNCTION_POLL)    printf ("Poll\n");
  else if (pTarget->function == FUNCTION_LIST)    printf ("List\n");
  else if (pTarget->function == FUNCTION_USAGE)   printf ("Usage\n");
  else if (pTarget->function == FUNCTION_VERSION) printf ("Version\n");

  printf ("\n\nTarget Information ...\n\n");

  printf 
  ("                     Location: %10.6fN, %10.6fE\n"
  , pTarget->latitude
  , pTarget->longitude
  );
  
  printf 
  ("                         Date: %2.2d-%s-%4.4d\n"
	, pTarget->dayOfMonth
	, months[pTarget->month-1]
  , pTarget->year
	);

  printf 
  ("        Sun transits meridian: %2.2d:%2.2d GMT\n",   hours(pTarget->noonTime), minutes(pTarget->noonTime)
  );

  if (pTarget->hourOffset != 0.0)
  { printf
    ( "                       Offset: %2.2d:%2.2d:%2.2d hours\n"
    , hours   (pTarget->hourOffset)
    , minutes (pTarget->hourOffset)
    , seconds (pTarget->hourOffset)
    );
  }

       if (pTarget->twilightAngle == TWILIGHT_ANGLE_DAYLIGHT)      printf("               Twilight angle: %5.2f degrees (daylight)\n",     twilightAngleTarget);
  else if (pTarget->twilightAngle == TWILIGHT_ANGLE_CIVIL)         printf("               Twilight angle: %5.2f degrees (civil)\n",        twilightAngleTarget);
  else if (pTarget->twilightAngle == TWILIGHT_ANGLE_NAUTICAL)      printf("               Twilight angle: %5.2f degrees (nautical)\n",     twilightAngleTarget);
  else if (pTarget->twilightAngle == TWILIGHT_ANGLE_ASTRONOMICAL)  printf("               Twilight angle: %5.2f degrees (astronomical)\n", twilightAngleTarget);
  else                                                             printf("               Twilight angle: %5.2f degrees (custom angle)\n", twilightAngleTarget);

  print_situation 
  ( dayTypeTarget
  , "               Twilight rises:"
  , riseTimeTarget
  , setTimeTarget
  );
  
  if (pTarget->hourOffset != 0.0)
  { print_situation 
    ( dayTypeTarget
    , "     Rises (including offset):"
    , offsetRiseTimeTarget
    , offsetSetTimeTarget
    );
  }

  printf ("\nGeneral Information ...\n\n");

  print_situation 
  ( dayTypeDaylight
  , "                    Sun rises:"
  , riseTimeDaylight
  , setTimeDaylight
  );

  print_situation 
  ( dayTypeCivil
  , "         Civil twilight rises:"
  , riseTimeCivil
  , setTimeCivil
  );

  print_situation 
  ( dayTypeNautical
  , "      Nautical twilight rises:"
  , riseTimeNautical
  , setTimeNautical
  );

  print_situation 
  ( dayTypeAstonomical
  , "  Astronomical twilight rises:"
  , riseTimeAstronomical
  , setTimeAstronomical
  );

  printf ("\n");
  printf         ("                   Day length: %2.2d:%2.2d hours\n", hours(daylengthDaylight), minutes(daylengthDaylight));
  print_twilight ("          with civil twilight:", daylengthCivil,        (daylengthCivil-daylengthDaylight)/2.0);      
  print_twilight ("       with nautical twilight:", daylengthNautical,     (daylengthNautical-daylengthCivil)/2.0);
  print_twilight ("   with astronomical twilight:", daylengthAstronomical, (daylengthAstronomical-daylengthNautical)/2.0);
  printf ("\n");
}

void print_list (targetStruct *pTarget)
{
  for (unsigned int day=0; day < pTarget->list; day++)
  {
    sunriset (pTarget);
    print_situation
      ( pTarget->dayType
      , "rises:"
      , getOffsetRiseTime (pTarget)
      , getOffsetSetTime  (pTarget)
      );
    pTarget->daysSince2000++;
  }
}
