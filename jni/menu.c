#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android_native_app_glue.h>
#include "defs.h"

extern const char *vertexshader,*fragmentshader,*nightfragmentshader;
const char *tutorialtext=
"Hold finger in middle\nof screen\n\nMove finger from side\nto side to move player\n\nUninstall game because\nyou suck";
int menu_main(struct state *state){
	struct button playbutton={{-BUTTON_WIDTH/2.0f,2.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Play",false};
	struct button aboutbutton={{-BUTTON_WIDTH/2.0f,4.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Aboot",false};
	struct button confbutton={{state->rect.left+0.2f,state->rect.bottom-BUTTONSMALL_SIZE-0.2f,BUTTONSMALL_SIZE,BUTTONSMALL_SIZE,0.0f},"Conf",false};
	struct button quitbutton={{state->rect.right-BUTTONSMALL_SIZE-0.2f,state->rect.bottom-BUTTONSMALL_SIZE-0.2f,BUTTONSMALL_SIZE,BUTTONSMALL_SIZE,0.0f},"Quit",false};
	struct base powericon={quitbutton.base.x,quitbutton.base.y,BUTTONSMALL_SIZE,BUTTONSMALL_SIZE,0.0f};
	struct base conficon={confbutton.base.x,confbutton.base.y,BUTTONSMALL_SIZE,BUTTONSMALL_SIZE,0.0f};
	char highscoremsg[15]="";
	sprintf(highscoremsg,"HS: %u",state->highscore);
	while(process(state->app)){
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BACKGROUND].object);
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		draw(state,&state->background);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_PLAYER].object);
		draw(state,&state->player.base);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
		if(buttondraw(state,&playbutton)==BUTTON_ACTIVATE){
			if(state->showtutorial){
				state->showtutorial=false;
				if(!menu_message(state,"Tutorial",tutorialtext))return false;
				savedata(state);
			}
			return true;
		}
		if(buttondraw(state,&aboutbutton)==BUTTON_ACTIVATE){
			if(!menu_message(state,"Aboot","FREEFALL\nProgrammed by Josh Winter\n\n"
			"Fonts: Colonna MT,\nCorbel Regular"))return false;
		}
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTONSMALL].object);
		if(buttondraw(state,&confbutton)==BUTTON_ACTIVATE){
			if(!menu_conf(state))return false;
		}
		if(buttondraw(state,&quitbutton)==BUTTON_ACTIVATE||state->back){
			ANativeActivity_finish(state->app->activity);
		}
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_POWERICON].object);
		buttondrawicon(state,&powericon,&quitbutton);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_CONFICON].object);
		buttondrawicon(state,&conficon,&confbutton);
		glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
		glUniform4f(state->uniform.rgba,0.0f,0.0f,0.0f,1.0f);
		buttondrawtext(state->font.header,&playbutton);
		buttondrawtext(state->font.header,&aboutbutton);
		drawtextcentered(state->font.header,0.0f,-6.0f,"FREEFALL");
		glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
		drawtextcentered(state->font.main,0.0f,6.75f,highscoremsg);
		eglSwapBuffers(state->display,state->surface);
	}
	return false;
}

