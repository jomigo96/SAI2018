/**
 * @file fms.h
 * @author João Gonçalves, Tiago Oliveira, Carolina Serra
 * @date 31 Oct 2018
 * 
 * @brief Header file for the Flight Management System library
 *
 *
 * Defines functions for computing route distances, navigation references,
 * and contains I/O utilities. 
 * Can be linked to C or C++ programs.
 *
 * @see https://github.com/jomigo96/SAI2018
 * */

#ifndef __FMSLIB_H_
#define __FMSLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ALPHA 5
// alpha has units /h

/**
 * @brief Greographical position
 *
 * */
struct waypoints{

	double height;    /**< Height, in feet. */
	double latitude;  /**< Latitude, in radians. */
	double longitude; /**< Longitude, in radians. */

};
/**
 * @brief Route distance of a set of waypoints
 *
 * Computes the route distance of a set of waypoints, navigated following 
 * great-circle arcs (the smallest distance between two points in the 
 * surface of the Earth).
 * Uses the spherical Earth, approximation, and Vicenty's method.
 *
 * @param points pointer to an ordered array of waypoints.
 * @param count number of waypoints in @p points.
 * @return Total route distance, in nautical miles.
 * */
double route_distance(struct waypoints *points, int count);

/**
 * @brief Route distance of a set of waypoints, version 0
 *
 * Computes the route distance of a set of waypoints, navigated following 
 * great-circle arcs (the smallest distance between two points in the 
 * surface of the Earth).
 * Uses the spherical Earth, approximation, and the standard acos formula.
 * This function is deprecated, prefer @c route_distance() 
 *
 * @param points pointer to an ordered array of waypoints.
 * @param count number of waypoints in @p points.
 * @return Total route distance, in nautical miles.
 ** */
double route_distance_v0(struct waypoints *points, int count);

/**
 * @brief import route from file.
 *
 * Imports a route from a file, with example format:
 * KADOM;33°42'38.880000"S 150°18'0.000000"E 35000ft;
 *
 * @param file path to file.
 * @param points pointer to the array which will hold the data.
 * @param max_size maximum size of the array, after which points are not 
 * imported.
 * @return number of points imported, -1 in case of error.
 * */
int import(char* file, struct waypoints *points, size_t max_size);

/**
 * @brief import a route from file, with velocity column
 *
 * Identical to @c import(), but the last column contains velocity (TAS):
 * 7-FUNCHAL;32°37'59.99"N 16°53'60.00"W 30000ft;400km/h;
 *
 * @param file path to file.
 * @param points pointer to the array which will hold the position data.
 * @param velocities pointer to the array which will hold velocities.
 * @param max_size maximum size of both arrays, after which points are not 
 * imported.
 * @return number of points imported, -1 in case of error.
 */
int import_with_velocity(char* file, struct waypoints *points, 
				double *velocities, size_t max_size);

/**
 * @brief Compute next position given a set of references
 *
 * Calculates position at the next time-step, assuming all references are
 * held constant in this time (zero-order hold). The sampling time must be
 * small (up to 20km of distance travelled), since this function considers
 * a locally flat Earth. Does not consider wind.
 * The calculation is made with the formulas for rhumb lines (constant 
 * heading).  
 *
 * @param previous position at the current time-step
 * @param tas current True Air Speed in km/h
 * @param psi_ref True heading angle in radians
 * @param theta_ref Reference climb angle in radians
 * @param delta_t Sampling time in seconds
 * @return Position at the next time-step
 * */
struct waypoints next_position(struct waypoints previous, double tas, 
				double psi_ref, double theta_ref, double delta_t);

/**
 * @brief wraps an angle to the interval [-pi +pi]
 *
 * @param angle pointer to the angle to be wrapped, also in radians.
 * */
void wrap_pi(double * angle);

/**
 * @brief computes reference true heading angle for great-circle navigation
 *
 * This is also called initial bearing, the heading the aircraft must have
 * at @p pos1 to follow a great-circle route to @p pos2.  
 *
 * @param pos1 Current position
 * @param pos2 Destination
 * @return Reference heading, in radian.
 * */
double heading_angle(struct waypoints pos1, struct waypoints pos2);

/**
 * @brief computes reference climb angle.
 *
 * Follows the desired dynamics:
 * dh/dt = alpha x (target_height - current_height)
 *
 * Does not consider wind.
 *
 * @param h1 current height, in feet.
 * @param h_ref reference height, in feet.
 * @param tas True Air Speed, in km/h.
 * @return Climb angle, in radian.
 * */
double climb_angle(double h1, double h_ref, double tas);

/**
 * @brief Logs one line position data to file
 *
 * Data is written as this example:
 * 977.14s;40°7'12.256353"N 7°50'58.295191"W 33776.90ft;700.0km/h;36.23°;0.16°;
 *
 * @param os file pointer to open file.
 * @param pos Position.
 * @param t time in seconds.
 * @param heading heading angle in radian.
 * @param climb Climb angle in radian.
 * @param tas True Air Speed in km/h.
 * */
void log_data(FILE* os, struct waypoints pos, double t, double heading, 
				double climb, double tas);

/**
 * @brief Logs one line of error data to file
 *
 * Data is written as this example:
 * 668.57s;0.3765nm;
 *
 * @param os file pointer to open file.
 * @param error Position error, in nautical miles.
 * @param t time in seconds.
 * */
void log_errors(FILE* os, double error, double t);




#ifdef __cplusplus
}
#endif

#endif
