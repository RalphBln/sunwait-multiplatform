// sunwait.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <cstring>
#include <math.h>
#include "sunwait.h"
#include "sunriset.h"
#include "print.h"

using namespace std;

/* copyright (c) 2000,2004 Daniel Risacher */
/* minor changes courtesy of Dr. David M. MacMillan */
/* major changes courtesy of Ian Craig (2012-13) */
/* Licensed under the Gnu General Public License */

// Sunset is the instant at which the upper edge of the Sun disappears below the horizon (in the west).
// Civil twilight is the period from sunset until the geometric center of the sun is 6° below the horizon.
// Nautical twilight is the period when the geometric center of the sun is between 6° and 12° below the horizon.
// Astronomical twilight is the period when the geometric center of the sun is between 12° and 18° below the horizon.
// Night is period when the geometric center of the sun falls 18° below the horizon.


/*
** Define global:
** 'targetStruct' structure allows pretty much everything to be carted simply around functions.
** Functions can use a single parameter rather than have long parameter lists or less honest side-effects.
** It makes pointers or values in argument issues disappear.
** It forces functions to name the parameter they're accessing, rather than parameter position in argument.
** Data types are easier to handle. 
** It can be a bit naughty on side-effects.
*/
targetStruct gTarget;

void print_version ()
{
  printf ("Sunwait for Windows. Version 0.4 (IFC). Release 07 June 2013.\n");
  printf ("Code Contributors: P.Schlyter, D.Risacher, D.MacMillan and I.Craig.\n");
}

/* It's just too useful to have this here */
void print_usage () 
{ printf ("Usage: sunwait [major options] [minor options] [twilight types] [rise|set] [+/-offset] [latitude longitude]\n");
  printf ("\n");
  printf ("Example1: sunwait wait daylight rise -1:15:10 51.477932N 0.000000E\n");
  printf ("Wait until 1 hour 15 minutes 10 secs before the sun rises in Greenwich, London.\n");
  printf ("\n");
  printf ("Example2: sunwait list 7 report civil 55.752163N 37.617524E\n");
  printf ("Report twilight times and list civil sunrise/set times for next 7 days. Moscow.\n");
  printf ("\n");
  printf ("Example3: sunwait poll exit angle 10 54.897786N -1.517536E\n");
  printf ("Indicate by return if the sun is 10 deg higher than horizon. Washington USA.\n");
  printf ("\n");
  printf ("latitude/longitude coordinates: floating-point degrees, with [NESW] appended.\n");
  printf ("Default: Bingham, England.\n");
  printf ("\n");
  printf ("Twilight types, either:\n");
  printf ("    daylight      Top of sun just below the horizon. Default.\n");
  printf ("    civil         Civil Twilight.         -6 degrees (below horizon).\n");
  printf ("    nautical      Nautical twilight.     -12 degrees (below horizon).\n");
  printf ("    astronomical  Astronomical twilight. -18 degrees (below horizon).\n");
  printf ("    angle [X.XX]  User-specified twilight-angle. Default: 0.\n");
  printf ("\n");
  printf ("Major options, either:\n");
  printf ("    poll          Returns immediately. See 'return codes'. Default.\n");
  printf ("    wait          Sleep until specified event occurs. Else exit immediate.\n");
  printf ("    list [X]      Report twilight times for next 'X' days. Default X value: 7.\n");
  printf ("\n");
  printf ("Minor options, any of:\n");
  printf ("    [no]report    Print detailed report of twilight times. Default: noreport.\n");
  printf ("    [no]debug     Print extra info and returns in one minute. Default: nodebug.\n");
  printf ("    [no]version   Print the version number. Default: noversion.\n");
  printf ("    [no]help      Print this help. Default: nohelp.\n");
  printf ("    [no]exit      Print 'DAY','NIGHT','OK' or 'ERROR' on exit. Default: noexit.\n");
  printf ("\n");
  printf ("Sunrise/sunset. Only useful with major-option: 'wait'. Either:\n");
  printf ("    rise          Wait for the sun to rise past specified twilight & offset.\n");
  printf ("    set           Wait for the sun to  set past specified twilight & offset.\n");
  printf ("\n");
  printf ("Offset - Only useful with major-option: 'wait':\n");
  printf ("    Expressed as MM, or HH:MM, or HH:MM:SS, with an optional leading sign (+/-).\n");
  printf ("    Indicates an additional time after sunrise or before sunset to 'wait'\n");
  printf ("    relative to the specified twilight. Negative offsets move timings into the\n");
  printf ("    night (same as 'angle'). Unit: hours. Latitudes nearer the poles should\n");
  printf ("    specify a twilight-angle rather than an offset as Summer and winter\n");
  printf ("    twilight durations differ significantly.\n");
  printf ("\n");
  printf ("Target date. Only useful with major-options: 'report' or 'list'.\n");
  printf ("    d DD          Set the target Day-of-Month to calculate for. 1 to 31.\n");
  printf ("    m MM          Set the target Month to calculate for. 1 to 12.\n");
  printf ("    y YYYY        Set the target Year to calculate for. 2000 to 2099.\n");
  printf ("\n");
  printf ("Return Codes:\n");
  printf ("    %1d             Exit from 'wait' or 'list', everythings seems OK.\n", EXIT_OK);
  printf ("    %1d             Exit from 'poll': it is DAY or twilight.\n",          EXIT_DAY);
  printf ("    %1d             Exit from 'poll': it is NIGHT (after twilight).\n", EXIT_NIGHT);
  printf ("    %1d             Error.\n", EXIT_ERROR);
  printf ("\n");
  printf ("All times are GMT. Newer definitions of GMT are confusing (and/or a bit French).\n");
  printf ("GMT simplifies mapping of time to longitude, and reduces confusion relating to\n");
  printf ("timezones and daylight savings. Note that program converts readings of your\n");
  printf ("system's time to GMT using standard C library functions.\n");
  printf ("\n");
  printf ("Error for timings are estimated at: +/- 3 minutes.\n");
  printf ("\n");
}