int menu_conf(struct state *state){
	struct button nightbutton={{-BUTTON_WIDTH/2.0f,-1.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Night",false};
	struct button soundsbutton={{-BUTTON_WIDTH/2.0f,1.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Sounds",false};
	struct button vibrationbutton={{-BUTTON_WIDTH/2.0f,3.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Vibrate",false};
	struct button backbutton={{-BUTTON_WIDTH/2.0f,5.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Back",false};
	char message[90];
	int changed=false;
	while(process(state->app)){
		//glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BACKGROUND].object);
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		/*draw(state,&state->background);*/
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
		if(buttondraw(state,&nightbutton)==BUTTON_ACTIVATE){
			state->night=!state->night;
			glDeleteProgram(state->program);
			if(state->night){
				state->program=initshaders(vertexshader,nightfragmentshader);
				glClearColor(0.0f,0.0f,0.0f,1.0f);
			}
			else{
				state->program=initshaders(vertexshader,fragmentshader);
				glClearColor(1.0f,1.0f,1.0f,1.0f);
			}
			glUseProgram(state->program);
			state->uniform.vector=glGetUniformLocation(state->program,"vector");
			state->uniform.size=glGetUniformLocation(state->program,"size");
			state->uniform.texcoords=glGetUniformLocation(state->program,"texcoords");
			state->uniform.rot=glGetUniformLocation(state->program,"rot");
			state->uniform.rgba=glGetUniformLocation(state->program,"rgba");
			state->uniform.projection=glGetUniformLocation(state->program,"projection");
			float matrix[16];
			initortho(matrix,state->rect.left,state->rect.right,state->rect.bottom,state->rect.top,-1.0f,1.0f);
			glUniformMatrix4fv(state->uniform.projection,1,false,matrix);
			changed=true;
		}
		if(buttondraw(state,&soundsbutton)==BUTTON_ACTIVATE){
			state->soundsenabled=!state->soundsenabled;
			changed=true;
		}
		if(buttondraw(state,&vibrationbutton)==BUTTON_ACTIVATE){
			state->vibrationenabled=!state->vibrationenabled;
			changed=true;
		}
		if(buttondraw(state,&backbutton)==BUTTON_ACTIVATE||state->back){
			state->back=false;
			if(changed)savedata(state);
			return true;
		}
		glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
		glUniform4f(state->uniform.rgba,0.0f,0.0f,0.0f,1.0f);
		buttondrawtext(state->font.header,&nightbutton);
		buttondrawtext(state->font.header,&soundsbutton);
		buttondrawtext(state->font.header,&vibrationbutton);
		buttondrawtext(state->font.header,&backbutton);
		drawtextcentered(state->font.header,0.0f,-6.0f,"CONFIGURATION");
		glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
		sprintf(message,"Night mode is %s\nSounds are %s\nVibration is %s",
		state->night?"enabled":"disabled",state->soundsenabled?"enabled":"disabled",state->vibrationenabled?"enabled":"disabled");
		drawtextcentered(state->font.main,0.0f,-4.0f,message);
		eglSwapBuffers(state->display,state->surface);
	}
	return false;
}

int menu_pause(struct state *state){
	struct button confbutton={{-BUTTON_WIDTH/2.0f,1.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Conf.",false};
	struct button resetbutton={{-BUTTON_WIDTH/2.0f,-1.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Reset",false};
	struct button tutbutton={{-BUTTON_WIDTH/2.0f,3.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Tutorial",false};
	struct button backbutton={{-BUTTON_WIDTH/2.0f,-3.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Resume",false};
	struct button stopbutton={{-BUTTON_WIDTH/2.0f,5.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Stop",false};
	while(process(state->app)){
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		/*glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BACKGROUND].object);
		draw(state,&state->background);*/
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
		if(buttondraw(state,&confbutton)==BUTTON_ACTIVATE){
			if(!menu_conf(state))return false;
		}
		if(buttondraw(state,&tutbutton)==BUTTON_ACTIVATE){
			if(!menu_message(state,"Tutorial",tutorialtext))return false;
		}
		if(buttondraw(state,&backbutton)==BUTTON_ACTIVATE||state->back){
			state->back=false;
			return true;
		}
		if(buttondraw(state,&resetbutton)==BUTTON_ACTIVATE||state->back){
			reset(state);
			return true;
		}
		if(buttondraw(state,&stopbutton)==BUTTON_ACTIVATE){
			reset(state);
			state->showmenu=true;
			return core(state);
		}
		glUniform4f(state->uniform.rgba,0.0f,0.0f,0.0f,1.0f);
		glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
		drawtextcentered(state->font.header,0.0f,-6.0f,"PAUSED");
		buttondrawtext(state->font.header,&confbutton);
		buttondrawtext(state->font.header,&tutbutton);
		buttondrawtext(state->font.header,&backbutton);
		buttondrawtext(state->font.header,&resetbutton);
		buttondrawtext(state->font.header,&stopbutton);
		eglSwapBuffers(state->display,state->surface);
	}
	return false;
}

int menu_end(struct state *state){
	struct button againbutton={{-BUTTON_WIDTH/2.0f,3.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Again",false};
	struct button menubutton={{-BUTTON_WIDTH/2.0f,5.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"Menu",false};
	char msg[60],scoremsg[9];
	int highscore=state->highscore;
	sprintf(msg,"Score\n\n\nHigh score\n%u",state->highscore);
	sprintf(scoremsg,"%u",(unsigned)state->score);
	if((unsigned)state->score>state->highscore){
		state->highscore=(unsigned)state->score;
		savehighscore(state);
	}
	int showbuttons=false;
	const char *catchphrases[]={
		":(",
		"O, Fudge",
		"Noice",
		"MEDIOCRE",
		"Durn it"
	};
	int phrase=randomint(0,4);
	while(process(state->app)){
		if(!state->pointer[0].active)showbuttons=true;
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		if(showbuttons){
			glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
			if(buttondraw(state,&againbutton)==BUTTON_ACTIVATE||state->back){
				state->back=false;
				return true;
			}
			if(buttondraw(state,&menubutton)==BUTTON_ACTIVATE){
				state->showmenu=true;
				return true;
			}
			glUniform4f(state->uniform.rgba,0.0f,0.0f,0.0f,1.0f);
			glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
			buttondrawtext(state->font.header,&againbutton);
			buttondrawtext(state->font.header,&menubutton);
		}
		else{
			glUniform4f(state->uniform.rgba,0.0f,0.0f,0.0f,1.0f);
			glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
		}
		drawtextcentered(state->font.header,0.0f,-6.0f,catchphrases[phrase]);
		drawtextcentered(state->font.header,0.0f,-2.6f,scoremsg);
		glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
		drawtextcentered(state->font.main,0.0f,-3.0f,msg);
		if((unsigned)state->score>highscore){
			glUniform4f(state->uniform.rgba,1.0f,0.0f,0.0f,1.0f);
			drawtextcentered(state->font.main,0.0f,0.6f,"NEW HIGH SCORE!!");
		}
		eglSwapBuffers(state->display,state->surface);
	}
	return false;
}

int menu_message(struct state *state,const char *caption,const char *msg){
	struct button okbutton={{-BUTTON_WIDTH/2.0f,4.0f,BUTTON_WIDTH,BUTTON_HEIGHT,0.0f},"OK",false};
	while(process(state->app)){
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		/*glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BACKGROUND].object);
		draw(state,&state->background);*/
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
		if(buttondraw(state,&okbutton)==BUTTON_ACTIVATE||state->back){
			state->back=false;
			return true;
		}
		glUniform4f(state->uniform.rgba,0.0f,0.0f,0.0f,1.0f);
		glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
		buttondrawtext(state->font.header,&okbutton);
		drawtextcentered(state->font.header,0.0f,-7.0f,caption);
		glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
		drawtextcentered(state->font.main,0.0f,-4.0f,msg);
		eglSwapBuffers(state->display,state->surface);
	}
	return false;
}
	