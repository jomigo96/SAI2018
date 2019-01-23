#include <stdlib.h>
#include <signal.h>
#include "ils.h"
#include "reception_thread.h"

// Global variables
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int ready = 0;
struct position_gps position;

int main(int argc, char** argv){

	int running=1;
	int new=1;
	SDL_Renderer* renderer=NULL;
	SDL_Window* window;

	int num_rwys=0, sel_rwy, b_on=0;
	int bandeira=0, in_loc=0, in_gs=0, im_on=0, mm_on=0, om_on=0;
	int sel_freq=1095;
	double dist_rwy, loc_ang, gs_ang;
	double x_sum_pt=0, y_sum_pt=0;

	struct runway rwy[max_num_rwys];
	struct position_gps p;
	struct position p_ecef;
	struct position p_enu;

	if(argc != 3){
        fprintf(stderr, "usage: %s <runway file path> <port>\n", argv[0]);
        exit(1);
	}
	uint32_t port = atoi(argv[2]);

	// Launch data gatherer thread
	pthread_t tid;
	pthread_create(&tid, NULL, reception_thread, &port);
	
	struct sigaction action;
	sigaction(SIGINT, NULL, &action);
	SDL_Init ( SDL_INIT_VIDEO );
	TTF_Init();
	sigaction(SIGINT, &action, NULL); //SDL overwrites the CTRL-C signal
	
	window = SDL_CreateWindow("Indicador ILS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w_width, w_height, SDL_WINDOW_OPENGL );
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	TTF_Font *font = TTF_OpenFont("FreeMonoOblique.ttf", 24);
	if(font == NULL){
		fprintf(stderr, "Error opening font\n");
		exit(1);
	}

	import_info_runways(argv[1], rwy, &num_rwys);
	runway_coordinates_to_ecef(rwy, num_rwys);

	while(running){
		bandeira=0; in_loc=0; in_gs=0; om_on=0; mm_on=0; im_on=0;
		detect_sel_runway(rwy, num_rwys, sel_freq, &sel_rwy);

		if(sel_rwy!=-1)
		{
			//rececao da posição;
			pthread_mutex_lock(&m);
			if(ready){
				new=1;
				p=position;
				ready=0;
			}
			pthread_mutex_unlock(&m);

			if(new){
				new=0;

				SDL_RenderClear(renderer);

				coordenadas_gps_to_ecef(p, &p_ecef);
				coordenadas_ecef_to_enu(rwy, sel_rwy, p_ecef, &p_enu);

				distance_to_runway(p_enu, &dist_rwy);
				if(dist_rwy<18*NM_to_m)
				{
					in_localizer(p_enu, rwy, sel_rwy, &in_loc, &loc_ang);
					if (in_loc==1)
					{
						movimento_ponteiro_localizer(loc_ang, &x_sum_pt);
						in_glide_slope(p_enu, rwy, sel_rwy, dist_rwy, &in_gs, &gs_ang);
						if (in_gs==1)
							movimento_ponteiro_glide_slope(gs_ang, &y_sum_pt);
						in_markers(p_enu, rwy, sel_rwy, loc_ang, &im_on, &mm_on, &om_on);
					} else
						bandeira=1;
				} else
					bandeira=1;
			} else
				bandeira=1;

		}else
			bandeira=1;

		draw_indicator(renderer);
		draw_compass(renderer, font, rwy[sel_rwy].or_local);
		write_freq(renderer, sel_freq/10.0, font);
		name_markers( renderer, font);
		draw_beacons(renderer, im_on, mm_on, om_on, &b_on);
		draw_state(renderer, bandeira, font);
		draw_CDI(renderer, x_sum_pt, y_sum_pt);
		running = botao(renderer, &sel_freq);
		SDL_RenderPresent(renderer);

		//running = quit();
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	TTF_Quit();
	pthread_cancel(tid);
	return 0;
}