void myToLower (char *arg)
{ for (unsigned int i=0; i < strlen (arg); i++)
    arg[i] = tolower (arg[i]);
}

void myToLower (int argc, char *argv[])
{ for (int i=1; i < argc; i++)
    myToLower (argv [i]); 
}

boolean myIsNumber (char* arg)
{ bool digitSet = false;
  for (int i=0; ; i++)
  { switch (arg[i])
    {
    case  '0':
    case  '1':
    case  '2':
    case  '3':
    case  '4':
    case  '5':
    case  '6':
    case  '7':
    case  '8':
    case  '9': digitSet = true; break;
    case '\0': return digitSet; break;
    default:   return false;
    }
  }
  return false; /* Shouldn't get here */
}

boolean myIsSignedNumber (char* arg)
{ bool digitSet = false;
  for (int i=0; ; i++)
  { switch (arg[i])
    {
    case  '0':
    case  '1':
    case  '2':
    case  '3':
    case  '4':
    case  '5':
    case  '6':
    case  '7':
    case  '8':
    case  '9': digitSet = true; break;
    case  '+':
    case  '-': if (i>0) return false; break; /* Sign only at start */
    case '\0': return digitSet; break;
    default:   return false;
    }
  }
  return false; /* Shouldn't get here */
}

boolean myIsSignedFloat (char* arg)
{ bool digitSet = false;
  for (int i=0; ; i++)
  { switch (arg[i])
    {
    case  '0':
    case  '1':
    case  '2':
    case  '3':
    case  '4':
    case  '5':
    case  '6':
    case  '7':
    case  '8':
    case  '9': digitSet = true; break;
    case  '.': break; /* Can be anywhere (but in front of sign), or not there */
    case  '+':
    case  '-': if (i>0) return false; break; /* Sign only at start */
    case '\0': return digitSet; break;
    default:   return false;
    }
  }
  return false; /* Shouldn't get here */
}

