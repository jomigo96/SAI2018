#ifndef __FMSLIB_H_
#define __FMSLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ALPHA 5
// alpha has units /h

struct waypoints{

	double height;
	double latitude;
	double longitude;

};

double route_distance(struct waypoints *points, int count);
double route_distance_v0(struct waypoints *points, int count);
int import(char* file, struct waypoints *points, size_t max_size);
int import_with_velocity(char* file, struct waypoints *points, double *velocities, size_t max_size);
double delta_t(double velocity);
struct waypoints next_position(struct waypoints previous, double tas, double psi_ref, double theta_ref, double delta_t);
void wrap_pi(double * angle);
double heading_angle(struct waypoints pos1, struct waypoints pos2);
double climb_angle(double h1, double h_ref, double tas);
void log_data(FILE* os, struct waypoints pos, double t, double heading, double climb, double tas);
void log_errors(FILE* os, double error, double t);




#ifdef __cplusplus
}
#endif

#endif
