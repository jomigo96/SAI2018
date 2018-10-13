#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../fms/fms.h"
#include <string.h>
#include <math.h>

//Maximum error percentage
const double error_tolerance = 0;

TEST_CASE("Route distance tests, vicenty's formula"){
	

	char file[40];
	double result;
	struct waypoints points[150];
	int c;

	SECTION("Unexisting file"){
		strcpy(file, "nofile.txt");

		// Should return -1 when file doesn't exist
		CHECK( import(file, points, 150) == -1 );
	}
	SECTION("New Orleans - Paris"){
		strcpy(file, "../routes/kmsy-lfpg.txt");
		result=4227.0;
		c=import(file, points, 150);
		CHECK(abs(route_distance(points, c)-result)/result < error_tolerance);
	}
	

	SECTION("Chicago - Lisbon"){
		strcpy(file, "../routes/kord-lppt.txt");
		result=3748.0;
		c=import(file, points, 150);
		CHECK(abs(route_distance(points, c)-result)/result < error_tolerance);
	}

	SECTION("Auckland-Doha"){
		strcpy(file, "../routes/nzaa-otbd.txt");
		result=7871.0;
		c=import(file, points, 150);
		CHECK(abs(route_distance(points, c)-result)/result < error_tolerance);
	}


	SECTION("Shanghai-Los Angels"){
		strcpy(file, "../routes/zspd-klax.txt");
		result=5778.0;
		c=import(file, points, 150);
		CHECK(abs(route_distance(points, c)-result)/result < error_tolerance);
	}
}

TEST_CASE("Route distance tests, regular formula"){
	

	char file[40];
	double result;
	struct waypoints points[150];
	int c;
	
	SECTION("Unexisting file"){
		strcpy(file, "nofile.txt");

		// Should return -1 when file doesn't exist
		CHECK( import(file, points, 150) == -1 );
	}
	SECTION("New Orleans - Paris"){
		strcpy(file, "../routes/kmsy-lfpg.txt");
		result=4227.0;
		c=import(file, points, 150);
		CHECK(abs(route_distance_v0(points, c)-result)/result < error_tolerance);
	}
	

	SECTION("Chicago - Lisbon"){
		strcpy(file, "../routes/kord-lppt.txt");
		result=3748.0;
		c=import(file, points, 150);
		CHECK(abs(route_distance_v0(points, c)-result)/result < error_tolerance);
	}

	SECTION("Auckland-Doha"){
		strcpy(file, "../routes/nzaa-otbd.txt");
		result=7871.0;
		c=import(file, points, 150);
		CHECK(abs(route_distance_v0(points, c)-result)/result < error_tolerance);
	}


	SECTION("Shanghai-Los Angels"){
		strcpy(file, "../routes/zspd-klax.txt");
		result=5778.0;
		c=import(file, points, 150);
		CHECK(abs(route_distance_v0(points, c)-result)/result < error_tolerance);
	}
}
