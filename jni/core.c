#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android_native_app_glue.h>
#include "defs.h"

int core(struct state *state){
	if(state->showmenu){
		if(!menu_main(state))return false;
		state->showmenu=false;
	}
	if((state->pausebuttonstate=buttonprocess(state,&state->pausebutton))==BUTTON_ACTIVATE||state->back){
		state->back=false;
		if(!menu_pause(state))return false;
	}
	else if(state->pausebuttonstate!=BUTTON_PRESS&&state->speed>=GAMESPEED){
		if(state->pointer[0].active)state->player.xv=state->pointer[0].x/20.0f;
		else zerof(&state->player.xv,0.01f);
		state->player.base.rot=state->player.xv*1.5f;
	}
	if(state->speed<GAMESPEED){
		state->speed+=0.0036f;
		if(state->speed>GAMESPEED)state->speed=GAMESPEED;
		targetf(&state->player.base.rot,((GAMESPEED+0.001f-state->speed)/1.76f),PI2);
		targetf(&state->player.base.x,((GAMESPEED-state->speed)/4.0f),-PLAYER_WIDTH/2.0f);
	}
	targetf(&state->background.y,state->speed,-24.0f);
	state->player.base.x+=state->player.xv;
	targetf(&state->player.base.y,0.005f,-6.0f);
	if(state->player.base.x+(PLAYER_WIDTH/2.0f)>state->rect.right){
		state->player.base.x=state->rect.left-(PLAYER_WIDTH/2.0f);
	}
	else if(state->player.base.x+(PLAYER_WIDTH/2.0f)<state->rect.left){
		state->player.base.x=state->rect.right-(PLAYER_WIDTH/2.0f);
	}
	if(state->scorecheck!=dgst2((unsigned)state->score))newobstacle(state);
	state->score+=0.0166f;
	state->scorecheck=dgst2((unsigned)state->score);
	state->speed+=0.00005f;
	if(state->speed>MAXGAMESPEED)state->speed=MAXGAMESPEED;
	
	if(onein(15))newobstacle(state);
	for(struct obstacle *obstacle=state->obstaclelist,*prevobstacle=NULL;obstacle!=NULL;){
		if(smallcollide(&state->player,obstacle)){
			if(state->vibrationenabled)vibratedevice(&state->jni_info,VIBRATE_LENGTH);
			if(!menu_end(state))return false;
			reset(state);
			if(state->showmenu)return core(state);
			break;
		}
		obstacle->base.y-=state->speed;
		if(obstacle->base.y+obstacle->base.h<state->rect.top){
			obstacle=deleteobstacle(state,obstacle,prevobstacle);
			continue;
		}
		prevobstacle=obstacle;
		obstacle=obstacle->next;
	}
	return true;
}

void render(struct state *state){
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BACKGROUND].object);
	glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	draw(state,&state->background);
	
	int tex=0;
	for(struct obstacle *obstacle=state->obstaclelist;obstacle!=NULL;obstacle=obstacle->next){
		if(tex!=obstacle->type+TID_OBSTACLE1){
			tex=obstacle->type+TID_OBSTACLE1;
			glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_OBSTACLE1+obstacle->type].object);
		}
		draw(state,&obstacle->base);
	}
	
	glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_PLAYER].object);
	if(!onein(3)){
		float temp=state->player.base.x;
		if(state->speed>=GAMESPEED)state->player.base.x+=randomint(-1,1)/100.0f;
		draw(state,&state->player.base);
		state->player.base.x=temp;
	}
	else draw(state,&state->player.base);
	if(state->player.base.x<state->rect.left){
		struct base player={state->rect.right-(state->rect.left-state->player.base.x),state->player.base.y,PLAYER_WIDTH,PLAYER_HEIGHT,0.0f};
		if(!onein(3)){
			float temp=state->player.base.x;
			if(state->speed>=GAMESPEED)player.x+=randomint(-1,1)/100.0f;
			draw(state,&player);
		}
		else draw(state,&player);
	}
	else if(state->player.base.x+PLAYER_WIDTH>state->rect.right){
		struct base player={state->rect.left-(state->rect.right-state->player.base.x),state->player.base.y,PLAYER_WIDTH,PLAYER_HEIGHT,0.0f};
		if(!onein(3)){
			float temp=state->player.base.x;
			if(state->speed>=GAMESPEED)player.x+=randomint(-1,1)/100.0f;
			draw(state,&player);
		}
		else draw(state,&player);
	}
	
	glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTONSMALL].object);
	if(state->pausebuttonstate==BUTTON_PRESS){
		float y=state->pausebutton.base.y;
		state->pausebutton.base.y+=0.2;
		draw(state,&state->pausebutton.base);
		state->pausebutton.base.y=y;
	}
	else draw(state,&state->pausebutton.base);
	
	glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_PAUSEICON].object);
	if(state->pausebuttonstate==BUTTON_PRESS){
		float y=state->pauseicon.y;
		state->pauseicon.y+=0.2f;
		draw(state,&state->pauseicon);
		state->pauseicon.y=y;
	}
	else draw(state,&state->pauseicon);
	
	glUniform4f(state->uniform.rgba,0.0f,0.0f,0.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
	char highscoremsg[15];
	sprintf(highscoremsg,"HS: %u\n%u",(unsigned)state->highscore,(unsigned)state->score);
	drawtextcentered(state->font.main,0.0f,-7.8f,highscoremsg);
}

void init(struct state *state){
	if(!readdata(state)){
		state->night=false;
		state->soundsenabled=true;
		state->vibrationenabled=true;
		state->showtutorial=true;
	}
	else state->showtutorial=false;
	if(!readhighscore(state)){
		state->highscore=0;
	}
	state->showmenu=true;
	state->back=false;
	memset(state->pointer,0,sizeof(struct crosshair)*2);
	state->rect.left=-4.5f;
	state->rect.right=4.5f;
	state->rect.bottom=8.0f;
	state->rect.top=-8.0f;
	state->background.x=state->rect.left;
	state->background.y=state->rect.top;
	state->background.w=state->rect.right*2.0f;
	state->background.h=state->rect.bottom*2.0f;
	state->background.rot=0.0f;
	state->player.base.w=PLAYER_WIDTH;
	state->player.base.h=PLAYER_HEIGHT;
	state->pausebutton.base.w=BUTTONSMALL_SIZE;
	state->pausebutton.base.h=BUTTONSMALL_SIZE;
	state->pausebutton.base.x=state->rect.right-BUTTONSMALL_SIZE-0.2f;
	state->pausebutton.base.y=state->rect.top+0.2f;
	state->pausebutton.base.rot=0.0f;
	state->pauseicon.x=state->pausebutton.base.x;
	state->pauseicon.y=state->pausebutton.base.y;
	state->pauseicon.w=1.5f;
	state->pauseicon.h=1.5f;
	state->pauseicon.rot=0.0f;
	state->obstaclelist=NULL;
}
void reset(struct state *state){
	state->score=0.0f;
	state->scorecheck=dgst2((unsigned)state->score);
	state->speed=0.0f;
	state->background.y=state->rect.top;
	state->player.base.rot=PI;
	state->player.base.x=1.1f;
	state->player.base.y=-3.7f;
	state->player.xv=0.0f;
	state->pausebuttonstate=0;
	state->pausebutton.active=false;
	for(struct obstacle *obstacle=state->obstaclelist;obstacle!=NULL;obstacle=deleteobstacle(state,obstacle,NULL));
	state->obstaclelist=NULL;
}
