#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fms.h"

#define BUF_SIZE 100

double route_distance_v0(struct waypoints *points, int count){
		
	int i;
	const int R = 3440; 
	double distance = 0;
	double alpha;	
		
	for(i=0; i<count-1; i++){

		alpha = acos(
					sin(points[i].latitude)*sin(points[i+1].latitude) +
					cos(points[i].latitude)*cos(points[i+1].latitude) * 
					cos(points[i+1].longitude - points[i].longitude)
					);
			
		distance += alpha * (R + points[i].height);
	}

	return distance;
	
}

double route_distance(struct waypoints *points, int count){
		
	int i;
	const int R = 3440; 
	double distance = 0;
	double alpha;	
		
	for(i=0; i<count-1; i++){

		alpha =
		  atan(
				sqrt( 
				   pow( cos(points[i+1].latitude)*
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

	return distance;
	
}


int import(char* file, struct waypoints *points, size_t max_size){

	FILE* fp;
	int i = 0;
	int j, er;
	int lat_deg, lat_min, lon_deg, lon_min;
	double lat_sec, lon_sec;
	char lat_char, lon_char;
	const double torad = M_PI/180;
	char buffer[BUF_SIZE];

	fp = fopen(file, "r");
	
	if(fp == NULL){
		return -1;
	}

	// Process one line at a time and convert units
	while(fgets(buffer, BUF_SIZE, fp)&&(i<=max_size)){

		for(j=0; buffer[j]!=';' ;j++) //Ignore waypoint name
			;
			
		er = sscanf(buffer+j+1, "%d°%d\'%lf\"%c %d°%d\'%lf\"%c %lf\ft\n", &lat_deg, &lat_min, &lat_sec, &lat_char, &lon_deg, &lon_min, &lon_sec, &lon_char, &points[i].height);
		if(er != 9)
			return -1;

		points[i].latitude = (lat_char=='N'?1:-1) * torad * (lat_deg+lat_min/60.0+lat_sec/3600.0);       
		points[i].longitude = (lon_char=='E'?1:-1) * torad * (lon_deg+lon_min/60.0+lon_sec/3600.0);
		points[i].height *= 0.000164578834; //Convert to nm
		i++;

	}
	fclose(fp);
	
	return i;
}
