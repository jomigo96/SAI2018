#include <stdio.h>
#include "fms.h"

int main(int argc, char** argv){

	double result;

	if(argc < 2){
		printf("No input file provided, exiting.\n");
		return -1;
	}
	
	result = route_distance(argv[1]);
	
	if(result != -1){
		printf("---------------\n");
		printf("Total route distance: %lf\n", result);
	}else
		printf("An error occurred\n");
	
	return 0;
}
