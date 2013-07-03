/*
** sunriset.c - computes Sun rise/set times, including twilights
** Written as DAYLEN.C, 1989-08-16
** Modified to SUNRISET.C, 1992-12-01
** (c) Paul Schlyter, 1989, 1992
** Released to the public domain by Paul Schlyter, December 1992
*/

#include <stdio.h>
#include <stdlib.h> // Linux
#include <iostream>
#include <math.h>
#include "sunwait.h"
#include "sunriset.h"

using namespace std;

/************************************************************************/
/* Note: Eastern longitude positive, Western longitude negative         */
/*       Northern latitude positive, Southern latitude negative         */
/*                                                                      */
/* >>>   Longitude value IS critical in this function!              <<< */
/*                                                                      */
/*       days  = Days since 2000 plus fraction to local noon            */
/*       altit = the altitude which the Sun should cross                */
/*               Set to -35/60 degrees for rise/set, -6 degrees         */
/*               for civil, -12 degrees for nautical and -18            */
/*               degrees for astronomical twilight.                     */
/*       upper_limb: non-zero -> upper limb, zero -> center             */
/*               Set to non-zero (e.g. 1) when computing rise/set       */
/*               times, and to zero when computing start/end of         */
/*               twilight.                                              */
/*                                                                      */
/*               Both times are relative to the specified altitude,     */
/*               and thus this function can be used to comupte          */
/*               various twilight times, as well as rise/set times      */
/* Return Codes:                                                        */
/*       0  = sun rises/sets this day. Success.                         */
/*                    Times held at *trise and *tset.                   */
/*       +1 = Midnight Sun. Fail.                                       */
/*                    Sun above the specified "horizon" all 24 hours.   */
/*                    *trise set to time when the sun is at south,      */
/*                    minus 12 hours while *tset is set to the south    */
/*                    time plus 12 hours. "Day" length = 24 hours       */
/*       -1 = Polar Night. Fail.                                        */
/*                    Sun is below the specified "horizon" all 24hours. */
/*                    "Day" length = 0 hours, *trise and *tset are      */
/*                    both set to the time when the sun is at south.    */
/*                                                                      */
/************************************************************************/
void sunriset (targetStruct *pTarget)
{
  double sr;         /* solar distance, astronomical units */
  double sra;        /* sun's right ascension */
  double sdec;       /* sun's declination */
  double sradius;    /* sun's apparent radius */
  double t;          /* diurnal arc */
  double tsouth;     /* time when sun is at south */
  double sidtime;    /* local sidereal time */
  double altit;      /* sun's altitude: angle to the sun relative to the mathematical (flat-earth) horizon */

  /* compute local sideral time of this moment. */
  sidtime = revolution (GMST0(pTarget->daysSince2000) + 180.0 + pTarget->longitude);

  /* compute sun's ra + decl at this moment */
  sun_RA_dec (pTarget->daysSince2000, &sra, &sdec, &sr );

  /* compute time when sun is at south - in hours GMT. "12.00" == noon. "15" == 180degrees/12hours */
  tsouth = 12.0 - rev180(sidtime - sra)/15.0;

  /* compute the sun's apparent radius, degrees */
  sradius = 0.2666 / sr;

  /* do correction for upper limb, if necessary (only for my definition of sunset) */
  if (pTarget->twilightAngle == TWILIGHT_ANGLE_DAYLIGHT)
    altit = pTarget->twilightAngle - sradius;
  else
    altit = pTarget->twilightAngle;

  /* compute the diurnal arc that the sun traverses to reach the specified altitide altit: */
  double cost = (sind(altit) - sind(pTarget->latitude) * sind(sdec)) / (cosd(pTarget->latitude) * cosd(sdec));

  if (abs(cost) < 1.0)
  { pTarget->dayType = DAYTYPE_NORMAL; 
    t = acosd(cost)/15.0;    /* the diurnal arc, hours */

    /* store rise and set times - in hours GMT */
    pTarget->riseTime = tsouth - t;
    pTarget->noonTime = tsouth;
    pTarget->setTime  = tsouth + t;
  }
  else
  { pTarget->dayType = (cost>=1.0) ? DAYTYPE_POLAR_NIGHT : DAYTYPE_POLAR_DAY ;

    /* store rise and set times - in hours GMT */
    pTarget->riseTime = NOT_SET;
    pTarget->noonTime = tsouth;
    pTarget->setTime  = NOT_SET;
  }
}

void sunpos (double d, double *lon, double *r)
/******************************************************/
/* Computes the Sun's ecliptic longitude and distance */
/* at an instant given in d, number of days since     */
/* 2000 Jan 0.0.  The Sun's ecliptic latitude is not  */
/* computed, since it's always very near 0.           */
/******************************************************/
{
      double M,         /* Mean anomaly of the Sun */
             w,         /* Mean longitude of perihelion */
                        /* Note: Sun's mean longitude = M + w */
             e,         /* Eccentricity of Earth's orbit */
             E,         /* Eccentric anomaly */
             x, y,      /* x, y coordinates in orbit */
             v;         /* True anomaly */

      /* Compute mean elements */
      M = revolution (356.0470 + 0.9856002585 * d);
      w = 282.9404 + 4.70935E-5 * d;
      e = 0.016709 - 1.151E-9 * d;

      /* Compute true longitude and radius vector */
      E = M + e * RADIAN_TO_DEGREE * sind(M) * (1.0 + e * cosd(M));
      x = cosd (E) - e;
      y = sqrt (1.0 - e*e) * sind(E);
      *r = sqrt (x*x + y*y);              /* Solar distance */
      v = atan2d (y, x);                  /* True anomaly */
      *lon = v + w;                       /* True solar longitude */
      if (*lon >= 360.0)
        *lon -= 360.0;                    /* Make it 0..360 degrees */
}

