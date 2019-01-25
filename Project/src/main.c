/**
 * @file main.c
 * @author João Gonçalves, Tiago Oliveira, Carolina Serra
 * @date 23 Jan 2019
 * 
 * @brief Main funtion for the ILS emulator
 *
 * */

#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "ils.h"
#include "reception_thread.h"
#include <SDL_mixer.h>

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
	int in_loc=0, in_gs=0, im_on=0, mm_on=0, om_on=0;
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
	int last_timestamp=0;

	// Launch data gatherer thread
	pthread_t tid;
	pthread_create(&tid, NULL, reception_thread, &port);
	
	struct sigaction action;
	sigaction(SIGINT, NULL, &action);
	SDL_Init ( SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	TTF_Init();
	sigaction(SIGINT, &action, NULL); //SDL overwrites the CTRL-C signal
	
	window = SDL_CreateWindow("Indicador ILS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w_width, w_height, SDL_WINDOW_OPENGL );
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	TTF_Font *font = TTF_OpenFont("FreeMonoOblique.ttf", 24);
	if(font == NULL){
		fprintf(stderr, "Error opening font\n");
		exit(1);
	}
	int flags = MIX_INIT_MP3, result;
	Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
	if (flags != (result = Mix_Init(flags))) {
        printf("Could not initialize mixer (result: %d).\n", result);
        printf("Mix_Init: %s\n", Mix_GetError());
        exit(1);
    }

	Mix_Music *audio_outer, *audio_middle, *audio_inner;
	audio_outer = Mix_LoadMUS("outer.mp3");
	audio_middle = Mix_LoadMUS("middle.mp3");
	audio_inner = Mix_LoadMUS("inner.mp3");
	if((audio_outer == NULL)||(audio_middle == NULL)||(audio_inner == NULL)){
		printf("Error opening music file\n");
		exit(1);
	}
	//Mix_PlayMusic(audio_outer, 1);

	import_info_runways(argv[1], rwy, &num_rwys);
	runway_coordinates_to_ecef(rwy, num_rwys);

	while(running){
		om_on=0; mm_on=0; im_on=0; in_loc=0; in_gs=0;
		detect_sel_runway(rwy, num_rwys, sel_freq, &sel_rwy);

		if(sel_rwy!=-1)
		{
			//rececao da posição

			if(ready){ //reads are thread-safe

				pthread_mutex_lock(&m);
				new=1;
				p=position; // Copy to local memory
				ready=0;
				pthread_mutex_unlock(&m);
				last_timestamp = time(NULL);
			}

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
						in_glide_slope(p_enu, rwy, sel_rwy, dist_rwy, &in_gs, &gs_ang, loc_ang);
						if (in_gs==1)
							movimento_ponteiro_glide_slope(gs_ang, &y_sum_pt);
						in_markers(p_enu, rwy, sel_rwy, loc_ang, &im_on, &mm_on, &om_on);
					} 
				} 
			}else if((time(NULL)-last_timestamp) > 2){
                in_gs = 0;
                in_loc = 0;
            }
			  

		}

		if(om_on){
			Mix_PlayMusic(audio_outer, 1);
		}
		if(mm_on){
			Mix_PlayMusic(audio_middle, 1);
		}
		if(im_on){
			Mix_PlayMusic(audio_inner, 1);
		}
		if(!om_on && !mm_on && !im_on){
			Mix_HaltMusic();
		}

		draw_indicator(renderer);
		draw_compass(renderer, font, rwy[sel_rwy].or_local);
		write_freq(renderer, sel_freq/10.0, font);
		name_markers( renderer, font);
		draw_beacons(renderer, im_on, mm_on, om_on, &b_on);
		draw_state(renderer,  !in_loc, !in_gs, font);
		draw_CDI(renderer, x_sum_pt, y_sum_pt);
		running = botao(renderer, &sel_freq);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	Mix_FreeMusic(audio_outer);
	Mix_FreeMusic(audio_middle);
	Mix_FreeMusic(audio_inner);
	Mix_CloseAudio();
	SDL_DestroyWindow(window);
	SDL_Quit();
	TTF_Quit();
	pthread_cancel(tid);
	return 0;
}

