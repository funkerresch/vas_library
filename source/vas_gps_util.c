//
//  vas_gps_util.c
//  tr.binaural~
//
//  Created by Admin on 22.08.18.
//

#include "vas_gps_util.h"
#include "math.h"

#define RWA_EARTHRADIUS 6378137

double degrees2radians(double degrees)
{
    return degrees * (M_PI/180);
}

double radians2degrees(double radians)
{
    return radians * (180/M_PI);
}

double vas_coordinates_getLatitude(vas_coordinates *x)
{
    return x->y;
}

double vas_coordinates_getLongitude(vas_coordinates *x)
{
    return x->x;
}

double vas_coordinates_getX(vas_coordinates *x)
{
    return x->x;
}

double vas_coordinates_getY(vas_coordinates *x)
{
    return x->y;
}

vas_coordinates vas_gps_util_calculatePointOnCircle(vas_coordinates center, double distance)
{
    double d = distance / 1000;
    vas_coordinates endCoordinate;
    endCoordinate.x =  (d)/71.5 + center.x;
    endCoordinate.y = center.y;
    return endCoordinate;
}

vas_coordinates vas_gps_util_calculateDestination(vas_coordinates center, double radius, double bearingInDegrees)
{
    double long1, long2;
    double lat1, lat2;
    double delta;
    double bearing;
    vas_coordinates destination;
    
    bearing = degrees2radians(bearingInDegrees);
    lat1 = degrees2radians(center.y);
    long1 = degrees2radians(center.x);
    delta = radius /RWA_EARTHRADIUS;
    
    lat2 = radians2degrees(asin(sin(lat1) * cos(delta) + cos(lat1) * sin(delta) * cos(bearing)));
    long2 = fmod( (long1 - asin(sin(bearing)*sin(delta) / cos(lat1)) + M_PI), (2*M_PI)) - M_PI;
    long2 = radians2degrees(long2);
    
    destination.x = long2;
    destination.y = lat2;
    return destination;
}

double vas_gps_util_calculateDistance(vas_coordinates p1, vas_coordinates p2)
{
    double R = 6373; // Earth Radius
    double lat1 = degrees2radians(p1.y);
    double lat2 = degrees2radians(p2.y);
    double dlon = degrees2radians( p2.x - p1.x);
    double dlat = degrees2radians( p2.y - p1.y);
    double a = pow((sin(dlat/2)),2) + cos(lat1) * cos(lat2) * pow((sin(dlon/2)),2) ;
    double c = 2 * atan2( sqrt(a), sqrt(1-a) ) ;
    double d = R * c;
    return d;
}

double vas_gps_util_calculateBearing(vas_coordinates p1, vas_coordinates p2)
{
    double radians;
    double degrees;
    double phi1 = degrees2radians(p1.y);
    double phi2 = degrees2radians(p2.y);
    double lam1 = degrees2radians(p1.x);
    double lam2 = degrees2radians(p2.x);
    radians = atan2(sin(lam2-lam1)*cos(phi2),cos(phi1)*sin(phi2) - sin(phi1)*cos(phi2)*cos(lam2-lam1));
    degrees = radians2degrees(radians);
    //return degrees;
    return (((int)degrees+180) % 360); // offset for working correctly with the earplug~ in pd
}

double vas_gps_util_calculateBearingWithHeadOrientation(vas_coordinates p1, vas_coordinates p2, int headDirection)
{
    double radians;
    double degrees;
    double phi1 = degrees2radians(p1.y);
    double phi2 = degrees2radians(p2.y);
    double lam1 = degrees2radians(p1.x);
    double lam2 = degrees2radians(p2.x);
    
    radians = atan2(sin(lam2-lam1)*cos(phi2),cos(phi1)*sin(phi2) - sin(phi1)*cos(phi2)*cos(lam2-lam1));
    degrees = radians2degrees(radians);
    degrees -= headDirection;
    degrees += 360;
    return (((int)degrees+180) % 360); // offset for working correctly with the earplug~ in pd
}

