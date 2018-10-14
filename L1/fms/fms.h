#ifndef __FMSLIB_H_
#define __FMSLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

struct waypoints{

	double height;
	double latitude;
	double longitude;

};

double route_distance(struct waypoints *points, int count);
double route_distance_v0(struct waypoints *points, int count);
int import(char* file, struct waypoints *points, size_t max_size);
double delta_t(double velocity);
struct waypoints next_position(struct waypoints previous, double tas, double psi_ref, double theta_ref, double delta_t);
void wrap_pi(double * angle);

#ifdef __cplusplus
}
#endif

#endif