boolean myIsSignedFloat (char *pArg, double *pDouble)
{ double number = 0;
  int    exponent = 0;
  bool   negative = false;
  bool   exponentSet = false;
  for (int i=0; ; i++)
  { switch (pArg[i])
    {
    case '0': number = (number*10) + 0; exponentSet?exponent++:true; break;
    case '1': number = (number*10) + 1; exponentSet?exponent++:true; break;
    case '2': number = (number*10) + 2; exponentSet?exponent++:true; break;
    case '3': number = (number*10) + 3; exponentSet?exponent++:true; break;
    case '4': number = (number*10) + 4; exponentSet?exponent++:true; break;
    case '5': number = (number*10) + 5; exponentSet?exponent++:true; break;
    case '6': number = (number*10) + 6; exponentSet?exponent++:true; break;
    case '7': number = (number*10) + 7; exponentSet?exponent++:true; break;
    case '8': number = (number*10) + 8; exponentSet?exponent++:true; break;
    case '9': number = (number*10) + 9; exponentSet?exponent++:true; break;
    case '.': case ',':
      exponentSet = true;
      exponent = 0; // May be: N36.513679 (not right, but it'll do)
      break;
    case '+':
      if (i>0) return false; // Sign only at start
      negative = false;
      break;
    case '-':
      if (i>0) return false; // Sign only at start
      negative = true;
      break;
    case '\0': /* Exit */
      /* Place decimal point in number */
      if (exponentSet && exponent > 0) number = number / pow (10, (double) exponent);
      if (negative) number = -number;
      *pDouble = number;
      return true; /* All done */
      break;
    default:
      return false;
    }
  }
  return false; /* Shouldn't get to here */
}

boolean isBearing (targetStruct *pTarget, char* pArg)
{ double bearing = 0;
  int   exponent = 0;
  bool  negativeBearing = false;
  bool  exponentSet = false;
  char  compass = 'X';
  for (int i=0; ; i++)
  { switch (pArg[i])
    {
    case '0': bearing = (bearing*10) + 0; exponentSet?exponent++:true; break;
    case '1': bearing = (bearing*10) + 1; exponentSet?exponent++:true; break;
    case '2': bearing = (bearing*10) + 2; exponentSet?exponent++:true; break;
    case '3': bearing = (bearing*10) + 3; exponentSet?exponent++:true; break;
    case '4': bearing = (bearing*10) + 4; exponentSet?exponent++:true; break;
    case '5': bearing = (bearing*10) + 5; exponentSet?exponent++:true; break;
    case '6': bearing = (bearing*10) + 6; exponentSet?exponent++:true; break;
    case '7': bearing = (bearing*10) + 7; exponentSet?exponent++:true; break;
    case '8': bearing = (bearing*10) + 8; exponentSet?exponent++:true; break;
    case '9': bearing = (bearing*10) + 9; exponentSet?exponent++:true; break;
    case '.': case ',':
      exponentSet = true;
      exponent = 0; // May be: N36.513679 (not right, but it'll do)
      break;
    case '+':
      if (i>0) return false; // Sign only at start
      negativeBearing = false;
      break;
    case '-':
      if (i>0) return false; // Sign only at start
      negativeBearing = true;
      break;
    case 'n': case 'N': compass = 'N'; exponentSet = true; break; // Can support 36N513679 (not right, but it'll do)
    case 'e': case 'E': compass = 'E'; exponentSet = true; break;
    case 's': case 'S': compass = 'S'; exponentSet = true; break;
    case 'w': case 'W': compass = 'W'; exponentSet = true; break;
    case '\0': /* Exit */
      /* Fail, if the compass has not been set */
      if (compass == 'X') return false;
      /* Place decimal point in bearing */
      if (exponentSet && exponent > 0) bearing = bearing / pow (10, (double) exponent);
      /* Fix-up bearing so that it is in range zero to just under 360 */
      bearing = revolution (bearing);
      bearing = negativeBearing ? 360 - bearing : bearing;
      /* Fix-up bearing to Northings or Eastings only */
           if (compass == 'S') { bearing = 360 - bearing; compass = 'N'; }
      else if (compass == 'W') { bearing = 360 - bearing; compass = 'E'; }
      /* It's almost done, assign bearing to appropriate global */
           if (compass == 'N') pTarget->latitude  = bearing;
      else if (compass == 'E') pTarget->longitude = bearing;
      else return false;
      return true;  /* All done */
      break;
    default:
      return false;
    }
  }
  return false; /* Shouldn't get to here */
}

