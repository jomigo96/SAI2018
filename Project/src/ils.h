/**
 * @file ils.h
 * @author João Gonçalves, Tiago Oliveira, Carolina Serra
 * @date 23 Jan 2019
 * 
 * @brief Header file for the ILS auxiliary functions
 *
 * Defines data structures and functions to operate on streamed data and generate
 * ILS reference signals. Also contains procedures to draw them on a SDL renderer.
 *
 * */

#ifndef ILS_H
#define ILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

# define PI 3.14159
# define DEG_to_RAD (PI/180.0)
# define RAD_to_DEG (180.0/PI)
# define NM_to_m 1852
# define max_num_rwys 10
# define fps 30
# define w_width 700
# define w_height 500

/**
 * @brief runway data
 * */
struct runway {
    double or_local;          /**< Runway orientation (magnetic)*/
    double orientacao;        /**< Runway orientation (true)*/
    double latitude_thr;      /**< Runway threshold latitude, in radian*/
    double longitude_thr;     /**< Runway threshold longitude, in radian*/
    double altitude_thr;      /**< Runway threshold height, in meter */
    double x_ecef_thr;        /**< Runway threshold x-position (ECEF)*/
    double y_ecef_thr;        /**< Runway threshold y-position (ECEF)*/
    double z_ecef_thr;        /**< Runway threshold z-position (ECEF)*/
    double gs;                /**< Glide slope, in radian*/
    double im;                /**< Inner marker location, in meter*/
    double mm;                /**< Middle marker location, in meter*/
    double om;                /**< Outer marker location, in meter*/
    double freq;              /**< Runway ILS frequency, in MHz*/
};

/**
 * @brief Geographical Position
 * */
struct position_gps {
    float latitude;        /**< Latitude, in radian*/
    float longitude;       /**< Longitude, in radian*/
    float altitude;        /**< Altitude, in meter*/
};

/**
 * @brief Relative Position to a referential
 * */
struct position {
    double x;        /**< X-offset in meter*/
    double y;        /**< Y-offset in meter*/
    double z;        /**< Z-offset in meter*/
};

/**
 * @brief imports runway data from text file to rwy array
 *
 * Reads each line of the text document, storing the data in the runway array
 * Counts the number of lines in the text documents (i.e. the number of runways), putting
 * it in the num_rwys variable
 *
 * @param file pointer to the name of the text file with runway data
 * @param rwy pointer to the ordered array of runways
 * @param num_rwys pointer to the total number of runways in the array
 * */
void import_info_runways(char* file, struct runway* rwy, int* num_rwys);

/**
 * @brief converts runway GPS coordinates into Earth Centered, Earth Fixed coordinates
 *
 * Completes the structure of each runway in the array with the obtained coordinates
 * 
 * @param rwy pointer to the ordered array of runways
 * @param num_rwys pointer to the total number of runways in the array
 * */
void runway_coordinates_to_ecef(struct runway* rwy, int num_rwys);

/**
 * @brief identifies the runway that corresponds to the selected frequency
 * 
 * For each runway in the rwy array checks if the runway frequency is equal to the 
 * selected frequency
 * If yes, gives the value of the position of the selected runway in the array to the 
 * variable sel_rwy
 * If none of the runways in the array have the desired frequency, sel_rwy is set to -1
 *
 * @param rwy pointer to the ordered array of runways
 * @param num_rwys total number of runways in the array
 * @param sel_freq frequency selected in the ILS system
 * @param sel_rwy pointer to the position of the selected runway in the array 
 * */
void detect_sel_runway(struct runway* rwy, int num_rwys, int sel_freq, int* sel_rwy);

/**
 * @brief converts GPS coordinates into Earth Centered, Earth Fixed coordinates
 * 
 *
 * @param p desired point's GPS coordinates
 * @param p_ecef pointer to aircraft coordinates in the Earth Centered, Earth Fixed 
 * referential
 * */
void coordenadas_gps_to_ecef(struct position_gps p, struct position* p_ecef);

/**
 * @brief converts Earth Centered, Earth Fixed coordinates into East North Up coordinates 
 * relative to the runway
 * 
 *
 * @param rwy pointer to the ordered array of runways
 * @param sel_rwy position of the selected runway in the array 
 * @param p_ecef aircraft coordinates in the Earth Centered, Earth Fixed referential
 * @param p_enu aircraft coordinates in the East North Up referential, relative to the runway
 * */
void coordenadas_ecef_to_enu(struct runway* rwy, int sel_rwy, struct position p_ecef, struct position* p_enu);

/**
 * @brief determines the distance between the aircraft and the runway
 * 
 * Uses the formula d=sqrt(x^2+y^2+z^2) to determine the distance
 * Uses relative coordinates of the aircraft to the runway
 *
 * @param p_enu aircraft coordinates in the East North Up referential, relative to the runway
 * @param dist_rwy pointer to the runway distance 
 * */
