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

	FILE *out1, *out2, *out3, *out4, *error2, *error3, *error4, *error_waypoints;
	struct waypoints current_pos_true, current_pos_no_ap, current_pos_ap, current_pos_cor;
	struct waypoints pos_pair[2];
	double heading, climb, tas, tas_noisy, t, dt, last_dt;
	int interval;


	/********************** PART 1 **************************/

	if(argc == 2){
	
		i = import(argv[1], points, 100);
	
		result = route_distance(points, i);
	
		if(result != -1){
			printf("---------------\n");
			printf("Total route distance for file %s: %lfnm\n", argv[1], result);
		}else
			printf("An error occurred\n");
	}else
		printf("No input file provided\n");



	/********************** PART 2 **************************/

	printf("Starting part 2\n\n");
	
	i = import_with_velocity(file, points, velocity, 100);

	out1 = fopen("logs/log_exact.txt", "w");
	out2 = fopen("logs/log_noisy.txt", "w");
	out3 = fopen("logs/log_ap.txt", "w");
	out4 = fopen("logs/log_corrected.txt", "w");
	error2 = fopen("logs/log_err_noisy.txt", "w");
	error3 = fopen("logs/log_err_ap.txt", "w");
	error4 = fopen("logs/log_err_corrected.txt", "w");
	error_waypoints = fopen("logs/log_err_waypoints.txt", "w");
	fprintf(out1, "time;position;tas;heading_angle;climb_angle;\n");
	fprintf(out2, "time;position;tas;heading_angle;climb_angle;\n");
	fprintf(out3, "time;position;tas;heading_angle;climb_angle;\n");
	fprintf(out4, "time;position;tas;heading_angle;climb_angle;\n");
	fprintf(error2, "time;position_error\n");
	fprintf(error3, "time;position_error\n");
	fprintf(error4, "time;position_error\n");

	current_pos_true = points[0];
	current_pos_ap = points[0];
	current_pos_cor = points[0];
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

			t += dt;
			tas_noisy = tas*(1+0.01*sin(2*M_PI*t/(20*60)));
			

			// noisy sensor but position is not updated based on it
			heading = heading_angle(current_pos_true, points[j]);
			climb = climb_angle(current_pos_true.height, points[j].height, tas_noisy);
			current_pos_no_ap = next_position(current_pos_true, tas_noisy, heading, climb, dt);
			log_data(out2, current_pos_no_ap, t, heading, climb, tas_noisy);

			// exact route
			heading = heading_angle(current_pos_true, points[j]);
			climb = climb_angle(current_pos_true.height, points[j].height, tas);
			current_pos_true = next_position(current_pos_true, tas, heading, climb, dt);
			log_data(out1, current_pos_true, t, heading, climb, tas);
			
			// noisy sensor and auto-pilot
			heading = heading_angle(current_pos_ap, points[j]);
			climb = climb_angle(current_pos_ap.height, points[j].height, tas_noisy);
			current_pos_ap = next_position(current_pos_ap, tas_noisy, heading, climb, dt);
			log_data(out3, current_pos_ap, t, heading, climb, tas_noisy);

			// noisy sensor but auto-pilot corrects position at the waypoints
			heading = heading_angle(current_pos_cor, points[j]);
			climb = climb_angle(current_pos_cor.height, points[j].height, tas_noisy);
			current_pos_cor = next_position(current_pos_cor, tas_noisy, heading, climb, dt);
			log_data(out4, current_pos_cor, t, heading, climb, tas_noisy);

			// log errors
			pos_pair[0] = current_pos_true;
			pos_pair[1] = current_pos_no_ap;
			log_errors(error2, fabs(route_distance(pos_pair, 2)), t);

			pos_pair[0] = current_pos_true;
			pos_pair[1] = current_pos_ap;
			log_errors(error3, fabs(route_distance(pos_pair, 2)), t);

			pos_pair[0] = current_pos_true;
			pos_pair[1] = current_pos_cor;
			log_errors(error4, fabs(route_distance(pos_pair, 2)), t);

			interval--;
		}		
	
		// log waypoint errors
		fprintf(out1, "Switched destination to waypoint %d\n", j+2);
		fprintf(out2, "Switched destination to waypoint %d\n", j+2);
		fprintf(out3, "Switched destination to waypoint %d\n", j+2);
		fprintf(out4, "Switched destination to waypoint %d and corrected position\n", j+2);

		fprintf(error_waypoints, "Errors at waypoint %d:\n", j+1);

		pos_pair[0]=current_pos_true;
		pos_pair[1]=current_pos_no_ap;
		result = fabs( route_distance(pos_pair, 2) );
		fprintf(error_waypoints, "Noisy and no auto-pilot              : %.6lfnm\n", result);

		pos_pair[1]=current_pos_ap;
		result = fabs( route_distance(pos_pair, 2) );
		fprintf(error_waypoints, "Noisy and auto-pilot                 : %.6lfnm\n", result);

		pos_pair[1]=current_pos_cor;
		result = fabs( route_distance(pos_pair, 2) );
		fprintf(error_waypoints, "Noisy and auto-pilot with correction : %.6lfnm\n\n", result);

		current_pos_cor = current_pos_true;

	}

	fclose(out1);
	fclose(out2);
	fclose(out3);
	fclose(out4);
	fclose(error2);
	fclose(error3);
	fclose(error4);
	fclose(error_waypoints);

	printf("Finished\n");
	
	return 0;
}