boolean isOffset (targetStruct *pTarget, char* pArg)
{ int    colon = 0, number0 = 0, number1 = 0, number2 = 0;
  bool   negativeOffset = false;
  double returnOffset = 0.0;

  for (int i=0; ; i++)
  { switch (pArg[i])
    {
    case '0': number0 = (number0*10) + 0; break;
    case '1': number0 = (number0*10) + 1; break;
    case '2': number0 = (number0*10) + 2; break;
    case '3': number0 = (number0*10) + 3; break;
    case '4': number0 = (number0*10) + 4; break;
    case '5': number0 = (number0*10) + 5; break;
    case '6': number0 = (number0*10) + 6; break;
    case '7': number0 = (number0*10) + 7; break;
    case '8': number0 = (number0*10) + 8; break;
    case '9': number0 = (number0*10) + 9; break;
    case ':':
      number2 = number1;
      number1 = number0;
      number0 = 0;
      colon++;
      break;
    case '+':
      break;
    case '-':
      if (i>0) return false; // Sign only at start
      negativeOffset = true;
      break;
    case '\0': /* Exit */
           if (colon==0) returnOffset = number0/60.0;
      else if (colon==1) returnOffset = number1 + number0/60.0;
      else if (colon==2) returnOffset = number2 + number1/60.0 + number0/3600.0;
      else return false;
      if (negativeOffset) { returnOffset = -returnOffset; }
      pTarget->hourOffset = returnOffset;
      return true; /* <-- Hopefully, exit here <-- */
      break;
    default:
      return false;
    }
  }
  return false; /* Shouldn't get here */
}

double getOffsetRiseTime (targetStruct *pTarget) 
{ double offset = pTarget->riseTime + pTarget->hourOffset;
  if (offset <  0.00) return 0.0000;
  if (offset >= 12.0) return 11.999;
  return offset;
}

double getOffsetSetTime (targetStruct *pTarget) 
{ double offset = pTarget->setTime - pTarget->hourOffset;
  if (offset <  12.0) return 0.0000;
  if (offset >= 24.0) return 23.999;
  return offset;
}

/*
** >>>>> main() <<<<<
*/

