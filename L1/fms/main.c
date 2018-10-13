#include <stdio.h>
#include <math.h>
#include "fms.h"

int main(int argc, char** argv){

	double result;
	struct waypoints points[100];
	int i,j;
	const double torad = M_PI/180;

	if(argc < 2){
		printf("No input file provided, exiting.\n");
		return -1;
	}
	
	i = import(argv[1], points, 150);
	
	for(j=0; j<i; j++){
		printf("%lf : %lf : %lf\n", points[j].latitude / torad, points[j].longitude / torad, points[j].height / 0.000164578834);
	}
	
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
	
	return 0;
}
