/**
 * @file vas_gps_util
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technical University Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * Tools for performing gps-based calculation <br>
 * <br>
 * @brief Utilities for performing gps-based calculations.<br>
 * <br>
 * <br>
 */

#ifndef vas_gps_util_h
#define vas_gps_util_h

#include <stdio.h>

typedef struct vas_coordinates
{
    double x;
    double y;
} vas_coordinates;

double vas_coordinates_getLatitude(vas_coordinates *x);

double vas_coordinates_getLongitude(vas_coordinates *x);

double vas_coordinates_getX(vas_coordinates *x);

double vas_coordinates_getY(vas_coordinates *x);

vas_coordinates vas_gps_util_calculatePointOnCircle(vas_coordinates center, double distance);

vas_coordinates vas_gps_util_calculateDestination(vas_coordinates center, double radius, double bearingInDegrees);

double vas_gps_util_calculateDistance(vas_coordinates p1, vas_coordinates p2);

double vas_gps_util_calculateBearing(vas_coordinates p1, vas_coordinates p2);

double vas_gps_util_calculateBearingWithHeadOrientation(vas_coordinates p1, vas_coordinates p2, int headDirection);

#endif /* vas_gps_util_h */