void distance_to_runway(struct position p_enu, double* dist_rwy);

/**
 * @brief checks if aircraft in position p_enu is captured by the localizer
 * 
 * Compares the position of the arcraft to the localizer's coverage
 * If aircraft is within the covered area of the localizer, in_loc is set to 1
 *
 * @param p_enu aircraft coordinates in the East North Up referential, relative to the runway
 * @param rwy pointer to the ordered array of runways
 * @param sel_rwy position of the selected runway in the array 
 * @param in_loc pointer to integer that defines whether or not the aircraft 
 * is captured by the  localizer
 * @param loc_ang angle between runway direction and aircraft position
 * */
void in_localizer(struct position p_enu, struct runway* rwy, int sel_rwy, int* in_loc, double* loc_ang);

/**
 * @brief normalizes angle to the interval [0, 2*pi]
 *
 * @param alpha pointer to the angle to be normalized, in radians
 * */
void normalizar_angulo(double* alpha);

/**
 * @brief checks if aircraft in position p_enu captures the glideslope
 * 
 * Compares the position of the arcraft to the glideslope's coverage
 * If aircraft is within the covered area of the glideslope, in_gs is set to 1
 *
 * @param p_enu aircraft coordinates in the East North Up referential, relative to the runway
 * @param rwy pointer to the ordered array of runways
 * @param sel_rwy position of the selected runway in the array 
 * @param dist_rwy distance between aircraft and runway
 * @param in_gs pointer to integer that defines whether or not the aircraft 
 * is captured by the  localizer
 * @param gs_ang angle between runway direction and aircraft's vertical position
 * @param loc_ang angle between runway direction and aircraft's horizontal position
 * */
void in_glide_slope(struct position p_enu, struct runway* rwy, int sel_rwy, double dist_rwy, int* in_gs, double* gs_ang, double loc_ang);

/**
 * @brief Moves the localizer marker 
 *
 * @param loc_angle Localizer angle
 * @param x_sum_pt pointer to the marker coordiates, to be changed
 * */
void movimento_ponteiro_localizer(double loc_ang, double* x_sum_pt);

/**
 * @brief Moves the glideslope marker 
 *
 * @param gs_angle glideslope angle
 * @param y_sum_pt pointer to the marker coordiates, to be changed
 * */
void movimento_ponteiro_glide_slope(double gs_ang, double* y_sum_pt);

/**
 * @brief checks if aircraft in passing the markers
 * 
 * Compares the position of the aircraft to the position of each marker
 * If the aircraft is at 340m or less from the outer marker, and under 1200m
 * of altitude, om_on is set to 1
 * If the aircraft is at 120m or less from the middle marker, and under 600m
 * of altitude, mm_on is set to 1
 * If the aircraft is at 70m or less from the inner marker, and under 300m
 * of altitude, im_on is set to 1
 *
 * @param p_enu aircraft coordinates in the East North Up referential, relative to the runway
 * @param rwy pointer to the ordered array of runways
 * @param sel_rwy position of the selected runway in the array 
 * @param loc_ang angle between runway direction and aircraft position
 * @param im_on pointer to the integer that defines whether or not the aircraft is 
 * passing the inner marker
 * @param mm_on pointer to the integer that defines whether or not the aircraft is 
 * passing the middle marker
 * @param om_on pointer to the integer that defines whether or not the aircraft is 
 * passing the outer marker
 * */
void in_markers(struct position p_enu, struct runway* rwy, int sel_rwy, double loc_ang, int* im_on, int* mm_on, int* om_on);

/**
 * @brief draws the base of the ILS indicator
 * 
 * Clears the renderer, adding a grey background to the window
 * Draws a black circle that will be the background of the indicator
 * Draws a white small circunference in the center that will be the first angle marker
 * Draws 3 white dots up, down, right and left of the circunference, equally spaced
 * between them, that will be other angle markers
 * 
 *
 * @param renderer pointer to renderer
 * */
void draw_indicator(SDL_Renderer* renderer);

/**
 * @brief draws a circle centered in (x0, y0) with an outer radius of raioext and an 
 * inner radius of raioint
 * 
 * Uses the equations x=x0+r*cos(teta) and y=y0+r*sin(teta) to draw every point of the 
 * desired circle, with raioint<r<raioext and 0<teta<2*PI
 * The larger the radius is, the more angles need to be drawn in the circle to get it 
 * completely filled, but that can also severely impact our program's speed
 * For that reason, the step by which the teta angle is increased in each cycle is 
 * inversely related to the current radius, with a 1/8 factor that was obtained through
 * trial an error to get the biggest step possible (minimizing time) but a completely
 * filled large circle
 * 
 *
 * @param renderer pointer to renderer
 * @param x0 horizontal coordinate of the circle's center
 * @param y0 vertical coordinate of the circle's center
 * @param raioext exterior radius of the circle
 * @param raioint interior radius of the circle
 * */
