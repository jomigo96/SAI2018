/**
 * @file fms.c
 * @author João Gonçalves, Tiago Oliveira, Carolina Serra
 * @date 31 Oct 2018
 *
 * @brief Implementation file for the Flight Management System Library
 *
 * @see https://github.com/jomigo96/SAI2018
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fms.h"

#define BUF_SIZE 100

double route_distance_v0(struct waypoints *points, int count){
		
	int i;
	const int R = 3440; 
	const double ft2nm = 0.000164578834;
	double distance = 0;
	double alpha;	
		
	for(i=0; i<count-1; i++){

		alpha = acos(
					sin(points[i].latitude)*sin(points[i+1].latitude) +
					cos(points[i].latitude)*cos(points[i+1].latitude) * 
					cos(points[i+1].longitude - points[i].longitude)
					);
			
		distance += alpha * (R + points[i].height*ft2nm);
	}

	return distance;
	
}

double route_distance(struct waypoints *points, int count){
		
	int i;
	const int R = 3440; 
	const double ft2nm = 0.000164578834;
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
			
		distance += alpha * (R + points[i].height*ft2nm);
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
			
		er = sscanf(buffer+j+1, "%d°%d\'%lf\"%c %d°%d\'%lf\"%c %lfft;\n",
					   	&lat_deg, &lat_min, &lat_sec, &lat_char, &lon_deg,
					   	&lon_min, &lon_sec, &lon_char, &points[i].height);
		if(er != 9)
			return -1;

		points[i].latitude = (lat_char=='N'?1:-1) * torad *
			   	(lat_deg+lat_min/60.0+lat_sec/3600.0);       
		points[i].longitude = (lon_char=='E'?1:-1) * torad *
			   	(lon_deg+lon_min/60.0+lon_sec/3600.0);
		i++;

	}
	fclose(fp);
	
	return i;
}

int import_with_velocity(char* file, struct waypoints *points,
			   	double *velocities, size_t max_size){

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
			
		er = sscanf(buffer+j+1,
					   	"%d°%d\'%lf\"%c %d°%d\'%lf\"%c %lfft;%lfkm/h;\n",
					   	&lat_deg, &lat_min, &lat_sec, &lat_char, &lon_deg,
					   	&lon_min, &lon_sec, &lon_char, &points[i].height,
					   	&velocities[i]);
		if(er != 10){
			return -1;
		}

		points[i].latitude = (lat_char=='N'?1:-1) * torad *
			   	(lat_deg+lat_min/60.0+lat_sec/3600.0);       
		points[i].longitude = (lon_char=='E'?1:-1) * torad *
			   	(lon_deg+lon_min/60.0+lon_sec/3600.0);
		i++;

	}
	fclose(fp);
	
	return i;


}


struct waypoints next_position(struct waypoints previous, double tas,
			   	double psi_ref, double theta_ref, double delta_t){

	const int R = 6371; //km
	const double ft2km = 0.0003048;

	struct waypoints n_pos;
	double v_north;
	double v_east;
	double v_up;
	double distance;
	double delta;
	double q;
	double d_psi;
	
	v_north = tas * cos(theta_ref) * cos(psi_ref);
	v_east  = tas * cos(theta_ref) * sin(psi_ref);		
	v_up = tas * sin(theta_ref);

	distance = delta_t/3600.0 * sqrt( pow(v_north, 2) + pow(v_east, 2) );
	delta = distance/(previous.height*ft2km + R);

	n_pos.latitude = previous.latitude + delta*cos(psi_ref);

	d_psi = log(tan(M_PI/4.0+n_pos.latitude/2.0)/
					tan(M_PI/4.0+previous.latitude/2.0));
	q = (fabs(d_psi) > 10e-9) ? (n_pos.latitude-previous.latitude)/d_psi :
		   	cos(previous.latitude);

	n_pos.longitude = previous.longitude + delta*sin(psi_ref)/q;

	wrap_pi(&n_pos.longitude);
	wrap_pi(&n_pos.latitude);

	n_pos.height = (previous.height + v_up/ft2km * delta_t/3600.0);
	
	return n_pos;

}


void wrap_pi(double * angle){

	if(*angle <= -M_PI)
		while((*angle += 2*M_PI) <= -M_PI)
			;
	if(*angle > M_PI)
		while((*angle -= 2*M_PI) > M_PI)
			;

}


double heading_angle(struct waypoints pos1, struct waypoints pos2){

	const double d_long = pos2.longitude-pos1.longitude;
	double psi = atan2(sin(d_long)*cos(pos2.latitude),
				   	cos(pos1.latitude)*sin(pos2.latitude) -
				   	sin(pos1.latitude)*cos(pos2.latitude)*cos(d_long) );
	wrap_pi(&psi);
	return psi;
}


double climb_angle(double h1, double h_ref, double tas){

	const double ft2km = 0.0003048; 

	return asin(ALPHA*(h_ref-h1)*ft2km/tas);

}


void log_data(FILE* os, struct waypoints pos, double t, double heading,
			   	double climb, double tas){

	const double torad = M_PI/180.0;
	int lat_deg, lon_deg, lat_min, lon_min;
	double lat_sec, lon_sec;
	char lat_char, lon_char;
	
	lat_char = (pos.latitude >= 0) ? 'N' : 'S';	
	lon_char = (pos.longitude >= 0) ? 'E' : 'W';
	pos.latitude = fabs(pos.latitude)/torad;
	pos.longitude = fabs(pos.longitude)/torad;
	lat_deg = floor(pos.latitude);
	lon_deg = floor(pos.longitude);
	lat_min = floor((pos.latitude-lat_deg)*60.0);
	lon_min = floor((pos.longitude-lon_deg)*60.0);
	lat_sec = (pos.latitude - lat_deg - lat_min/60.0)*3600.0;
	lon_sec = (pos.longitude - lon_deg - lon_min/60.0)*3600.0;
	

	fprintf(os,	"%.2lfs;%d°%d'%.6lf\"%c %d°%d'%.6lf\"%c %.2lfft"
					";%.1lfkm/h;%.2lf°;%.2lf°;\n", t, lat_deg, lat_min,
				   	lat_sec, lat_char, lon_deg, lon_min, lon_sec, lon_char,
				   	pos.height, tas, heading/torad, climb/torad);

}

void log_errors(FILE* os, double error, double t){

	fprintf(os, "%.2lfs;%.4lfnm;\n", t, error);

}
