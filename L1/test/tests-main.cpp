#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../fms/fms.h"
#include <string.h>
#include <math.h>

//Maximum error percentage
const double error_tolerance = 0.01;

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


TEST_CASE("Wrapping angles"){

	double angle;

	angle = 1.2;
	wrap_pi(&angle);
	CHECK(abs(angle-1.2) < 0.001);

	angle = -1.2;
	wrap_pi(&angle);
	CHECK(abs(angle - -1.2) < 0.001);

	angle = -3*M_PI;
	wrap_pi(&angle);
	CHECK(abs(angle-M_PI) < 0.001);

	angle = 6.5*M_PI;
	wrap_pi(&angle);
	CHECK(abs(angle-M_PI/2) < 0.001);
}

TEST_CASE("Position integration"){

	struct waypoints prev, next;
	double v, phi, theta, t;
	const double torad = M_PI/180;
	const double er_tol = 0.00174; //0.1deg

	prev.latitude = 53.32056 * torad;
	prev.longitude = -1.729722 * torad;
	prev.height = 30000; //ft
	v=700;//kmh
	theta=0;
	t=51.42;
	phi=90 * torad;

	next = next_position(prev, v, phi, theta, t);

	CHECK(abs(next.latitude - 53.32056*torad) < er_tol);
	CHECK(abs(next.longitude - -1.579167*torad) < er_tol);
	CHECK(abs(next.height - prev.height) < 50);
	std::cout << next.latitude/torad << " -- " << next.longitude/torad << " -- " << next.height << std::endl;
}
