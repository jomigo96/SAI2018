/**
 * @file main.c
 * @author João Gonçalves, Tiago Oliveira, Carolina Serra
 * @date 31 Oct 2018
 * 
 * @brief Program to solve laboratory work questions.
 *
 *
 * Part 1 computes the total distance of a route defined in a .txt file. 
 * The file is passed as a command line argument.
 *
 * Part 2 assembles a Dead-reckoning navigation system.
 *
 * @see https://github.com/jomigo96/SAI2018
 * */

#include <stdio.h>
#include <math.h>
#include "fms.h"

int main(int argc, char** argv){

	double result;
	struct waypoints points[100];
	double velocity[100];
	int i,j;
	char file[] = "../routes/part2.txt";
	//const double torad = M_PI/180;
	const double nm2km = 1.85200;

	// Logs
	FILE *out1, *out2, *out3, *out4, *error2, *error3, *error4, 
		 *error_waypoints;

	// Positions
	struct waypoints
			current_pos,				// Position for simple navigation
										//		(Point 1)
			current_pos_noisy,			// Position for navigation with
										//		noisy sensor (Point 2)
			current_pos_feedback,		// Position for navigation with 
										//		velocity control (Point 3)
			current_pos_true_feedback,	// True position with velocity
										//		control, for computing
										//		errors
			current_pos_cor,			// Position for navigation with
										//		position update at
										//		waypoints (Point 4)
			current_pos_true_cor;		// True position with position
										//		update at waypoints

	struct waypoints pos_pair[2];
	double heading, climb, tas, tas_noisy, tas_feedback, vm_feedback, t,
		   dt, last_dt;
	int interval;

	/********************** PART 1 **************************/
	
	if(argc == 2){
	
		i = import(argv[1], points, 100);
	
		result = route_distance(points, i);
	
		if(result != -1){
			printf("---------------\n");
			printf("Total route distance for file %s: %lfnm\n", argv[1],
						   	result);
		}else
			printf("An error occurred\n");
	}else
		printf("No input file provided\n");



	/********************** PART 2 **************************/

	printf("Starting part 2\n\n");
	
	// Import route from file
	i = import_with_velocity(file, points, velocity, 100);

	// Open log files
	out1 = fopen("logs/log_simple.txt", "w");
	out2 = fopen("logs/log_noisy.txt", "w");
	out3 = fopen("logs/log_feedback.txt", "w");
	out4 = fopen("logs/log_corrected.txt", "w");
	error2 = fopen("logs/log_err_noisy.txt", "w");
	error3 = fopen("logs/log_err_feedback.txt", "w");
	error4 = fopen("logs/log_err_corrected.txt", "w");
	error_waypoints = fopen("logs/log_err_waypoints.txt", "w");
	fprintf(out1, "time;position;tas;heading_angle;climb_angle;\n");
	fprintf(out2, "time;position;tas;heading_angle;climb_angle;\n");
	fprintf(out3, "time;position;tas;heading_angle;climb_angle;\n");
	fprintf(out4, "time;position;tas;heading_angle;climb_angle;\n");
	fprintf(error2, "time;position_error\n");
	fprintf(error3, "time;position_error\n");
	fprintf(error4, "time;position_error\n");

	// Initialize variables
	current_pos = points[0];
	current_pos_noisy = points[0];
	current_pos_feedback = points[0];
	current_pos_true_feedback = points[0];
	current_pos_cor = points[0];
	current_pos_true_cor = points[0];
	t = 0.0;
	vm_feedback = velocity[0];
	tas_feedback = velocity[0];

	// Loop through route
	for(j=1; j<i; j++){
	
		tas = velocity[j-1];
		dt = 10.0/tas*3600.0;
		pos_pair[0]=points[j-1];
		pos_pair[1]=points[j];
		result = route_distance(pos_pair, 2) * nm2km;
		interval = ceil(result/(tas*dt/3600.0));
		last_dt = 3600.0*(result-(interval-1)*tas*dt/3600.0)/tas;

		// Compute new positions at sample rate dt until the waypoint
		// is reached
		while(interval>0){

			if(interval == 1){
				dt = last_dt;
			}

			t += dt;
			tas_noisy = tas*(1+0.01*sin(2*M_PI*t/(20*60)));
			
			tas_feedback = (tas_feedback + velocity[j-1] - vm_feedback);
		   	// uses previous sample of vm_feedback

			vm_feedback = tas_feedback*(1+0.01*sin(2*M_PI*t/(20*60)));
			
			// Exact route (Point 1)
			heading = heading_angle(current_pos, points[j]);
			climb = climb_angle(current_pos.height, points[j].height, tas);
			current_pos = next_position(current_pos, tas, heading, climb,
						   	dt);
			log_data(out1, current_pos, t, heading, climb, tas);
			
			// Noisy sensor (Point 2)
			heading = heading_angle(current_pos_noisy, points[j]);
			climb = climb_angle(current_pos_noisy.height, points[j].height,
						   	tas_noisy);
			current_pos_noisy = next_position(current_pos_noisy, tas_noisy,
						   	heading, climb, dt);
			log_data(out2, current_pos_noisy, t, heading, climb, tas_noisy);

			// With velocity feedback control (Point 3)
			heading = heading_angle(current_pos_feedback, points[j]);
			climb = climb_angle(current_pos_feedback.height,
						   	points[i].height, vm_feedback);
			current_pos_feedback = next_position(current_pos_feedback,
						   	vm_feedback, heading, climb, dt);
			log_data(out3, current_pos_feedback, t, heading, climb,
						   	vm_feedback);

			// Exact route considering the aircraft's true velocity
			current_pos_true_feedback = next_position(
							current_pos_true_feedback, tas_feedback,
						   	heading, climb, dt);

			// With position correction at waypoints (Point 4)
			heading = heading_angle(current_pos_cor, points[j]);
			climb = climb_angle(current_pos_cor.height, points[j].height,
						   	vm_feedback);
			current_pos_cor = next_position(current_pos_cor, vm_feedback,
						   	heading, climb, dt);
			log_data(out4, current_pos_cor, t, heading, climb, vm_feedback);

			// Exact route considering the aircraft's true velocity
			current_pos_true_cor = next_position(current_pos_true_cor,
						   	tas_feedback, heading, climb, dt);

			// Log errors
			pos_pair[0] = current_pos;
			pos_pair[1] = current_pos_noisy;
			log_errors(error2, fabs(route_distance(pos_pair, 2)), t);

			pos_pair[0] = current_pos_true_feedback;
			pos_pair[1] = current_pos_feedback;
			log_errors(error3, fabs(route_distance(pos_pair, 2)), t);

			pos_pair[0] = current_pos_true_cor;
			pos_pair[1] = current_pos_cor;
			log_errors(error4, fabs(route_distance(pos_pair, 2)), t);

			interval--;
		}		
	
		// Log waypoint errors
		fprintf(out1, "Switched destination to waypoint %d\n", j+2);
		fprintf(out2, "Switched destination to waypoint %d\n", j+2);
		fprintf(out3, "Switched destination to waypoint %d\n", j+2);
		fprintf(out4, "Switched destination to waypoint %d and corrected"
						" position\n", j+2);

		fprintf(error_waypoints, "Errors at waypoint %d:\n", j+1);

		pos_pair[0]=current_pos;
		pos_pair[1]=current_pos_noisy;
		result = fabs( route_distance(pos_pair, 2) );
		fprintf(error_waypoints, "Noisy sensor readings"
						"                       : %.6lfnm\n", result);

		pos_pair[0]=current_pos_true_feedback;
		pos_pair[1]=current_pos_feedback;
		result = fabs( route_distance(pos_pair, 2) );
		fprintf(error_waypoints, "Noisy sensor readings with feedback "
						"control : %.6lfnm\n", result);

		pos_pair[0]=current_pos_true_cor;
		pos_pair[1]=current_pos_cor;
		result = fabs( route_distance(pos_pair, 2) );
		fprintf(error_waypoints, "With position update at the waypoints"
						"       : %.6lfnm\n\n", result);

		// Correct position at the waypoint
		current_pos_cor = current_pos_true_cor;

	}

	// clean-up
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
