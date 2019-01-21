# include "ils.h"
# include "reception_thread.h"

// Global variables
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int ready = 0;
struct position_gps position;

int main(int argc, char** argv){

	int running=1;
	unsigned int starting_tick;
	SDL_Event event;
	SDL_Renderer* renderer=NULL;
	SDL_Window* window;

	int num_rwys=0, sel_rwy, b_on=0;
	int bandeira=0, in_loc=0, in_gs=0, im_on=0, mm_on=0, om_on=0;
	double sel_freq=109.5;
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

	SDL_Init ( SDL_INIT_VIDEO );
	window = SDL_CreateWindow("Indicador ILS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w_width, w_height, SDL_WINDOW_OPENGL );
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	draw_indicator(renderer);

	import_info_runways(argv[1], rwy, &num_rwys);
	runway_coordinates_to_ecef(rwy, num_rwys);

	while(running)
	{
		bandeira=0; in_loc=0; in_gs=0; om_on=0; mm_on=0; im_on=0;

		detect_sel_runway(rwy, num_rwys, sel_freq, &sel_rwy);
		if(sel_rwy!=-1)
		{
			//rececao da posição;
			pthread_mutex_lock(&m);
			if(ready){
				p=position;
				ready=0;
			}
			pthread_mutex_unlock(&m);

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

		draw_CDI(renderer, x_sum_pt, y_sum_pt);
		acender_beacons(renderer, im_on, mm_on, om_on, &b_on);

		SDL_RenderPresent(renderer);
		starting_tick = SDL_GetTicks();
		if ((1000/fps)>SDL_GetTicks()-starting_tick)
			SDL_Delay(1000/fps-(SDL_GetTicks()-starting_tick));

		quit(&running);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	pthread_cancel(tid);
	return 0;
}

/******************************************************************************************************************************************/
void import_info_runways(char* file, struct runway* rwy, int* num_rwys)
{
	FILE* f;
	const int buf_size=70;
	char buffer[buf_size];

	int i, or, lat_deg, lat_min, lon_deg, lon_min;
	double lat_sec, lon_sec, elev, gs, im, mm, om, fr;
	char lat_char, lon_char;


	f=fopen(file, "r");

	while(fgets(buffer,buf_size,f)&&(*num_rwys<=max_num_rwys))
	{
		for(i=0; buffer[i]!=';' ;i++);
		sscanf(buffer+i+1,"RWY%d;%dº%d\'%lf\"%c;%dº%d\'%lf\"%c;%lf;%lf;%lf;%lf;%lf;%lf;\n",
					&or, &lat_deg, &lat_min, &lat_sec, &lat_char, &lon_deg, &lon_min,
					&lon_sec, &lon_char, &elev, &gs, &im, &mm, &om, &fr);

		rwy[*num_rwys].orientacao = or*10.0*DEG_to_RAD;
		rwy[*num_rwys].latitude_thr =(lat_char=='N'?1:-1)*(lat_deg+lat_min/60.0+lat_sec/3600.0)*DEG_to_RAD;
		rwy[*num_rwys].longitude_thr=(lon_char=='E'?1:-1)*(lon_deg+lon_min/60.0+lon_sec/3600.0)*DEG_to_RAD;
		rwy[*num_rwys].altitude_thr = elev;
		rwy[*num_rwys].gs = gs * DEG_to_RAD;
		rwy[*num_rwys].im = im * NM_to_m;
		rwy[*num_rwys].mm = mm * NM_to_m;
		rwy[*num_rwys].om = om * NM_to_m;
		rwy[*num_rwys].freq = fr;

		*num_rwys=*num_rwys+1;
	}

	fclose(f);
}

/******************************************************************************************************************************************/
void runway_coordinates_to_ecef(struct runway* rwy, int num_rwys)
{
	const int WGS84_a = 6378137.0;
	const double WGS84_f = 0.0033528107;

	int i;
	double N;


	for(i=0; i<num_rwys; i++)
	{
		N = WGS84_a/(sqrt(1-WGS84_f*(2-WGS84_f)*pow(sin(rwy[i].latitude_thr),2)));

		rwy[i].x_ecef_thr = (rwy[i].altitude_thr + N) * cos(rwy[i].latitude_thr) * cos(rwy[i].longitude_thr);
		rwy[i].y_ecef_thr = (rwy[i].altitude_thr + N) * cos(rwy[i].latitude_thr) * sin(rwy[i].longitude_thr);
		rwy[i].z_ecef_thr = (rwy[i].altitude_thr + (N * pow((1-WGS84_f),2))) * sin(rwy[i].latitude_thr);
	}
}

/******************************************************************************************************************************************/
void detect_sel_runway(struct runway* rwy, int num_rwys, double sel_freq, int* sel_rwy)
{
	int i;

	for(i=0; i<num_rwys; i++)
	{
		if (rwy[i].freq == sel_freq){
			*sel_rwy = i;
			return;		    }
	}
	*sel_rwy=-1;
}

/******************************************************************************************************************************************/
void coordenadas_gps_to_ecef(struct position_gps p, struct position* p_ecef)
{
	const int WGS84_a = 6378137;
	const double WGS84_f = 0.0033528107;

	double N;
	N = WGS84_a/(sqrt(1-WGS84_f*(2-WGS84_f)*pow(sin(p.latitude),2)));

	(*p_ecef).x = (p.altitude + N) * cos(p.latitude) * cos(p.longitude);
	(*p_ecef).y = (p.altitude + N) * cos(p.latitude) * sin(p.longitude);
	(*p_ecef).z = (p.altitude + (N * pow((1-WGS84_f),2))) * sin(p.latitude);
}

/******************************************************************************************************************************************/
void coordenadas_ecef_to_enu(struct runway* rwy, int sel_rwy, struct position p_ecef, struct position* p_enu)
{
	int i=sel_rwy;
	double diff_x, diff_y, diff_z;


	diff_x = p_ecef.x - rwy[i].x_ecef_thr;
	diff_y = p_ecef.y - rwy[i].y_ecef_thr;
	diff_z = p_ecef.z - rwy[i].z_ecef_thr;

	(*p_enu).x = -sin(rwy[i].longitude_thr)*diff_x + cos(rwy[i].longitude_thr)*diff_y;

	(*p_enu).y = -sin(rwy[i].latitude_thr)*cos(rwy[i].longitude_thr)*diff_x +
		     -sin(rwy[i].latitude_thr)*sin(rwy[i].longitude_thr)*diff_y +
		      cos(rwy[i].latitude_thr)*diff_z;

	(*p_enu).z = cos(rwy[i].latitude_thr)*cos(rwy[i].longitude_thr)*diff_x +
		     cos(rwy[i].latitude_thr)*sin(rwy[i].longitude_thr)*diff_y +
		     sin(rwy[i].latitude_thr)*diff_z;
}

/******************************************************************************************************************************************/
void distance_to_runway(struct position p_enu, double* dist_rwy)
{
	*dist_rwy = sqrt(pow(p_enu.x,2)+pow(p_enu.y,2)+pow(p_enu.z,2));
}

/******************************************************************************************************************************************/
void in_localizer(struct position p_enu, struct runway* rwy, int sel_rwy, int* in_loc, double* loc_ang)
{
	double alpha;
	double h_dist;

	if (p_enu.x==0){
		if (p_enu.y>=0)
			alpha=PI/2;
		else
			alpha=PI;
	}
	else{
		alpha=atan(p_enu.y/p_enu.x);
		if (p_enu.x<0)
			alpha=alpha+PI;
		normalizar_angulo(&alpha);
	}

	h_dist=sqrt(pow(p_enu.x,2)+pow(p_enu.y,2));
	if (h_dist<18*NM_to_m){
		if(rwy[sel_rwy].orientacao<35*DEG_to_RAD){
			if (alpha>=325*DEG_to_RAD)
				alpha=alpha-360*DEG_to_RAD;}
		*loc_ang=alpha-rwy[sel_rwy].orientacao;
		if (*loc_ang>=-10*DEG_to_RAD && *loc_ang<=10*DEG_to_RAD){
			*in_loc=1;
			return;   }
	}
	if (h_dist<10*NM_to_m){
		if(rwy[sel_rwy].orientacao>325*DEG_to_RAD){
			if (alpha<=35*DEG_to_RAD)
				alpha=alpha+360*DEG_to_RAD;}
		*loc_ang=alpha-rwy[sel_rwy].orientacao;
		if (*loc_ang>=-35*DEG_to_RAD && *loc_ang<=35*DEG_to_RAD)
			*in_loc=1;
	}

}

/******************************************************************************************************************************************/
void normalizar_angulo(double* alpha)
{
	if (*alpha<0)
		*alpha=*alpha+2*PI;
	if (*alpha>2*PI)
		*alpha=*alpha-2*PI;
}

/******************************************************************************************************************************************/
void in_glide_slope(struct position p_enu, struct runway* rwy, int sel_rwy, double dist_rwy, int* in_gs, double* gs_ang)
{
	double gamma;

	gamma=asin(p_enu.z/dist_rwy);
	if (dist_rwy<=10*NM_to_m){
		if (gamma>=rwy[sel_rwy].gs-1.65*DEG_to_RAD && gamma<=rwy[sel_rwy].gs+2.25*DEG_to_RAD){
			*in_gs=1;
			*gs_ang=gamma;
		}
	}
}

/******************************************************************************************************************************************/
void movimento_ponteiro_localizer(double loc_ang, double* x_sum_pt)
{
	if (loc_ang>=2.5*DEG_to_RAD){
		*x_sum_pt=200/6*5;
		return;}
	if (loc_ang<=-2.5*DEG_to_RAD){
		*x_sum_pt=-200/6*5;
		return;}
	if ((loc_ang>-2.5*DEG_to_RAD)&&(loc_ang<2.5*DEG_to_RAD))
		*x_sum_pt=200/6*5/2.5*loc_ang*RAD_to_DEG;
}

/******************************************************************************************************************************************/
void movimento_ponteiro_glide_slope(double gs_ang, double* y_sum_pt)
{
	if (gs_ang>=3.7*DEG_to_RAD){
		*y_sum_pt=200/6*5;
		return;}
	if (gs_ang<=2.3*DEG_to_RAD){
		*y_sum_pt=-200/6*5;
		return;}
	if ((gs_ang>2.3*DEG_to_RAD)&&(gs_ang<3.7*DEG_to_RAD))
		*y_sum_pt=(gs_ang*RAD_to_DEG-3.7)*(-200/6*5)/(-0.7)+200/6*5;
}

/******************************************************************************************************************************************/
void in_markers(struct position p_enu, struct runway* rwy, int sel_rwy, double loc_ang, int* im_on, int* mm_on, int* om_on)
{
	double h_dist;

	if ((loc_ang>-3.75*DEG_to_RAD)&&(loc_ang<3.75*DEG_to_RAD)){
		h_dist=sqrt(pow(p_enu.x,2)+pow(p_enu.y,2));
		if ((h_dist<rwy[sel_rwy].om+300)&&(h_dist>rwy[sel_rwy].om-300)){
			if (p_enu.z<1200){
				*om_on=1;
				return;  }
		}
		if ((h_dist<rwy[sel_rwy].mm+150)&&(h_dist>rwy[sel_rwy].om-150)){
			if (p_enu.z<600){
				*mm_on=1;
				return; }
		}
		if ((h_dist<rwy[sel_rwy].im+75)&&(h_dist>rwy[sel_rwy].om-75)){
			if (p_enu.z<300){
				*im_on=1;
				return; }
		}
	}
}

/******************************************************************************************************************************************/
void draw_indicator(SDL_Renderer* renderer)
{
	int r, i;
	float x0=w_height/2;
	float y0=x0;

	r=w_height/2-50;

	SDL_Rect flag_nav={x0+r/6, y0-4*r/5, r/6, 2*r/6};
	SDL_Rect flag_gs={x0-4*r/5, y0-7*r/24, 2*r/6, r/6};


	SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);					//fundo
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	draw_circle(renderer, x0, y0, r+6, 0);				//circulo indicador
	SDL_SetRenderDrawColor(renderer, 250, 250, 250, SDL_ALPHA_OPAQUE);
	draw_circle(renderer, x0, y0, r+4, r);				//contorno
	SDL_SetRenderDrawColor(renderer, 250, 250, 250, SDL_ALPHA_OPAQUE);
	draw_circle(renderer, x0, y0, r/6, r/8);			//circ branco
	for (i=2; i<6; i++){
		draw_circle(renderer, x0+i*r/6, y0, r/24,0);
		draw_circle(renderer, x0-i*r/6, y0, r/24,0);
		draw_circle(renderer, x0, y0+i*r/6, r/24,0);
		draw_circle(renderer, x0, y0-i*r/6, r/24,0);
	}

	SDL_SetRenderDrawColor(renderer, 153, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &flag_nav);
	SDL_RenderFillRect(renderer, &flag_gs);

	draw_markerlights(renderer);
}

/******************************************************************************************************************************************/
void draw_circle(SDL_Renderer* renderer, float x0, float y0, float raioext, float raioint)
{
	float r=raioint;
	float teta;
	float x, y;
	while (r<=raioext) {
		teta=0;
		while (teta<(2*PI)) {
			x = x0+r*cos(teta);
			y = y0+r*sin(teta);
			SDL_RenderDrawPoint(renderer, x, y);
			teta=teta+PI/(r*8);
		}
		r++;
	}
}

/******************************************************************************************************************************************/
void draw_markerlights(SDL_Renderer* renderer){

	int i;

	float x=w_height+(w_width-w_height)/3;
	float r=w_height/12;
	float y=w_height-1.5*r;


	SDL_Rect freq={x-2*r, r/2, (w_width-w_height)*0.8, 2*r};

	SDL_SetRenderDrawColor(renderer, 160, 160, 160, SDL_ALPHA_OPAQUE);
	draw_circle(renderer, x, y, r, 0);
	SDL_SetRenderDrawColor(renderer, 102, 51, 0, SDL_ALPHA_OPAQUE);
	draw_circle(renderer, x, y-3*r, r, 0);
	SDL_SetRenderDrawColor(renderer, 0, 51, 102, SDL_ALPHA_OPAQUE);
	draw_circle(renderer, x, y-6*r, r, 0);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &freq);
	for (i=0; i<3; i++){
		draw_circle(renderer, x, y-r*3*i, r+1, r);
	}
}

/******************************************************************************************************************************************/
void draw_CDI(SDL_Renderer* renderer, double x_sum_pt, double y_sum_pt)
{
	float x0=w_height/2;
	float y0=x0;

	int raio=w_height/2-50;
	float espessura=raio/24;

	SDL_Rect hor={x0-raio/2, y0+y_sum_pt-espessura/2, raio, espessura};
	SDL_Rect ver={x0+x_sum_pt-espessura/2, y0-raio/2, espessura, raio};

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &hor);
	SDL_RenderFillRect(renderer, &ver);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &hor);
	SDL_RenderDrawRect(renderer, &ver);
}