void draw_circle(SDL_Renderer* renderer, float x0, float y0, float raioext, float raioint);

/**
 * @brief draws the Course Deviation Indicator on the ILS indicator
 * 
 * Draws a vertical line deviated x_sum_pt pixels from the vertical axis of the indicator
 * Draws a horizontal line deviated y_sum_pt pixels from the horizontal axis of the indicator
 * 
 *
 * @param renderer pointer to renderer
 * @param x_sum_pt number of pixels that the vertical CDI should be deviated from 
 * the center of the indicator
 * @param y_sum_pt number of pixels that the horizontal CDI should be deviated from 
 * the center of the indicator
 * */
void draw_CDI(SDL_Renderer* renderer, double x_sum_pt, double y_sum_pt);

/**
 * @brief draws marker beacon lights either off or blinking
 * 
 * Draws three circles, corresponding to each marker light, in dark colours to represent 
 * the lights turned off
 * Counts the number of cycles, restarting the count after half a second
 * Checks if each marker is on, if yes, compares the cycle to a designated time for 
 * each marker, in order to get the markers lights to blink at different speeds
 * At that time, the cycle count is restarted so the lights will be turned off on the 
 * next cycle
 * At half of that time, it is drawn a circle in a bright colour, on top of the previously
 * drawn marker light, to represent the light turning on
 * 
 *
 * @param renderer pointer to renderer
 * @param im_on integer that defines whether or not the inner marker is activated
 * @param mm_on integer that defines whether or not the middle marker is activated
 * @param om_on integer that defines whether or not the outter marker is activated
 * @param b_on integer that defines whether or not the current marker's light is on
 * */
void draw_beacons(SDL_Renderer* renderer, int im_on, int mm_on, int om_on, int* b_on);

/**
 * @brief Draws text on the graphical window
 *
 * Uses a font object to render text in the display.
 *
 * @param renderer pointer to the renderer
 * @param x x-position, where the text should start (counting from the upper left corner)
 * @param y y-position, where the text should start (counting from the upper left corner)
 * @param angle rotation of the text from the horizontal
 * @param size scaling factor
 * @param text string to be rendered
 * @param font pointer to the font object
 * */
void draw_text(SDL_Renderer *renderer, int x, int y, float angle, int size, char* text, TTF_Font *font);

/**
 * @brief Draws the compass around the marker
 *
 * Draws the numbers 0 3 6 9 12 15 18 21 24 27 30 33 on a circle, which represent
 * the tens part of the full 360 degrees. Used to navigate on a given radial, which
 * in this case is the runway heading.
 * Calls draw_text() internally.
 *
 * @param renderer pointer to the renderer object
 * @param font pointer to the font object
 * @param course angle to be left on top
 * */
void draw_compass(SDL_Renderer *renderer, TTF_Font *font, float course);

/**
 * @brief draws GS and NAV flags on the ILS indicator
 * 
 * Draws two red boxes on the ILS indicator
 * Writes GS on the horizontal box
 * Depending on the value of the parameter bandeira, writes OFF or NAV 
 * on the vertical box
 *
 *
 * @param renderer pointer to renderer
 * @param bandeira integer that defines whether the NAV flag should be ON or OFF
 * @param font pointer to the font which will be used to write 
 * the flags
 * */
void draw_state(SDL_Renderer *renderer, int bandeira, TTF_Font *font);

/**
 * @brief writes the selected frequency on the top right corner of the window
 * 
 * Converts frequency float value to a string
 * Draws a black box on the top right corner of the window
 * Displays the string on the box
 *
 *
 * @param renderer pointer to renderer
 * @param frequency value of the selected frequency to be shown
 * @param font pointer to the font which will be used to write 
 * the selected frequency
 * */
void write_freq(SDL_Renderer *renderer, float frequency, TTF_Font *font);

/**
 * @brief defines buttons to change the selected frequency
 * 
 * Draws two boxes next to the frequency, a + in the top box and a - in the bottom
 * When the mouse button is pressed, gets the mouse position and compares it to 
 * the boxes' coordinates
 * Adds or removes 0.1 to the selected frequency depending on which box is clicked
 * Also polls for a window closing event, returning the state
 *
 *
 * @param renderer pointer to renderer
 * @param sel_frec pointer to the selected frequency
 * @return 0 if the 'x' was pressed, 1 otherwise
 * */
int botao(SDL_Renderer* renderer, int* sel_freq);


/**
 * @brief draws a black box where the markers will be placed and
 * writes each marker's identification by their right side, 
 *
 *
 * @param renderer pointer to renderer
 * @param font pointer to the font which will be used to write 
 * the marker's identification
 * */
void name_markers(SDL_Renderer* renderer, TTF_Font *font);

#endif //ILS_H
