#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct waypoints{

	double height;
	double latitude;
	double longitude;

};


int main(int argc, char** argv){

	const int R = 3440; //nm
	char buffer[100];
	int i=0, j;
	int er;
	struct waypoints points[100];

	memset(points, 0, 100*sizeof(struct waypoints));

	FILE* fp = fopen("lis-frk.txt", "r");

	while(fgets(buffer, 100, fp)){

		for(j=0; buffer[j]!=';' ;j++) //Ignore waypoint name
			;
		er = sscanf(buffer+j+1, "%lf%*c%lf%*c%lf", &points[i].latitude, &points[i].longitude, &points[i].height);
		if(er != 3){
			printf("Error reading file, exiting\n");
			return -1;
		}
		i++;

	}
	fclose(fp);

	for(i--; i>=0; i--){

		printf("%lf-%lf-%lf\n", points[i].height, points[i].latitude, points[i].longitude);
	}
	

	return 0;
}