/******************************************************************************************************************************************/
void acender_beacons(SDL_Renderer* renderer, int im_on, int mm_on, int om_on, int* b_on)
{
	float x=w_height+(w_width-w_height)/3;
	float r=w_height/12;
	float y=w_height-1.5*r;

	*b_on=*b_on+1;

	if (*b_on==fps*2)
		*b_on=0;

	if (om_on==1) {
		if(*b_on==fps)
			*b_on=0;
		if(*b_on<=fps/2)
			SDL_SetRenderDrawColor(renderer, 0, 128, 255, SDL_ALPHA_OPAQUE);
		else
			SDL_SetRenderDrawColor(renderer, 0, 51, 102, SDL_ALPHA_OPAQUE);
		draw_circle(renderer, x, y-6*r, r, 0); }

	else if(mm_on==1) {
		if(*b_on==fps/2)
			*b_on=0;
		if(*b_on<=fps/4)
			SDL_SetRenderDrawColor(renderer, 255, 153, 51, SDL_ALPHA_OPAQUE);
		else
			SDL_SetRenderDrawColor(renderer, 102, 51, 0, SDL_ALPHA_OPAQUE);
		draw_circle(renderer, x, y-3*r, r, 0); }

	else if(im_on==1) {
		if(*b_on==fps/4)
			*b_on=0;
		if(*b_on<=fps/8)
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		else
			SDL_SetRenderDrawColor(renderer, 160, 160, 160, SDL_ALPHA_OPAQUE);
		draw_circle(renderer, x, y, r, 0); }
}

/******************************************************************************************************************************************/
void quit(int* running)
{
	SDL_Event event;

	while(SDL_PollEvent(&event))
	{
		if ( event.type == SDL_QUIT ) {
			*running = 0;
			break;		      }
	}
}