int main(int argc, char *argv[])
{
  gTarget.latitude       = NOT_SET;
  gTarget.longitude      = NOT_SET;
  gTarget.twilightAngle  = TWILIGHT_ANGLE_DAYLIGHT;
  gTarget.hourOffset     = 0.0;
  gTarget.riseTime       = 0.0;
  gTarget.setTime        = 0.0;
  gTarget.daysSince2000  = 0;
  gTarget.year           = NOT_SET;
  gTarget.month          = NOT_SET;
  gTarget.dayOfMonth     = NOT_SET;
  gTarget.function       = FUNCTION_NOT_SET;
  gTarget.report         = ONOFF_OFF;
  gTarget.debug          = ONOFF_OFF;
  gTarget.exitReport     = ONOFF_OFF;
  gTarget.dayType        = DAYTYPE_NORMAL;

  /* Return code */
  int exitCode = EXIT_OK;

  /*
  ** Get current time in GMT
  */

  { 
    struct tm tmNow;

    /* Windows code: Start */
    //errno_t err;
    //__int64 ltime;
    //time (&ltime);
    //err = _gmtime64_s (&tmNow, &ltime);
    //if (err) { printf ("Error: Invalid Argument to _gmtime64_s."); }
    /* Windows code: End */

    ///* Linux code: Start */
    time_t tt;
    tt = time (NULL);
    ctime (&tt);
    gmtime_r (&tt, &tmNow);
    ///* Linux code: End */

    gTarget.nowTime        = tmNow.tm_hour + tmNow.tm_min/60.0 + tmNow.tm_sec/3600;
    gTarget.nowYear        = tmNow.tm_year + 1900;
    gTarget.nowMonth       = tmNow.tm_mon  + 1;
    gTarget.nowDayOfMonth  = tmNow.tm_mday;
    gTarget.year           = gTarget.nowYear;
    gTarget.month          = gTarget.nowMonth;
    gTarget.dayOfMonth     = gTarget.nowDayOfMonth;
  }

  /*
  ** Parse command line arguments 
  */

  /* Change to all lowercase, just to make life easier ... */
  myToLower (argc, argv); 
  /* Look for debug being activated ... */
  for (int i=1; i < argc; i++) if (!strcmp (argv [i], "-debug")) gTarget.debug = ONOFF_ON;
  /* For each argument */
  for (int i=1; i < argc; i++)
  { 
    char *arg = argv[i];

    /* Echo argument, if in debug */
    if (gTarget.debug == ONOFF_ON) printf ("Debug: argv[%d]: >%s<\n", i, arg);

    /* Strip any hyphen from arguments, but not negative signs for numbers */
    if (arg[0]=='-' && arg[1] != '\0' && !isdigit(arg[1])) *arg++;
    
         if   (!strcmp (arg, "v")             ||
               !strcmp (arg, "version"))      gTarget.function = FUNCTION_VERSION;
    else if   (!strcmp (arg, "nv")            ||
               !strcmp (arg, "noversion"))    {} // Ignore

    else if   (!strcmp (arg, "?")             ||
               !strcmp (arg, "h")             ||
               !strcmp (arg, "help"))         gTarget.function = FUNCTION_USAGE;
    else if   (!strcmp (arg, "nh" )           ||
               !strcmp (arg, "nohelp"))       {} // Ignore

    else if   (!strcmp (arg, "d")             ||
               !strcmp (arg, "debug"))        gTarget.debug = ONOFF_ON;
    else if   (!strcmp (arg, "nd")            ||
               !strcmp (arg, "nodebug"))      gTarget.debug = ONOFF_OFF;

    else if   (!strcmp (arg, "r")             ||
               !strcmp (arg, "p")             ||
               !strcmp (arg, "print")         ||
               !strcmp (arg, "report"))       gTarget.report = ONOFF_ON;
    else if   (!strcmp (arg, "nr")            ||
               !strcmp (arg, "np")            ||
               !strcmp (arg, "noprint")       ||
               !strcmp (arg, "noreport"))     gTarget.report = ONOFF_OFF;

    else if   (!strcmp (arg, "e")             ||
               !strcmp (arg, "er")            ||
               !strcmp (arg, "exit")          ||
               !strcmp (arg, "exitreport"))   gTarget.exitReport = ONOFF_ON;
    else if   (!strcmp (arg, "ne")            ||
               !strcmp (arg, "ner")           ||
               !strcmp (arg, "noexit")        ||
               !strcmp (arg, "noexitreport")) gTarget.exitReport = ONOFF_OFF;

    /* If a setting follows flag, process ... NOTE: targetGMT - other "struct tm" fields are probably broken from now on */
    else if   (!strcmp (arg, "y") && i+1<argc && myIsNumber (argv[i+1])) gTarget.year       = atoi (argv [++i]); // Note: "++i"
    else if   (!strcmp (arg, "m") && i+1<argc && myIsNumber (argv[i+1])) gTarget.month      = atoi (argv [++i]); // Note: "++i"
    else if   (!strcmp (arg, "d") && i+1<argc && myIsNumber (argv[i+1])) gTarget.dayOfMonth = atoi (argv [++i]); // Note: "++i"

    else if   (!strcmp (arg, "sun")           ||
               !strcmp (arg, "day")           ||
               !strcmp (arg, "light")         ||
               !strcmp (arg, "daylight"))     gTarget.twilightAngle = TWILIGHT_ANGLE_DAYLIGHT;
    else if   (!strcmp (arg, "civil")         ||
               !strcmp (arg, "civ"))          gTarget.twilightAngle = TWILIGHT_ANGLE_CIVIL;
    else if   (!strcmp (arg, "nautical")      ||
               !strcmp (arg, "nau")           ||
               !strcmp (arg, "naut"))         gTarget.twilightAngle = TWILIGHT_ANGLE_NAUTICAL;
    else if   (!strcmp (arg, "astronomical")  ||
               !strcmp (arg, "ast")           ||
               !strcmp (arg, "astr")          ||
               !strcmp (arg, "astro"))        gTarget.twilightAngle = TWILIGHT_ANGLE_ASTRONOMICAL;
    else if   (!strcmp (arg, "a")             ||
               !strcmp (arg, "angle")         ||
               !strcmp (arg, "twilightangle") ||
               !strcmp (arg, "twilight"))     {
                                                if (i+1<argc && myIsSignedFloat (argv[i+1]))
                                                  gTarget.twilightAngle = atof (argv [++i]); // Note: "++i"
                                                else
                                                  gTarget.twilightAngle = TWILIGHT_ANGLE_DAYLIGHT;
                                              }

    else if   (!strcmp (arg, "sunrise")       ||
               !strcmp (arg, "rise")          ||
               !strcmp (arg, "dawn")          ||
               !strcmp (arg, "sunup")         ||
               !strcmp (arg, "up"))           gTarget.upDown = UPDOWN_SUNRISE;
    else if   (!strcmp (arg, "sunset")        ||
               !strcmp (arg, "set")           ||
               !strcmp (arg, "dusk")          ||
               !strcmp (arg, "sundown")       ||
               !strcmp (arg, "down"))         gTarget.upDown = UPDOWN_SUNSET;

    else if   (!strcmp (arg, "wait"))         gTarget.function = FUNCTION_WAIT;
    else if   (!strcmp (arg, "poll"))         gTarget.function = FUNCTION_POLL;
    else if   (!strcmp (arg, "list")          ||
               !strcmp (arg, "l"))            {
                                                gTarget.function = FUNCTION_LIST;
                                                if (i+1<argc && myIsSignedNumber (argv[i+1]))
                                                  gTarget.list = atoi (argv [++i]); // Note: ++i
                                                else
                                                  gTarget.list = 7;
                                              }

    else if   (isBearing (&gTarget, arg)) {} /* Functionality in "isBearing()" */
    else if   (isOffset  (&gTarget, arg)) {} /* Functionality in "isOffset()" */
    else printf ("Error: Unknown command-line argument: %s\n", arg);
  }

  /*
  ** Analyse command line, check for errors and fill in gaps
  */

  /*
  ** Check: Target Date
  */

  if (gTarget.year     < 100 && gTarget.year       >= 0) gTarget.year += 2000;
  if (gTarget.month      < 1 && gTarget.month      > 12) { printf ("Error: \"Month\" must be between 1 and 12: %u\n", gTarget.month); exit (EXIT_ERROR); }
  if (gTarget.dayOfMonth < 1 && gTarget.dayOfMonth > 31) { printf ("Error: \"Day of month\" must be between 1 and 31: %u\n", gTarget.dayOfMonth); exit (EXIT_ERROR); }
  // The sunset calculator requires the number of days since Jan 0, 2000
  gTarget.daysSince2000 = daysSince2000 (gTarget.year, gTarget.month, gTarget.dayOfMonth);

  /*
  ** Check: Latitude and Longitude
  */

  if (gTarget.latitude == NOT_SET || gTarget.longitude == NOT_SET) 
  { if (gTarget.debug == ONOFF_ON) printf ("Debug: latitude or longitude not set. Default applied.\n"); 
    gTarget.latitude  = 52.952308;
    gTarget.longitude = 359.048052; /* The Buttercross, Bingham, England */
  }

  /* Co-ordinates must be in 0 to 360 range */
  gTarget.latitude  = revolution (gTarget.latitude);
  gTarget.longitude = revolution (gTarget.longitude);

  if (gTarget.debug == ONOFF_ON)
  {  printf ("Debug: Co-ordinates - Latitude:  %f\n", gTarget.latitude);
     printf ("Debug: Co-ordinates - Longitude: %f\n", gTarget.longitude);
  }

  /*
  ** Check: Twilight Angle 
  */

  if (gTarget.twilightAngle == NOT_SET)
  { if (gTarget.debug == ONOFF_ON) printf ("Debug: Sunset/Sunrise type not set. Default: daylight.\n");
    gTarget.twilightAngle = TWILIGHT_ANGLE_DAYLIGHT;
  }

  if (gTarget.twilightAngle <= -90 || gTarget.twilightAngle >= 90)
  {
    printf("Error: Twilight angle must be between -90 and +90 (-ve = below horizon), your setting: %f\n", gTarget.twilightAngle);
    gTarget.twilightAngle = TWILIGHT_ANGLE_DAYLIGHT;
  }

  if (gTarget.debug == ONOFF_ON)
  {       if (gTarget.twilightAngle == TWILIGHT_ANGLE_DAYLIGHT)     printf ("Debug: Twilight - Daylight\n");
     else if (gTarget.twilightAngle == TWILIGHT_ANGLE_CIVIL)        printf ("Debug: Twilight - Civil\n");
     else if (gTarget.twilightAngle == TWILIGHT_ANGLE_NAUTICAL)     printf ("Debug: Twilight - Nautical\n");
     else if (gTarget.twilightAngle == TWILIGHT_ANGLE_ASTRONOMICAL) printf ("Debug: Twilight - Astronomical\n");
     else printf ("Debug: User specified twilight angle (degrees): %f\n", gTarget.twilightAngle);
  }

  /*
  ** Check: Major-option or Function
  */

  // IF no function requested THEN default to "poll"
  if (gTarget.function == FUNCTION_NOT_SET)
  {
    if (argc < 2)
      gTarget.function = FUNCTION_USAGE;
    else
    gTarget.function = FUNCTION_POLL;
  }

  if (gTarget.debug == ONOFF_ON)
  {      if (gTarget.function == FUNCTION_LIST)    printf ("Debug: Function - List\n");
    else if (gTarget.function == FUNCTION_NOT_SET) printf ("Debug: Function - Not set\n");
    else if (gTarget.function == FUNCTION_POLL)    printf ("Debug: Function - Poll\n");
    else if (gTarget.function == FUNCTION_USAGE)   printf ("Debug: Function - Usage\n");
    else if (gTarget.function == FUNCTION_VERSION) printf ("Debug: Function - Version\n");
    else if (gTarget.function == FUNCTION_WAIT)    printf ("Debug: Function - Wait\n");
  }

  /*
  ** Calculate start/end of twilight for given twilight type. 
  ** For latitudes near poles, the sun might not pass through specified twilight angle that day. 
  */

  sunriset (&gTarget);

  // Print out (on standard output) the report about sunrise and sunset times
  if (gTarget.report == ONOFF_ON) generate_report (&gTarget);

  // Anything decided on now?
  if (gTarget.function == FUNCTION_VERSION) 
  { print_version (); 
    exitCode = EXIT_OK; 
  }
  else if (gTarget.function == FUNCTION_USAGE) 
  { print_usage (); 
    exitCode = EXIT_OK; 
  }
  else if (gTarget.function == FUNCTION_LIST)  
  { print_list (&gTarget);  
    exitCode = EXIT_OK; 
  }
  else if (gTarget.function == FUNCTION_WAIT)
  { exitCode = wait (&gTarget); 
  }
  else if (gTarget.function == FUNCTION_POLL)  
  { exitCode = poll (&gTarget); 
  }

  if (gTarget.exitReport == ONOFF_ON)
  {      if (exitCode == EXIT_DAY)   printf("DAY\n");
    else if (exitCode == EXIT_NIGHT) printf("NIGHT\n");
    else if (exitCode == EXIT_OK)    printf("OK\n");
    else if (exitCode == EXIT_ERROR) printf("ERROR\n");
  }

  exit (exitCode);
}

