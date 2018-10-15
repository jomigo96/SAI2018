#include <stdio.h>
#include <math.h>
#include "fms.h"

int main(int argc, char** argv){

	double result;
	struct waypoints points[100];
	double velocity[100];
	int i,j;
	char file[] = "../routes/part2.txt";
	const double torad = M_PI/180;
	const double nm2km = 1.85200;

	FILE* out;
	struct waypoints current_pos;
	struct waypoints pos_pair[2];
	double heading, climb, tas, t, dt, last_dt;
	int interval;

	if(argc < 2){
		printf("No input file provided, exiting.\n");
		return -1;
	}
	
	i = import(argv[1], points, 100);
	
	/*for(j=0; j<i; j++){
		printf("%lf : %lf : %lf\n", points[j].latitude / torad, points[j].longitude / torad, points[j].height);
	}*/
	
	result = route_distance_v0(points, i);
	
	if(result != -1){
		printf("---------------\n");
		printf("Total route distance: %lf\n", result);
	}else
		printf("An error occurred\n");
		
	result = route_distance(points, i);
	
	if(result != -1){
		printf("---------------\n");
		printf("Total route distance: %lf (vicenty)\n", result);
	}else
		printf("An error occurred\n");

	/********************** PART 2 **************************/
	printf("Starting part 2\n");
	
	i = import_with_velocity(file, points, velocity, 100);
	/*for(j=0; j<i; j++){
		printf("%lf : %lf : %lf : %lf\n", points[j].latitude / torad, points[j].longitude / torad, points[j].height, velocity[j]);
	}*/

	out = fopen("log.txt", "w");
	fprintf(out, "time;position;tas;heading_angle;climb_angle;\n");

	current_pos = points[0];
	t = 0.0;

	for(j=1; j<i; j++){
	
		tas = velocity[j-1];
		dt = delta_t(tas);
		pos_pair[0]=points[j-1];
		pos_pair[1]=points[j];
		result = route_distance(pos_pair, 2) * nm2km;
		interval = ceil(result/(tas*dt/3600.0));
		last_dt = 3600.0*(result-(interval-1)*tas*dt/3600.0)/tas;

		while(interval>0){

			if(interval == 1){
				dt = last_dt;
			}
			heading = heading_angle(current_pos, points[j]);
			climb = climb_angle(current_pos.height, points[j].height, tas);
			//printf("%lf %lf %lf\n", tas, heading, climb);			

			current_pos = next_position(current_pos, tas, heading, climb, dt);
			//printf("%lf - %lf - %lf\n", current_pos.latitude, current_pos.longitude, current_pos.height);
			t +=dt;
			log_data(out, current_pos, t, heading, climb, tas);
			interval--;
		}		
	
		fprintf(out, "Switched destination to waypoint %d\n", j+2);
	}
	fclose(out);
	printf("Finished\n");
	
	return 0;
}








