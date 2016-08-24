#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android_native_app_glue.h>
#include "defs.h"

void newobstacle(struct state *state){
	int type=randomint(0,3);
	float w,h,x,y;
	switch(type){
		case 0:
			w=2.0f;
			h=1.5f;
			break;
		case 1:
			w=2.3f;
			h=2.875f;
			break;
		case 2:
			w=2.1f;
			h=2.75f;
			break;
		case 3:
			w=0.75f;
			h=3.5f;
			break;
	}
	if(onein(6)){
		if(onein(2))x=state->rect.left-0.1f;
		else x=state->rect.right-w+0.1f;
	}
	else x=randomint(state->rect.left*100.0f,(state->rect.right-w)*100.0f)/100.0f;
	y=state->rect.bottom+2.0f;
	struct obstacle collide_test={x,y,w,h}; // need temp obstacle 'object' to pass to 'largecollide'
	for(struct obstacle *obstacle2=state->scorecheck==dgst2((unsigned)state->score)?state->obstaclelist:(struct obstacle*)1;obstacle2!=NULL;obstacle2=obstacle2->next){
		if(largecollide(&collide_test,obstacle2))return; // cancel obstacle creation
	}
	
	struct obstacle *obstacle=malloc(sizeof(struct obstacle));
	obstacle->type=type;
	obstacle->base.x=x;
	obstacle->base.y=y;
	obstacle->base.w=w;
	obstacle->base.h=h;
	obstacle->base.rot=0.0f;
	if(state->obstaclelist==NULL){
		state->obstaclelist=obstacle;
		obstacle->next=NULL;
	}
	else
	for(struct obstacle *obst=state->obstaclelist,*prevobst=NULL;obst!=NULL;obst=obst->next){
		if(obstacle->type<=obst->type){
			if(prevobst!=NULL){
				prevobst->next=obstacle;
				obstacle->next=obst;
			}
			else{
				state->obstaclelist=obstacle;
				obstacle->next=obst;
			}
			break;
		}
		else if(obst->next==NULL){
			obst->next=obstacle;
			obstacle->next=NULL;
			break;
		}
		prevobst=obst;
	}
}
struct obstacle *deleteobstacle(struct state *state,struct obstacle *obstacle,struct obstacle *prev){
	if(prev!=NULL)prev->next=obstacle->next;
	else state->obstaclelist=obstacle->next;
	void *temp=obstacle->next;
	free(obstacle);
	return temp;
}

int buttonprocess(struct state *state,struct button *button){
	if(state->pointer[0].x>button->base.x&&state->pointer[0].x<button->base.x+button->base.w&&
	state->pointer[0].y>button->base.y&&state->pointer[0].y<button->base.y+button->base.h&&state->pointer[0].active){
		button->active=true;
		return BUTTON_PRESS;
	}
	else if(state->pointer[0].x>button->base.x&&state->pointer[0].x<button->base.x+button->base.w&&
	state->pointer[0].y>button->base.y&&state->pointer[0].y<button->base.y+button->base.h&&!state->pointer[0].active&&button->active){
		button->active=false;
		return BUTTON_ACTIVATE;
	}
	button->active=false;
	return 0;
}
int buttondraw(struct state *state,struct button *button){
	int bstatus=buttonprocess(state,button);
	float y;
	switch(bstatus){
		case BUTTON_PRESS:
			y=button->base.y;
			button->base.y+=0.1f;
			draw(state,&button->base);
			button->base.y=y;
			return bstatus;
	}
	draw(state,&button->base);
	return bstatus;
}
void buttondrawtext(ftfont *font,struct button *button){
	float y=button->active?button->base.y+0.1f:button->base.y;
	drawtextcentered(font,button->base.x+(button->base.w/2.0f),y+(button->base.h/2.0f)-(font->fontsize/2.0f),button->label);
}
void buttondrawicon(struct state *state,struct base *icon,struct button *button){
	if(button->active){
		float y=icon->y;
		icon->y+=0.1f;
		draw(state,icon);
		icon->y=y;
	}
	else draw(state,icon);
}

int readdata(struct state *state){
	FILE *file=fopen(DATAPATH"d01","rb");
	if(!file)return false;
	fread(&state->night,sizeof(char),3,file);
	fclose(file);
	return true;
}
void savedata(struct state *state){
	FILE *file=fopen(DATAPATH"d01","wb");
	if(!file){
		logcat("Unable to save to d01");
		return;
	}
	fwrite(&state->night,sizeof(char),3,file);
	fclose(file);
}
int readhighscore(struct state *state){
	unsigned cs;
	FILE *file=fopen(DATAPATH"d02","rb");
	if(!file)return false;
	fread(&state->highscore,sizeof(unsigned),1,file);
	fread(&cs,sizeof(unsigned),1,file);
	fclose(file);
	if(cs!=dgst(state->highscore)){
		state->highscore=0;
		if(remove(DATAPATH"d02")){
			logcat("failed to remove file");
		}
		*(int*)0=0;
	}
	return true;
}
void savehighscore(struct state *state){
	int cs=dgst(state->highscore);
	FILE *file=fopen(DATAPATH"d02","wb");
	if(!file){
		logcat("Unable to save to d02");
		return;
	}
	fwrite(&state->highscore,sizeof(unsigned),1,file);
	fwrite(&cs,sizeof(unsigned),1,file);
	fclose(file);
}

int collide(struct base *a,struct base *b){
	return a->x+a->w>b->x&&a->x<b->x+b->w&&a->y+a->h>b->y&&a->y<b->y+b->h;
}
int largecollide(struct obstacle *a,struct obstacle *b){
	const float TOLERANCE=0.75f;
	return a->base.x+a->base.w+TOLERANCE>b->base.x-TOLERANCE&&a->base.x-TOLERANCE<b->base.x+b->base.w+TOLERANCE&&
	a->base.y+a->base.h+TOLERANCE*2.0f>b->base.y-TOLERANCE*2.0f&&a->base.y-TOLERANCE*2.0f<b->base.y+b->base.h+TOLERANCE*2.0f;
}
int smallcollide(struct player *p,struct obstacle *o){
	const float TOLERANCE=0.3f;
	return p->base.x+p->base.w-TOLERANCE>o->base.x&&p->base.x+TOLERANCE<o->base.x+o->base.w&&p->base.y+p->base.h>o->base.y&&p->base.y<o->base.y+o->base.h;
}

void draw(struct state *state,struct base *target){
	glUniform4f(state->uniform.texcoords,0.0f,1.0f,0.0f,1.0f);
	glUniform2f(state->uniform.vector,target->x,target->y);
	glUniform2f(state->uniform.size,target->w,target->h);
	glUniform1f(state->uniform.rot,target->rot);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}