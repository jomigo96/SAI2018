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


struct runway {
	double orientacao;	//em [rad]
	double latitude_thr;	//em [rad]
	double longitude_thr;	//em [rad]
	double altitude_thr;	//em [m]
	double x_ecef_thr;	//em [m]
	double y_ecef_thr;	//em [m]
	double z_ecef_thr;	//em [m]
	double gs;		//em [rad]
	double im;		//em [m]
	double mm;		//em [m]
	double om;		//em [m]
	double freq;		//em [MHz]
};

struct position_gps {
	float latitude;	//em [rad]
	float longitude;	//em [rad]
	float altitude;	//em [m]
};

struct position {
	double x;		//em [m]
	double y;		//em [m]
	double z;		//em [m]
};

void import_info_runways(char* file, struct runway* rwy, int* num_rwys);
void runway_coordinates_to_ecef(struct runway* rwy, int num_rwys);
void detect_sel_runway(struct runway* rwy, int num_rwys, double sel_freq, int* sel_rwy);
void coordenadas_gps_to_ecef(struct position_gps p, struct position* p_ecef);
void coordenadas_ecef_to_enu(struct runway* rwy, int sel_rwy, struct position p_ecef, struct position* p_enu);
void distance_to_runway(struct position p_enu, double* dist_rwy);
void in_localizer(struct position p_enu, struct runway* rwy, int sel_rwy, int* in_loc, double* loc_ang);
void normalizar_angulo(double* alpha);
void in_glide_slope(struct position p_enu, struct runway* rwy, int sel_rwy, double dist_rwy, int* in_gs, double* gs_ang);
void movimento_ponteiro_localizer(double loc_ang, double* x_sum_pt);
void movimento_ponteiro_glide_slope(double gs_ang, double* y_sum_pt);
void in_markers(struct position p_enu, struct runway* rwy, int sel_rwy, double loc_ang, int* im_on, int* mm_on, int* om_on);
void draw_indicator(SDL_Renderer* renderer);
void draw_circle(SDL_Renderer* renderer, float x0, float y0, float raioext, float raioint);
void draw_CDI(SDL_Renderer* renderer, double x_sum_pt, double y_sum_pt);
void draw_beacons(SDL_Renderer* renderer, int im_on, int mm_on, int om_on, int* b_on);
int quit(void);

void draw_text(SDL_Renderer *renderer, int x, int y, float angle, char* text, TTF_Font *font);
void draw_compass(SDL_Renderer *renderer, TTF_Font *font, float course);
#endif //ILS_H
