#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fms.h"

#define BUF_SIZE 100
#define DATA_SIZE 100

struct waypoints{

	double height;
	double latitude;
	double longitude;

};

double route_distance(char* file){
		
	const int R = 3440; 					//Earth radius in nm
	char buffer[BUF_SIZE];					//Buffer to read data
	unsigned size;							//Number of read points
	int i=0, j;
	int er;
	FILE* fp;
	struct waypoints points[DATA_SIZE];		//Data structure
	memset(points, 0, 100*sizeof(struct waypoints));
	
	double distance=0;						//Distance in nm
	double alpha;							//Earth center angle in rad
	const double torad = M_PI/180;			//Conversion factor deg->rad
		
		
	fp = fopen(file, "r");
	
	if(fp == NULL){
		printf("Error opening file \"%s\", exiting.\n", file);
		return -1;
	}

	// Process one line at a time and convert units
	while(fgets(buffer, BUF_SIZE, fp)){

		for(j=0; buffer[j]!=';' ;j++) //Ignore waypoint name
			;
		er = sscanf(buffer+j+1, "%lf%*c%lf%*c%lf", &points[i].latitude, 
							   &points[i].longitude, &points[i].height);
		if(er != 3){
			printf("Error reading file, exiting\n");
			return -1;
		}
		points[i].latitude *= torad;        //Convert to radians
		points[i].longitude *= torad;
		points[i].height *= 0.000164578834; //Convert to nm
		i++;

	}
	fclose(fp);
	size=i;
	
	//Calculate distance
	for(i=0; i<(size-1); i++){

		alpha =
		  atan(
				sqrt( 
				   pow( cos(points[i].latitude)*
				   sin(points[i+1].longitude-points[i].longitude), 2 ) +
				   pow(cos(points[i].latitude)*sin(points[i+1].latitude)
				   - sin(points[i].latitude)*cos(points[i+1].latitude) *
				   cos(points[i+1].longitude-points[i].longitude), 2) 
				    )
				/(sin(points[i].latitude)*sin(points[i+1].latitude) + 
					cos(points[i].latitude)*cos(points[i+1].latitude)*
					cos(points[i+1].longitude-points[i].longitude)
				 )
			   );
			
		distance += alpha * (R + points[i].height);
	}
	
	//return result
	return distance;
}