/*
** Simply check if we think now/current-time is night OR day (including twilight)
*/
int poll (targetStruct *pTarget)
{
  if (pTarget->dayType == DAYTYPE_POLAR_DAY)    return EXIT_DAY;
  if (pTarget->dayType == DAYTYPE_POLAR_NIGHT ) return EXIT_NIGHT;

  if 
  (  pTarget->nowTime >= getOffsetRiseTime (pTarget)
  && pTarget->nowTime <  getOffsetSetTime  (pTarget)
  ) return EXIT_DAY;

  return EXIT_NIGHT;
}

int wait (targetStruct *pTarget)
{
  int days = daysSince2000 (pTarget->year,    pTarget->month,    pTarget->dayOfMonth)
           - daysSince2000 (pTarget->nowYear, pTarget->nowMonth, pTarget->nowDayOfMonth);

  if (days < 0) printf ("Debug: Event already passed previous day.\n");

  double  nowTime = pTarget->nowTime;
  double riseTime = getOffsetRiseTime (pTarget);
  double  setTime = getOffsetSetTime  (pTarget);
  double interval = (pTarget->upDown == UPDOWN_SUNRISE) ?  riseTime - nowTime : setTime - nowTime;

  // Add days
  interval += days * 24;

  // Don't wait if event has passed
  if (interval < 0) 
  { interval = 0;
    if (pTarget->debug == ONOFF_ON) printf ("Debug: Event already passed today.\n");
    return EXIT_ERROR;
  }

  // In debug mode, we don't want to wait for sunrise or sunset. Wait a minute instead.
  if (pTarget->debug == ONOFF_ON)
  {
    printf("Debug: Debug mode, \"wait\" reduced from %f hours to 1 minute.\n", interval);
    interval = (60 * 1000);
  }
  else
  {
    printf("Debug: Wait (seconds): %f\n", interval);
    // Convert hours to milliseconds
    interval *= 60 * 60 * 1000;
  }

  // This is it - wait until event occurs and then exit normally
  //Sleep (interval); // Windows
  sleep (interval);    // Linux

  return EXIT_OK;
}
