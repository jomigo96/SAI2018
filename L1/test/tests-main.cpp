#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../vicenty/fms.h"
#include <string.h>
#include <math.h>

TEST_CASE("Route distance tests"){
	

	char file[40];
	double result;
	//Maximum error percentage
	const double error_tolerance = 0;

	SECTION("Unexisting file"){
		strcpy(file, "nofile.txt");

		// Should return -1 when file doesn't exist
		CHECK( route_distance(file) == -1 );
	}

	SECTION("Chicago - Lisbon"){
		//Chicago-Lisbon route, expected value is
		result=3748.0;
		strcpy(file, "../routes/KORD-LPPT.txt");
		CHECK(abs(route_distance(file)-result)/result < error_tolerance);
	}

	SECTION("Auckland-Doha"){
		//Auckland-Doha route, expected value is
		result=7871.0;
		strcpy(file, "../routes/NZAA-OTBD.txt");	
		CHECK(abs(route_distance(file)-result)/result < error_tolerance);
	}

	SECTION("Toronto-Lisbon"){
		//Toronto-Lisbon route, expected value is
		result=3382.0;
		strcpy(file, "../routes/CYYZ-LPPT.txt");	
		CHECK(abs(route_distance(file)-result)/result < error_tolerance);
	}

	SECTION("Shanghai-Los Angels"){
		//Shanghai-LA route, expected value is
		result=5950.0;
		strcpy(file, "../routes/ZSPD-KLAX.txt");	
		CHECK(abs(route_distance(file)-result)/result < error_tolerance);
	}
}
