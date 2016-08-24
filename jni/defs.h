#include "glesutil.h"
#define VIBRATE_LENGTH 100
#define PI 3.14159f
#define PI2 6.28318f
#define GAMESPEED 0.205f
#define MAXGAMESPEED 0.3f
#define dgst(x) ((~x)/3)
#define dgst2(x) ((~x)/2)
#define DATAPATH "/data/data/joshwinter.freefall/files/"

#define TID_BACKGROUND 0
#define TID_BUTTON 1
#define TID_BUTTONSMALL 2
#define TID_POWERICON 3
#define TID_CONFICON 4
#define TID_PAUSEICON 5
#define TID_PLAYER 6
#define TID_OBSTACLE1 7

struct base{
	float x,y,w,h,rot;
};

#define BUTTON_ACTIVATE 1
#define BUTTON_PRESS 2
#define BUTTON_WIDTH 5.0f
#define BUTTON_HEIGHT 1.75f
#define BUTTONSMALL_SIZE 1.5f
struct button{
	struct base base;
	char *label;
	int active;
};

#define PLAYER_WIDTH 0.65f
#define PLAYER_HEIGHT 1.0f
struct player{
	struct base base;
	float xv;
};

struct obstacle{
	struct base base;
	int type;
	struct obstacle *next;
};

struct state{
	int running,back,showmenu,showtutorial,pausebuttonstate;
	unsigned highscore;
	char night,soundsenabled,vibrationenabled;
	float speed,score;
	EGLContext context;
	EGLSurface surface;
	EGLDisplay display;
	unsigned vao,vbo,program;
	struct button pausebutton;
	struct android_app *app;
	struct device device;
	struct crosshair pointer[2];
	struct jni_info jni_info;
	struct pack assets;
	unsigned scorecheck;
	struct apack aassets;
	struct{int vector,size,rot,texcoords,rgba,projection;}uniform;
	struct{ftfont *main,*header;}font;
	struct{float left,right,bottom,top;}rect;
	struct base background,pauseicon;
	struct player player;
	struct obstacle *obstaclelist;
};

int32_t inputproc(struct android_app*,AInputEvent*);
void cmdproc(struct android_app*,int32_t);
int process(struct android_app*);
void init_display(struct state*);
void term_display(struct state*);
void draw(struct state*,struct base*);
void init(struct state*);
void reset(struct state*);
int readdata(struct state*);
void savedata(struct state*);
int readhighscore(struct state*);
void savehighscore(struct state*);

int buttonprocess(struct state*,struct button*);
int buttondraw(struct state*,struct button*);
void buttondrawtext(ftfont*,struct button*);
void buttondrawicon(struct state*,struct base*,struct button*);
int core(struct state*);
void render(struct state*);
int menu_main(struct state*);
int menu_conf(struct state*);
int menu_pause(struct state*);
int menu_end(struct state*);
int menu_message(struct state*,const char*,const char*);
int collide(struct base*,struct base*);
int largecollide(struct obstacle*,struct obstacle*);
int smallcollide(struct player*,struct obstacle*);
void newobstacle(struct state*);
struct obstacle *deleteobstacle(struct state*,struct obstacle*,struct obstacle*);
