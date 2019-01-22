#include "ils.h"

/******************************************************************************************************************************************/
void import_info_runways(char* file, struct runway* rwy, int* num_rwys)
{
	FILE* f;
	const int buf_size=100;
	char buffer[buf_size];

	int i, or_local, lat_deg, lat_min, lon_deg, lon_min;
	double or, lat_sec, lon_sec, elev, gs, im, mm, om, fr;
	char lat_char, lon_char;


	f=fopen(file, "r");
	if(f == NULL){
		fprintf(stderr,"Error opening file\n");
		exit(1);
	}

	while(fgets(buffer,buf_size,f)&&(*num_rwys<=max_num_rwys))
	{
		for(i=0; buffer[i]!=';' ;i++);
		sscanf(buffer+i+1,"RWY%d;%lf;%dº%d\'%lf\"%c;%dº%d\'%lf\"%c;%lf;%lf;%lf;%lf;%lf;%lf;\n",
					&or_local, &or, &lat_deg, &lat_min, &lat_sec, &lat_char, &lon_deg, &lon_min,
					&lon_sec, &lon_char, &elev, &gs, &im, &mm, &om, &fr);

		rwy[*num_rwys].or_local = or_local*10*DEG_to_RAD;
		rwy[*num_rwys].orientacao = or*DEG_to_RAD;
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
void detect_sel_runway(struct runway* rwy, int num_rwys, int sel_freq, int* sel_rwy)
{
	int i;
		
	for(i=0; i<num_rwys; i++)
	{
		if (rwy[i].freq == sel_freq/10.0){
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

	if (p_enu.y==0){
		if (p_enu.x>=0)
			alpha=PI/2;
		else
			alpha=PI;
	}
	else{
		alpha=atan(p_enu.x/p_enu.y);
		if (p_enu.y<0)
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
		if ((h_dist<rwy[sel_rwy].om+340)&&(h_dist>rwy[sel_rwy].om-340)){
			if (p_enu.z<1200){
				*om_on=1;
				return;  }
		}
		if ((h_dist<rwy[sel_rwy].mm+120)&&(h_dist>rwy[sel_rwy].mm-120)){
			if (p_enu.z<600){
				*mm_on=1;
				return; }
		}
		if ((h_dist<rwy[sel_rwy].im+70)&&(h_dist>rwy[sel_rwy].im-70)){
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

	/*SDL_Rect flag_nav={x0+r/6, y0-4*r/5, r/6, 2*r/6};
	SDL_Rect flag_gs={x0-4*r/5, y0-7*r/24, 2*r/6, r/6};*/


	SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);					//fundo
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	draw_circle(renderer, x0, y0, r+6+r/6, 0);				//circulo indicador
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

	/*SDL_SetRenderDrawColor(renderer, 153, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &flag_nav);
	SDL_RenderFillRect(renderer, &flag_gs);*/


	//CAROLINA ACRESCENTOU::::
	//draw_circle(renderer, x0, y0, );


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
void draw_beacons(SDL_Renderer* renderer, int im_on, int mm_on, int om_on, int* b_on)
{
	float x=w_height+(w_width-w_height)/3;
	float r=w_height/12;
	float y=w_height-1.5*r;
	
				
	SDL_SetRenderDrawColor(renderer, 160, 160, 160, SDL_ALPHA_OPAQUE);
	draw_circle(renderer, x, y, r, 0);
	SDL_SetRenderDrawColor(renderer, 102, 51, 0, SDL_ALPHA_OPAQUE);
	draw_circle(renderer, x, y-3*r, r, 0);
	SDL_SetRenderDrawColor(renderer, 0, 51, 102, SDL_ALPHA_OPAQUE);
	draw_circle(renderer, x, y-6*r, r, 0);
	
	*b_on=*b_on+1;

	if (*b_on==fps/2)
		*b_on=0;

	if (om_on==1) {
		if(*b_on==fps/2)
			*b_on=0;
		if(*b_on<=fps/4)
			SDL_SetRenderDrawColor(renderer, 0, 128, 255, SDL_ALPHA_OPAQUE);
		else
			SDL_SetRenderDrawColor(renderer, 0, 51, 102, SDL_ALPHA_OPAQUE);
		draw_circle(renderer, x, y-6*r, r, 0); }

	else if(mm_on==1) {
		if(*b_on==fps/4)
			*b_on=0;
		if(*b_on<=fps/8)
			SDL_SetRenderDrawColor(renderer, 255, 153, 51, SDL_ALPHA_OPAQUE);
		else
			SDL_SetRenderDrawColor(renderer, 102, 51, 0, SDL_ALPHA_OPAQUE);
		draw_circle(renderer, x, y-3*r, r, 0); }

	else if(im_on==1) {
		if(*b_on==fps/8)
			*b_on=0;
		if(*b_on<fps/15)
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		else
			SDL_SetRenderDrawColor(renderer, 160, 160, 160, SDL_ALPHA_OPAQUE);
		draw_circle(renderer, x, y, r, 0); }
}

/******************************************************************************************************************************************/
int quit(void)
{
	SDL_Event event;

	SDL_PollEvent(&event);

	return (event.type != SDL_QUIT);
}
/*********************************************************************************/
void draw_text(SDL_Renderer *renderer, int x, int y, float angle, int size, char* text, TTF_Font *font)
{

	int text_width;
	int text_height;
	SDL_Surface *surface;
	const SDL_Color textColor = {255, 255, 255, 0};
	SDL_Texture *texture;
	SDL_Rect rect;

	surface = TTF_RenderText_Solid(font, text, textColor);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	text_width = surface->w;
	text_height = surface->h;
	SDL_FreeSurface(surface);
	rect.x = x;
	rect.y = y;
	rect.w = text_width*size;
	rect.h = text_height*size;



	SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
	SDL_DestroyTexture(texture);

}

/******************************************************************************************************************************************/
void draw_compass(SDL_Renderer *renderer, TTF_Font *font, float course){

	int text_width;
    int text_height;
    SDL_Surface *surface;
    const SDL_Color textColor = {255, 255, 255, 0};
	SDL_Texture *texture;
	SDL_Rect rect;

	const int center_x=240;
	const int center_y=240;
	const int radius=220;

	const int angles[12] = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33};


	int i;
	char text[32];

	for(i=0; i<12; i++){
		sprintf(text, "%d", angles[i]);
		surface = TTF_RenderText_Solid(font, text, textColor);
	    texture = SDL_CreateTextureFromSurface(renderer, surface);
	    text_width = surface->w;
	    text_height = surface->h;
	    rect.w = text_width;
	    rect.h = text_height;
		rect.x = center_x + sin(-course + i*PI/6)*radius;
	    rect.y = center_y - cos(-course + i*PI/6)*radius;

		SDL_RenderCopyEx(renderer, texture, NULL, &rect, 30*i-course*RAD_to_DEG, NULL, SDL_FLIP_NONE);
		SDL_DestroyTexture(texture);
		SDL_FreeSurface(surface);
	}
}

/******************************************************************************************************************************************/
void write_freq(SDL_Renderer *renderer, float frequency, TTF_Font *font){

	int rectx = w_height/2+w_width/3;
	int recty = w_height/24;
	int rect_width = (w_width-w_height)*0.8;
	int rect_height = w_height/6;
	char f[10];

	sprintf(f, "%4.1f", frequency);
	//gcvt(frequency,5,f);
	SDL_Rect freq = {rectx, recty, rect_width, rect_height};

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &freq);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &freq);
	draw_text(renderer, rectx+rect_width/15, recty+rect_height/5, 0, 2, f, font);

}

/******************************************************************************************************************************************/
void draw_state(SDL_Renderer *renderer, int bandeira, TTF_Font *font){
	//Desenhar quadradinhos (tirar da funçao draw_indicator
	//float x=w_height/2;
	//float r=x-50;

	int xgs=w_height/6;
	int ygs=3*w_height/8;
	int wgs=w_height/6;
	int hgs=w_height/12;

	int xnav=13*w_height/24;


	SDL_Rect flag_nav={xnav, xgs,hgs, wgs};
	SDL_Rect flag_gs={xgs,ygs, wgs, hgs};

	SDL_SetRenderDrawColor(renderer, 153, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &flag_nav);
	SDL_RenderFillRect(renderer, &flag_gs);
	// int b=0;  // Testar com OFF
	//Escrever GS

	draw_text(renderer, xgs+wgs/3,ygs+hgs/5 , 0, 1, "GS", font);
	//Escrever OFF/NAV
	if (bandeira==0){
		draw_text(renderer, xnav+hgs/3,xgs , 0, 1, "N", font);
		draw_text(renderer, xnav+hgs/3, xgs+wgs/3, 0, 1, "A", font);
		draw_text(renderer, xnav+hgs/3, xgs+2*wgs/3, 0, 1, "V", font);
	}
	else {
		draw_text(renderer, xnav+hgs/3,xgs , 0, 1, "O", font);
		draw_text(renderer, xnav+hgs/3, xgs+wgs/3, 0, 1, "F", font);
		draw_text(renderer, xnav+hgs/3, xgs+2*wgs/3, 0, 1, "F", font);
	}

}

/******************************************************************************************************************************************/
void botao(SDL_Renderer* renderer, int* sel_freq){
	int x, y;
	int rectx1 = w_height/2+w_width/3;
	int recty1 = w_height/24;
	int rectx2 = (w_width-w_height)*0.8+rectx1;
	int recty2 = w_height/6+recty1;
	int larg = (w_width-w_height)*0.17;
	SDL_Event botao;
	SDL_Rect rect= {rectx2, recty1, larg, w_height/6};
	SDL_Rect mais= {rectx2, recty1, larg, w_height/12+1};
	SDL_Rect menos= {rectx2, recty1+w_height/12+1, larg, w_height/12};

	SDL_Rect maishor = {rectx2+larg/4, recty1+(recty2-recty1)/4-1, larg/2 ,2};
	SDL_Rect maisver = {rectx2+larg/2-1, recty1+(recty2-recty1)/4-larg/4, 2,larg/2};
	SDL_Rect menoshor = {rectx2+larg/4, recty2-(recty2-recty1)/4-1, larg/2 ,2};
	
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);

	
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &mais);
	SDL_RenderDrawRect(renderer, &menos);	
	SDL_RenderFillRect(renderer, &maishor);
	SDL_RenderFillRect(renderer, &menoshor);	
	SDL_RenderFillRect(renderer, &maisver);


	while (SDL_PollEvent(&botao)){
		if (botao.type==SDL_MOUSEBUTTONDOWN){
			SDL_GetMouseState(&x,&y);
			if( (x>=rectx2) && (x<=rectx2+larg) && (y<=recty1+w_height/12+1) && (y>=recty1) ){
				*sel_freq=*sel_freq+1;
			}
			else if( (x>=rectx2) && (x<=rectx2+larg) && (y<=recty2) && (y>=recty1+w_height/12+1) ){
				*sel_freq=*sel_freq-1;
			}
			return;
		}
	}
		
}

/******************************************************************************************************************************************/
void name_markers(SDL_Renderer* renderer, TTF_Font *font){
	
	float r=w_height/12;
	float x=w_height+(w_width-w_height)/3+1.2*r;
	float y=w_height-2*r;
	float x2 = w_height+(w_width-w_height)/3-1.5*r;
	float y2 = w_height-9*r;
	float width=4*r;
	float height=8.9*r;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

	SDL_Rect markers={x2,y2,width,height};
	SDL_RenderFillRect(renderer, &markers);

	draw_text(renderer, x+0.2*r, y+0.2*r, 0, 1, "IM", font);
	draw_text(renderer, x+0.2*r, y-2.8*r, 0, 1, "MM",font);
	draw_text(renderer, x+0.2*r, y-5.8*r, 0, 1, "OM",font);
	
}