void sun_RA_dec (double d, double *RA, double *dec, double *r)
{
  double lon, obl_ecl;
  double xs, ys, zs;
  double xe, ye, ze;
  
  /* Compute Sun's ecliptical coordinates */
  sunpos (d, &lon, r);
  
  /* Compute ecliptic rectangular coordinates */
  xs = *r * cosd(lon);
  ys = *r * sind(lon);
  zs = 0; /* because the Sun is always in the ecliptic plane! */

  /* Compute obliquity of ecliptic (inclination of Earth's axis) */
  obl_ecl = 23.4393 - 3.563E-7 * d;
  
  /* Convert to equatorial rectangular coordinates - x is unchanged */
  xe = xs;
  ye = ys * cosd(obl_ecl);
  ze = ys * sind(obl_ecl);
  
  /* Convert to spherical coordinates */
  *RA = atan2d(ye, xe);
  *dec = atan2d(ze, sqrt(xe*xe + ye*ye));
      
}

double revolution (double x)
/*****************************************/
/* Reduce angle to within 0..360 degrees */
/*****************************************/
{
  return x - (360.0 * floor(x/360.0));
}

double rev180 (double x)
/*********************************************/
/* Reduce angle to -179.999 to +180 degrees  */
/*********************************************/
{
  double y = revolution (x);
  return y <= 180 ? y : y - 360.0;
}

/*******************************************************************/
/* This function computes GMST0, the Greenwhich Mean Sidereal Time */
/* at 0h UT (i.e. the sidereal time at the Greenwhich meridian at  */
/* 0h UT).  GMST is then the sidereal time at Greenwich at any     */
/* time of the day.  I've generelized GMST0 as well, and define it */
/* as:  GMST0 = GMST - UT  --  this allows GMST0 to be computed at */
/* other times than 0h UT as well.  While this sounds somewhat     */
/* contradictory, it is very practical:  instead of computing      */
/* GMST like:                                                      */
/*                                                                 */
/*  GMST = (GMST0) + UT * (366.2422/365.2422)                      */
/*                                                                 */
/* where (GMST0) is the GMST last time UT was 0 hours, one simply  */
/* computes:                                                       */
/*                                                                 */
/*  GMST = GMST0 + UT                                              */
/*                                                                 */
/* where GMST0 is the GMST "at 0h UT" but at the current moment!   */
/* Defined in this way, GMST0 will increase with about 4 min a     */
/* day.  It also happens that GMST0 (in degrees, 1 hr = 15 degr)   */
/* is equal to the Sun's mean longitude plus/minus 180 degrees!    */
/* (if we neglect aberration, which amounts to 20 seconds of arc   */
/* or 1.33 seconds of time)                                        */
/*                                                                 */
/*******************************************************************/

double GMST0 (double d)
{
  /* Sidtime at 0h UT = L (Sun's mean longitude) + 180.0 degr  */
  /* L = M + w, as defined in sunpos().  Since I'm too lazy to */
  /* add these numbers, I'll let the C compiler do it for me.  */
  /* Any decent C compiler will add the constants at compile   */
  /* time, imposing no runtime or code overhead.               */
  return revolution ((180.0 + 356.0470 + 282.9404) + (0.9856002585 + 4.70935E-5) * d);
}

int myTrunc  (double d) { return (d>0) ? (int) floor(d) : (int) ceil(d) ; }
double myAbs (double d) { return (d>0) ? d : -d ; }

int hours    (double d) { return myTrunc(d); }
int minutes  (double d) { return myTrunc(fmod(myAbs(d)*60,60)); }
int seconds  (double d) { return myTrunc(fmod(myAbs(d)*3600,60)); }

unsigned int daysSince2000 (unsigned int pYear, unsigned int pMonth, unsigned int pDay)
{ 
  unsigned int yearsSince2000 = pYear - 2000;

  unsigned int leapDaysSince2000 
    = (unsigned int) floor (yearsSince2000/4)        /* Every evenly divisible 4 years is a leap-year */
    - (unsigned int) floor (yearsSince2000/100)      /* Except centuries, unless evenly diviable by 400 */
    + (unsigned int) floor (yearsSince2000/400) +1;  /* 2000 itself was a leap year with the 400 rule */;

  unsigned int monthDays = 0;
  switch (pMonth)
  { case  1: monthDays = 0;  break;
    case  2: monthDays = 31; break;
    case  3: monthDays = 31+28; break;
    case  4: monthDays = 31+28+31; break;
    case  5: monthDays = 31+28+31 +30; break;
    case  6: monthDays = 31+28+31 +30+31; break;
    case  7: monthDays = 31+28+31 +30+31+30; break;
    case  8: monthDays = 31+28+31 +30+31+30 +31; break;
    case  9: monthDays = 31+28+31 +30+31+30 +31+31; break;
    case 10: monthDays = 31+28+31 +30+31+30 +31+31+30; break;
    case 11: monthDays = 31+28+31 +30+31+30 +31+31+30 +31; break;
    case 12: monthDays = 31+28+31 +30+31+30 +31+31+30 +31+30; break;
    default: printf ("Error: Number of month is out of range\n");
  }

  return (yearsSince2000 * 365) + leapDaysSince2000 + monthDays + pDay -1; /* -1 : Don't include today in count */
}


