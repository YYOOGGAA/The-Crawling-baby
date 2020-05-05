#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 800;
SDL_Window *window= NULL;
SDL_Renderer *ren = NULL;
SDL_Texture *tex=NULL;
TTF_Font *font=NULL;
Mix_Music *gMusic=NULL;
Mix_Chunk *gcountdown=NULL;
Mix_Chunk *gbabycry=NULL;
Mix_Music *gGameover=NULL;
Mix_Chunk *gameover=NULL;

bool quit=false;

const int START=0;
const int MENU=1;
const int INSTRUCTION=2;
const int COUNTDOWN=3;
const int PLAYING=4;
const int GAMEOVER=5;
int ScreenState=START;
int cnt=3;

const int PLAY=0;
const int HowToPlay=1;
int MenuSelect=PLAY;

const int RESTART=0;
const int QUIT=1;
const int menu=2;
int GameOverSelect=RESTART;

const int BLUE=0;
const int WHITE=1;
const int BLACK=2;
const int RED=3;


class LTexture
{
public:
    LTexture();
    ~LTexture();
    bool loadFromFile( std::string path );
    void free();
    void setColor( Uint8 red, Uint8 green, Uint8 blue );
    void setBlendMode( SDL_BlendMode blending );
    void setAlpha( Uint8 alpha );
    void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );
    int getWidth();
    int getHeight();

private:
    SDL_Texture* mTexture;
    int mWidth;
    int mHeight;
};
const int BL_WALKING_ANIMATION_FRAMES = 12;
SDL_Rect gBLSpriteClips[ BL_WALKING_ANIMATION_FRAMES ];
LTexture gBLSpriteSheetTexture;
//Walking animation
const int BR_WALKING_ANIMATION_FRAMES = 12;
SDL_Rect gBRSpriteClips[ BR_WALKING_ANIMATION_FRAMES ];
LTexture gBRSpriteSheetTexture;
const int WL_WALKING_ANIMATION_FRAMES = 12;
SDL_Rect gWLSpriteClips[ WL_WALKING_ANIMATION_FRAMES ];
LTexture gWLSpriteSheetTexture;
const int WR_WALKING_ANIMATION_FRAMES = 12;
SDL_Rect gWRSpriteClips[ WR_WALKING_ANIMATION_FRAMES ];
LTexture gWRSpriteSheetTexture;
bool loadMedia();
LTexture* currentTexture = NULL;
int frame1 = 0;
int frame2 = 0;
const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );
SDL_Rect* currentClip1 = &gWRSpriteClips[ 0 ];
SDL_Rect* currentClip2 = &gBLSpriteClips[ 0 ];


void loadpngwithrec(std::string path,SDL_Rect Viewport)
{
    SDL_Surface *png = IMG_Load(path.c_str());
    tex = SDL_CreateTextureFromSurface(ren, png);
    SDL_FreeSurface(png);
    SDL_RenderCopy(ren,tex,NULL,&Viewport);
    SDL_DestroyTexture(tex);
    tex=NULL;
}

void loadtextatcenter(std::string fontpath,int textsize,SDL_Color textcolor,std::string content,int x,int y)
{
    font=TTF_OpenFont(fontpath.c_str(),textsize);
    SDL_Surface* textsurface=TTF_RenderText_Solid(font,content.c_str(),textcolor);
    tex=SDL_CreateTextureFromSurface(ren,textsurface);
    SDL_FreeSurface(textsurface);
    TTF_CloseFont(font);
    SDL_Rect textrec;
    SDL_QueryTexture(tex,NULL,NULL,&textrec.w,&textrec.h);
    textrec.x=x-textrec.w/2;
    textrec.y=y-textrec.h/2;
    SDL_RenderCopy(ren,tex,NULL,&textrec);
    SDL_DestroyTexture(tex);
    tex=NULL;
}

class item
{
private:
    int category;
    bool create;
    SDL_Rect ItemRec;
    static int ItemVy;
public:
    item()
    {
        category=rand()%4;
        create=false;
        ItemRec={SCREEN_WIDTH,SCREEN_HEIGHT,60,60};
    }
    static void SetItemVy(int vy)
    {
        ItemVy=vy;
    }
    void SetCreate(bool y)
    {
        create=y;
    }
    bool GetCreate() const
    {
        return create;
    }
    void CreateItem()
    {
        ItemRec.x=rand()%540+20;
        ItemRec.y=825;
    }
    void MoveItem()
    {
        ItemRec.y-=ItemVy;
        if(ItemRec.y+ItemRec.h<=0)create=false;
    }
    void loadItem()
    {
        loadpngwithrec("image/blacktowel.png",ItemRec);
    }
    int  BabyGetItem(SDL_Rect player)
    {
        //from left top || from right top
        if((player.x+player.w>=ItemRec.x && player.x+player.w<=ItemRec.x+ItemRec.w && ItemRec.y<=player.y+player.h && ItemRec.y>=player.y)
           ||(player.x<=ItemRec.x+ItemRec.w && player.x>=ItemRec.x && ItemRec.y<=player.y+player.h && ItemRec.y>=player.y))
        {
            create=false;
            return category;
        }
        else return -1;
    }
};
int item::ItemVy=0;

class floor
{
private:
    const static int HoleW=101;
    const static int HoleH=10;
    int state;
    SDL_Rect hole[2];
    static int FloorVy;
public:
    SDL_Rect stair;
    static int GetFloorVy()
    {
        return FloorVy;
    }
    static void SetFloorVy(int v)
    {
        FloorVy=v;
    }
    int GetHoleLeftX(int i) const
    {
        return hole[i].x;
    }
    int GetHoleRightX(int i) const
    {
        return hole[i].x+hole[i].w;
    }
    void SetHoleRec()
    {
        switch(state)
        {
        case 0:
            hole[0]={520,stair.y-HoleH+100,HoleW,HoleH};
            hole[1]={640,stair.y-HoleH+100,HoleW,HoleH};
            break;
        case 1:
            hole[0]={20,stair.y-HoleH+100,HoleW,HoleH};
            hole[1]={640,stair.y-HoleH+100,HoleW,HoleH};
            break;
        case 2:
            hole[0]={150,stair.y-HoleH+100,HoleW,HoleH};
            hole[1]={390,stair.y-HoleH+100,HoleW,HoleH};
            break;
        case 3:
            hole[0]={240,stair.y-HoleH+100,HoleW,HoleH};
            hole[1]={490,stair.y-HoleH+100,HoleW,HoleH};
            break;
        case 4:
            hole[0]={50,stair.y-HoleH+100,HoleW,HoleH};
            hole[1]={345,stair.y-HoleH+100,HoleW,HoleH};
            break;
        case 5:
            hole[0]={340,stair.y-HoleH+100,HoleW,HoleH};
            hole[1]={640,stair.y-HoleH+100,HoleW,HoleH};
            break;
        case 6:
            hole[0]={200,stair.y-HoleH+100,HoleW,HoleH};
            hole[1]={640,stair.y-HoleH+100,HoleW,HoleH};
            break;
        case 7:
            hole[0]={270,stair.y-HoleH+100,HoleW,HoleH};
            hole[1]={640,stair.y-HoleH+100,HoleW,HoleH};
            break;
        case 8:
            hole[0]={100,stair.y-HoleH+100,HoleW,HoleH};
            hole[1]={440,stair.y-HoleH+100,HoleW,HoleH};
            break;
        }
    }
    void DrawHole()
    {
        SDL_SetRenderDrawColor( ren, 0x00, 0x00, 0xFF, 0xFF );
        SDL_RenderDrawRect( ren, &hole[0] );
        SDL_RenderDrawRect( ren, &hole[1] );
    }
    bool GenerateItem(Uint32 t)
    {
        if((t/1000)%8==0&&t/1000!=0)return true;
        return false;
    }
    void SetRandState()
    {
        state=rand()%9;
    }
    void loadfloor()
    {
        switch(state)
        {
        case 0:
            loadpngwithrec("image/stair0.png",stair);
            break;
        case 1:
            loadpngwithrec("image/stair1.png",stair);
            break;
        case 2:
            loadpngwithrec("image/stair2.png",stair);
            break;
        case 3:
            loadpngwithrec("image/stair3.png",stair);
            break;
        case 4:
            loadpngwithrec("image/stair4.png",stair);
            break;
        case 5:
            loadpngwithrec("image/stair5.png",stair);
            break;
        case 6:
            loadpngwithrec("image/stair6.png",stair);
            break;
        case 7:
            loadpngwithrec("image/stair7.png",stair);
            break;
        case 8:
            loadpngwithrec("image/stair8.png",stair);
            break;
        default:
            loadpngwithrec("image/stair8.png",stair);
            break;
        }
    }
};
int floor::FloorVy=0;

class baby
{
private:
    const static int BabyWidth=80;
    const static int BabyHeight=80;
    int vx,vy;
    int Speedx,Speedy;
    int StairNum;
    bool dead;
    int score;
public:
    SDL_Rect babyrec;
    baby(int x,int i,int sx,int sy)
    {
        Speedx=sx;
        vy=sy;
        vx=0;
        StairNum=i;
        babyrec.x=x;
        babyrec.y=(i+1)*100-90;
        babyrec.w=BabyWidth;
        babyrec.h=BabyHeight;
        dead=false;
        score=0;
    }
    void ReBorn(int x,int i,int sx,int sy)
    {
        Speedx=sx;
        vy=sy;
        vx=0;
        StairNum=i;
        babyrec.x=x;
        babyrec.y=(i+1)*100-90;
        babyrec.w=BabyWidth;
        babyrec.h=BabyHeight;
        dead=false;
        score=0;
    }
    void SetVY(int y)
    {
        vy=y;
    }
    int GetBabyStairNum() const
    {
        return StairNum;
    }
    void loadBaby(int playNum)
    {
        switch(playNum)
        {
        case 1:
            loadpngwithrec("image/rightw1.png",babyrec);
            break;
        case 2:
            loadpngwithrec("image/rightb1.png",babyrec);
            break;
        }
    }
    bool GetBabyState() const
    {
        return dead;
    }
    void MoveBaby(floor nowstair,floor nextstair)
    {
        babyrec.x+=vx;
        if(babyrec.x<=0)babyrec.x=0;
        if(babyrec.x>=SCREEN_WIDTH-BabyWidth)babyrec.x=SCREEN_WIDTH-BabyWidth;
        for(int i=0;i<2;i++)
        {
            if(babyrec.x>=nowstair.GetHoleLeftX(i)&&babyrec.x+babyrec.w<=nowstair.GetHoleRightX(i))
            {
                vy=-floor::GetFloorVy()*2;
            }
        }
        babyrec.y-=vy;
        if(babyrec.y>=SCREEN_HEIGHT-babyrec.h)babyrec.y=SCREEN_HEIGHT-babyrec.h;
        if(babyrec.y+babyrec.h<=0)
        {
            dead=true;
            gbabycry=Mix_LoadWAV("sound/babycry.mp3");
            Mix_PlayChannel( -1, gbabycry, 0 );
        }
        if(babyrec.y<610)
        {
            if(babyrec.y+babyrec.h>=nextstair.stair.y+90)
            {
                //std::cout<<nowstair.stair.y<<std::endl;
                //std::cout<<"top land"<<std::endl;
                babyrec.y=nextstair.stair.y+10;
                vy=floor::GetFloorVy();
                StairNum++;
                StairNum%=9;
                score++;
            }
        }
        else
        {
            if(babyrec.y+babyrec.h>=nowstair.stair.y+190)
            {
                //std::cout<<nowstair.stair.y<<std::endl;
                //std::cout<<"top land"<<std::endl;
                babyrec.y=nowstair.stair.y+110;
                vy=floor::GetFloorVy();
                StairNum++;
                StairNum%=9;
                score++;
            }
        }
    }
    void ShowScore(int playerNum)
    {
        std::stringstream ScoreText;
        ScoreText.str( "" );
        ScoreText<<score;
        SDL_Color ScoreColor={128,0,128,255};
        switch(playerNum)
        {
        case 1:
            loadtextatcenter("TTF/BROADWAY.ttf",30,ScoreColor,"1P's SCORE:",100,40);
            loadtextatcenter("TTF/PRISTINA.ttf",35,ScoreColor,ScoreText.str().c_str(),100,80);
            break;
        case 2:
            loadtextatcenter("TTF/BROADWAY.ttf",30,ScoreColor,"2P's SCORE:",540,40);
            loadtextatcenter("TTF/PRISTINA.ttf",35,ScoreColor,ScoreText.str().c_str(),540,80);
            break;
        }
    }
    void ShowDead(int playerNum)
    {
        SDL_Color MSG_Color={255,0,0,255};
        SDL_Rect X={55,15,100,65};
        switch(playerNum)
        {
        case 1:
            loadpngwithrec("image/x.png",X);
            loadtextatcenter("TTF/BROADWAY.ttf",35,MSG_Color,"DEAD",100,115);
            break;
        case 2:
            X.x=495;
            loadpngwithrec("image/x.png",X);
            loadtextatcenter("TTF/BROADWAY.ttf",35,MSG_Color,"DEAD",540,115);
            break;
        }
    }
    void handleEvent(SDL_Event& e,int playerNum)
    {
        switch(playerNum)
        {
        case 1:
            //If a key was pressed
            if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
            {
                //Adjust the velocity
                switch( e.key.keysym.sym )
                {
                    case SDLK_a:
                        vx -= Speedx;
                        break;
                    case SDLK_d:
                        vx += Speedx;
                        break;
                }
            }
            //If a key was released
            else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
            {
                //Adjust the velocity
                switch( e.key.keysym.sym )
                {
                    case SDLK_a:
                        vx += Speedx;
                        break;
                    case SDLK_d:
                        vx -= Speedx;
                        break;
                }
            }
            break;
        case 2:
            //If a key was pressed
            if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
            {
                //Adjust the velocity
                switch( e.key.keysym.sym )
                {
                    case SDLK_LEFT:
                        vx -= Speedx;
                        break;
                    case SDLK_RIGHT:
                        vx += Speedx;
                        break;
                }
            }
            //If a key was released
            else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
            {
                //Adjust the velocity
                switch( e.key.keysym.sym )
                {
                    case SDLK_LEFT:
                        vx += Speedx;
                        break;
                    case SDLK_RIGHT:
                        vx -= Speedx;
                        break;
                }
            }
            break;
        }
    }
};

void MenuHandleEvent(SDL_Event& e)
{
    if(e.type==SDL_KEYDOWN && e.key.repeat == 0)
    {
        switch(e.key.keysym.sym)
        {
        case SDLK_UP:
            MenuSelect--;
            MenuSelect+=2;
            MenuSelect%=2;
            break;
        case SDLK_DOWN:
            MenuSelect++;
            MenuSelect%=2;
            break;
        case SDLK_RETURN:
           if(MenuSelect==PLAY)
            {
                Mix_FadeOutMusic(1500);
                ScreenState=COUNTDOWN;
                cnt=3;
            }
            else
            {
                ScreenState=INSTRUCTION;
            }
            break;
        }
    }
}

void GameOverHandleEvent(SDL_Event& e)
{
    if(e.type==SDL_KEYDOWN && e.key.repeat == 0)
    {
        switch(e.key.keysym.sym)
        {
        case SDLK_UP:
            GameOverSelect--;
            GameOverSelect+=3;
            GameOverSelect%=3;
            break;
        case SDLK_DOWN:
            GameOverSelect++;
            GameOverSelect%=3;
            break;
        case SDLK_RETURN:
            if(GameOverSelect==RESTART)
            {
                Mix_FadeOutMusic(500);
                ScreenState=COUNTDOWN;
                cnt=3;
                gMusic=Mix_LoadMUS("sound/backgroundsound.mp3");
                Mix_PlayMusic(gMusic,-1);
            }
            else if(GameOverSelect==QUIT)
            {
                quit=true;
                break;
            }
            else
            {
                Mix_FadeOutMusic(500);
                ScreenState=MENU;
                gMusic=Mix_LoadMUS("sound/backgroundsound.mp3");
                Mix_FadeInMusic(gMusic,-1,500);
            }
            break;
        }
    }
}

int main(int argc,char* argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);
    window=SDL_CreateWindow("Crawling Baby",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
    ren= SDL_CreateRenderer(window, -1, 0);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    SDL_Surface* icon=IMG_Load("image/leftw1.png");
    //std::cout<<icon<<std::endl;
    SDL_SetWindowIcon(window,icon);
    SDL_FreeSurface(icon);
    Uint32 StartTime=0,PlayTime=0;
    SDL_Rect background={0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
    SDL_Color textcolor={0,0,0,255};
    floor STAIR[9];
    item towel[9];
    SDL_Rect selecter={160,120,70,70};
    if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 ) return -1;
    gMusic=Mix_LoadMUS("sound/backgroundsound.mp3");
    Mix_FadeInMusic(gMusic,-1,500);
    for(int i=0;i<9;i++)
    {
        STAIR[i].stair={0,100*i,SCREEN_WIDTH,100};
        STAIR[i].SetRandState();
        STAIR[i].SetHoleRec();
    }
    floor::SetFloorVy(5);
    item::SetItemVy(floor::GetFloorVy());
    baby p1(160,7,11,floor::GetFloorVy());
    baby p2(400,7,11,floor::GetFloorVy());
    SDL_Event e;
    loadMedia();
	while( !quit )
    {
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                quit = true;
                break;
            }
            switch(ScreenState)
            {
            case START:
                if(e.type==SDL_KEYDOWN&&e.key.repeat==0)
                {
                    ScreenState=MENU;
                }
                break;
            case MENU:
                MenuHandleEvent(e);
                break;
            case INSTRUCTION:
                if(e.type==SDL_KEYDOWN&&e.key.repeat==0)
                {
                    Mix_FadeOutMusic(1500);
                    ScreenState=COUNTDOWN;
                    cnt=3;
                }
                break;
            case PLAYING:
                p1.handleEvent(e,1);
                p2.handleEvent(e,2);
                break;
            case GAMEOVER:
                GameOverHandleEvent(e);
                break;
            }
        }
        SDL_SetRenderDrawColor( ren, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear(ren);
        switch(ScreenState)
        {
        default:
        case START:
            loadpngwithrec("image/baby_start.png",background);
            break;
        case MENU:
            loadpngwithrec("image/menu.jpg",background);
            loadtextatcenter("TTF/BROADWAY.TTF",70,textcolor,"Play",SCREEN_WIDTH/2,170);
            loadtextatcenter("TTF/BROADWAY.TTF",70,textcolor,"How to Play",SCREEN_WIDTH/2,240);
            if(MenuSelect==PLAY)
            {
                selecter.x=160;
                selecter.y=140;
                loadpngwithrec("image/bottle.png",selecter);
           }
            else
            {
                selecter.x=40;
                selecter.y=210;
                loadpngwithrec("image/bottle.png",selecter);
            }
            break;
        case INSTRUCTION:
            loadpngwithrec("image/howtoplay.png",background);
            break;
        case COUNTDOWN:
            //after start
            gMusic=Mix_LoadMUS("sound/backgroundsound.mp3");
            Mix_FadeInMusic(gMusic,-1,4000);
            PlayTime=0;
            if(cnt==3)
            {
                for(int i=0;i<9;i++)
                {
                    STAIR[i].stair={0,100*i,SCREEN_WIDTH,100};
                    STAIR[i].SetRandState();
                    STAIR[i].SetHoleRec();
                }
                floor::SetFloorVy(5);
                p1.ReBorn(160,7,10,5);
                p2.ReBorn(400,7,10,5);
            }
            loadpngwithrec("image/background2.jpg",background);
            for(int i=0;i<9;i++)
            {
                STAIR[i].loadfloor();
                //STAIR[i].DrawHole();
            }
            p1.loadBaby(1);
            p2.loadBaby(2);
            p1.ShowScore(1);
            p2.ShowScore(2);
            switch(cnt)
            {
            case 3:
                gcountdown=Mix_LoadWAV("sound/countdown.mp3");
                Mix_PlayChannel( -1, gcountdown, 0 );
                Mix_VolumeChunk(gcountdown,128);
                loadtextatcenter("TTF/lazy.ttf",140,textcolor,"3",SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
                cnt--;
                break;
            case 2:
                loadtextatcenter("TTF/lazy.ttf",140,textcolor,"2",SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
                cnt--;
                break;
            case 1:
                loadtextatcenter("TTF/lazy.ttf",140,textcolor,"1",SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
                cnt--;
                break;
            case 0:
                loadtextatcenter("TTF/lazy.ttf",140,textcolor,"START!",SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
                cnt--;
                break;
            }
            break;
        case PLAYING:
            //after start
            PlayTime=(SDL_GetTicks()-StartTime);
            loadpngwithrec("image/background1.jpg",background);
            for(int i=0;i<9;i++)
            {
                STAIR[i].loadfloor();
                //STAIR[i].DrawHole();
                if(towel[i].GetCreate())towel[i].loadItem();
            }
            if(p1.GetBabyState())p1.ShowDead(1);
            if(p2.GetBabyState())p2.ShowDead(2);
            p1.ShowScore(1);
            p2.ShowScore(2);
            for(int i=0;i<9;i++)
            {
                if(STAIR[i].stair.y>(-1)*STAIR[i].stair.h)
                {
                    STAIR[i].stair.y-=STAIR[i].GetFloorVy();
                    STAIR[i].SetHoleRec();
                }
                else
                {
                    STAIR[i].stair.y=795;
                    STAIR[i].SetRandState();
                    STAIR[i].SetHoleRec();
                    towel[i].SetCreate(STAIR[i].GenerateItem(PlayTime));
                    if(towel[i].GetCreate())towel[i].CreateItem();
                }
                if(towel[i].GetCreate())
                {
                    towel[i].MoveItem();
                    if(!p1.GetBabyState())towel[i].BabyGetItem(p1.babyrec);
                    if(!p2.GetBabyState())towel[i].BabyGetItem(p2.babyrec);
                }
            }
            if(!p1.GetBabyState())p1.MoveBaby(STAIR[p1.GetBabyStairNum()],STAIR[(p1.GetBabyStairNum()+1)%9]);
            if(!p2.GetBabyState())p2.MoveBaby(STAIR[p2.GetBabyStairNum()],STAIR[(p2.GetBabyStairNum()+1)%9]);
            if(p1.GetBabyState()&&p2.GetBabyState())
            {
                ScreenState=GAMEOVER;
                Mix_FadeOutMusic(500);
                gGameover=Mix_LoadMUS("sound/gameovermus.mp3");
                Mix_FadeInMusic(gGameover,-1,1000);
                gameover=Mix_LoadWAV("sound/gameover.mp3");
                Mix_PlayChannel( -1, gameover, 0 );
            }
            //lazy foo
            currentKeyStates = SDL_GetKeyboardState( NULL );
            if( currentKeyStates[ SDL_SCANCODE_A ] )
            {
                currentTexture = &gWLSpriteSheetTexture;
                currentClip1 = &gWLSpriteClips[ frame1 / 12 ];
                gWLSpriteSheetTexture.render( ( p1.babyrec.x ) , ( p1.babyrec.y ) , currentClip1 );
                SDL_RenderPresent( ren );
                ++frame1;
                if( frame1 / 12 >= WL_WALKING_ANIMATION_FRAMES )
                {
                    frame1 = 0;
                }
            }
            else if( currentKeyStates[ SDL_SCANCODE_D ] )
            {
                currentTexture = &gWRSpriteSheetTexture;
                currentClip1 = &gWRSpriteClips[ frame1 / 12 ];
                gWRSpriteSheetTexture.render( ( p1.babyrec.x ) , ( p1.babyrec.y ) , currentClip1 );
                SDL_RenderPresent( ren );
                ++frame1;
                if( frame1 / 12 >= WR_WALKING_ANIMATION_FRAMES )
                {
                    frame1 = 0;
                }
            }
            else
            {
                gWRSpriteSheetTexture.render( ( p1.babyrec.x ) , ( p1.babyrec.y ) , currentClip1 );
                SDL_RenderPresent( ren );
            }

            currentKeyStates = SDL_GetKeyboardState( NULL );
            if( currentKeyStates[ SDL_SCANCODE_LEFT ] )
            {
                currentTexture = &gBLSpriteSheetTexture;
                currentClip2 = &gBLSpriteClips[ frame2 / 12 ];
                gBLSpriteSheetTexture.render( ( p2.babyrec.x ) , ( p2.babyrec.y ) , currentClip2 );
                SDL_RenderPresent( ren );
                ++frame2;
                if( frame2 / 12 >= BL_WALKING_ANIMATION_FRAMES )
                {
                    frame2 = 0;
                }
            }
            else if( currentKeyStates[ SDL_SCANCODE_RIGHT ] )
            {
                currentTexture = &gBRSpriteSheetTexture;
                currentClip2 = &gBRSpriteClips[ frame2 / 12 ];
                gBRSpriteSheetTexture.render( ( p2.babyrec.x ) , ( p2.babyrec.y ) , currentClip2 );
                SDL_RenderPresent( ren );
                ++frame2;
                if( frame2 / 12 >= BR_WALKING_ANIMATION_FRAMES )
                {
                    frame2 = 0;
                }
            }
            else
            {
                gBRSpriteSheetTexture.render( ( p2.babyrec.x ) , ( p2.babyrec.y ) , currentClip2 );
                SDL_RenderPresent( ren );
            }
            break;
        case GAMEOVER:
            loadpngwithrec("image/gameover.png",background);
            loadtextatcenter("TTF/BROADWAY.TTF",70,textcolor,"RESTART",SCREEN_WIDTH/2,380);
            loadtextatcenter("TTF/BROADWAY.TTF",70,textcolor,"QUIT",SCREEN_WIDTH/2,450);
            loadtextatcenter("TTF/BROADWAY.TTF",70,textcolor,"MENU",SCREEN_WIDTH/2,520);
            if(GameOverSelect==RESTART)
            {
                selecter.x=80;
                selecter.y=345;
                loadpngwithrec("image/bottle.png",selecter);
            }
            else if(GameOverSelect==QUIT)
            {
                selecter.x=162;
                selecter.y=415;
                loadpngwithrec("image/bottle.png",selecter);
            }
            else
            {
                selecter.x=150;
                selecter.y=485;
                loadpngwithrec("image/bottle.png",selecter);
            }
            break;
        }
        std::stringstream timetext;
        timetext.str("");
        timetext<<PlayTime/1000.f;
        //loadtextatcenter("TTF/lazy.ttf",40,textcolor,timetext.str().c_str(),SCREEN_WIDTH/2,700);
        SDL_RenderPresent(ren);
        if(ScreenState==COUNTDOWN)
        {
            SDL_Delay(1000);
            if(cnt<0)
            {
                ScreenState=PLAYING;
                StartTime=SDL_GetTicks();
            }
        }
    }
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    Mix_FreeMusic(gMusic);
    Mix_FreeChunk(gcountdown);
    Mix_FreeChunk(gbabycry);
    Mix_FreeMusic(gGameover);
    Mix_FreeChunk(gameover);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    Mix_CloseAudio();
    return 0;
}


LTexture::LTexture()
{
    //Initialize
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

LTexture::~LTexture()
{
    //Deallocate
    free();
}

bool LTexture::loadFromFile( std::string path )
{
    //Get rid of preexisting texture
    free();

    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Color key image
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( ren, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    //Return success
    mTexture = newTexture;
    return mTexture != NULL;
}

void LTexture::free()
{
    //Free texture if it exists
    if( mTexture != NULL )
    {
        SDL_DestroyTexture( mTexture );
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
    //Modulate texture rgb
    SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
    //Set blending function
    SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{
    //Modulate texture alpha
    SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { x, y+5, mWidth, mHeight };

    //Set clip rendering dimensions
    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    //Render to screen
    SDL_RenderCopyEx( ren, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}


bool loadMedia()
{
    //Loading success flag
    bool success = true;

    //Load left texture
    if( !gBLSpriteSheetTexture.loadFromFile( "image/bleft.png" ) )
    {
        printf( "Failed to load walking animation texture!\n" );
        success = false;
    }
    else
    {
        //Set sprite clips
        gBLSpriteClips[ 0 ].x =   0;
        gBLSpriteClips[ 0 ].y =   0;
        gBLSpriteClips[ 0 ].w =  80;
        gBLSpriteClips[ 0 ].h =  80;

        gBLSpriteClips[ 1 ].x =  80;
        gBLSpriteClips[ 1 ].y =   0;
        gBLSpriteClips[ 1 ].w =  80;
        gBLSpriteClips[ 1 ].h =  80;

        gBLSpriteClips[ 2 ].x = 160;
        gBLSpriteClips[ 2 ].y =   0;
        gBLSpriteClips[ 2 ].w =  80;
        gBLSpriteClips[ 2 ].h =  80;

        gBLSpriteClips[ 3 ].x = 240;
        gBLSpriteClips[ 3 ].y =   0;
        gBLSpriteClips[ 3 ].w =  80;
        gBLSpriteClips[ 3 ].h =  80;

        gBLSpriteClips[ 4 ].x = 320;
        gBLSpriteClips[ 4 ].y =   0;
        gBLSpriteClips[ 4 ].w =  80;
        gBLSpriteClips[ 4 ].h =  80;

        gBLSpriteClips[ 5 ].x = 400;
        gBLSpriteClips[ 5 ].y =   0;
        gBLSpriteClips[ 5 ].w =  80;
        gBLSpriteClips[ 5 ].h =  80;

        gBLSpriteClips[ 6 ].x = 480;
        gBLSpriteClips[ 6 ].y =   0;
        gBLSpriteClips[ 6 ].w =  80;
        gBLSpriteClips[ 6 ].h =  80;

        gBLSpriteClips[ 7 ].x = 400;
        gBLSpriteClips[ 7 ].y =   0;
        gBLSpriteClips[ 7 ].w =  80;
        gBLSpriteClips[ 7 ].h =  80;

        gBLSpriteClips[ 8 ].x = 320;
        gBLSpriteClips[ 8 ].y =   0;
        gBLSpriteClips[ 8 ].w =  80;
        gBLSpriteClips[ 8 ].h =  80;

        gBLSpriteClips[ 9 ].x = 240;
        gBLSpriteClips[ 9 ].y =   0;
        gBLSpriteClips[ 9 ].w =  80;
        gBLSpriteClips[ 9 ].h =  80;

        gBLSpriteClips[ 10 ].x = 160;
        gBLSpriteClips[ 10 ].y =   0;
        gBLSpriteClips[ 10 ].w =  80;
        gBLSpriteClips[ 10 ].h =  80;

        gBLSpriteClips[ 11 ].x =  80;
        gBLSpriteClips[ 11 ].y =   0;
        gBLSpriteClips[ 11 ].w =  80;
        gBLSpriteClips[ 11 ].h =  80;
    }

    //Load right texture
    if( !gBRSpriteSheetTexture.loadFromFile( "image/bright.png" ) )
    {
        printf( "Failed to load walking animation texture!\n" );
        success = false;
    }
    else
    {
        //Set sprite clips
        gBRSpriteClips[ 0 ].x =   0;
        gBRSpriteClips[ 0 ].y =   0;
        gBRSpriteClips[ 0 ].w =  80;
        gBRSpriteClips[ 0 ].h =  80;

        gBRSpriteClips[ 1 ].x =  80;
        gBRSpriteClips[ 1 ].y =   0;
        gBRSpriteClips[ 1 ].w =  80;
        gBRSpriteClips[ 1 ].h =  80;

        gBRSpriteClips[ 2 ].x = 160;
        gBRSpriteClips[ 2 ].y =   0;
        gBRSpriteClips[ 2 ].w =  80;
        gBRSpriteClips[ 2 ].h =  80;

        gBRSpriteClips[ 3 ].x = 240;
        gBRSpriteClips[ 3 ].y =   0;
        gBRSpriteClips[ 3 ].w =  80;
        gBRSpriteClips[ 3 ].h =  80;

        gBRSpriteClips[ 4 ].x = 320;
        gBRSpriteClips[ 4 ].y =   0;
        gBRSpriteClips[ 4 ].w =  80;
        gBRSpriteClips[ 4 ].h =  80;

        gBRSpriteClips[ 5 ].x = 400;
        gBRSpriteClips[ 5 ].y =   0;
        gBRSpriteClips[ 5 ].w =  80;
        gBRSpriteClips[ 5 ].h =  80;

        gBRSpriteClips[ 6 ].x = 480;
        gBRSpriteClips[ 6 ].y =   0;
        gBRSpriteClips[ 6 ].w =  80;
        gBRSpriteClips[ 6 ].h =  80;

        gBRSpriteClips[ 7 ].x = 400;
        gBRSpriteClips[ 7 ].y =   0;
        gBRSpriteClips[ 7 ].w =  80;
        gBRSpriteClips[ 7 ].h =  80;

        gBRSpriteClips[ 8 ].x = 320;
        gBRSpriteClips[ 8 ].y =   0;
        gBRSpriteClips[ 8 ].w =  80;
        gBRSpriteClips[ 8 ].h =  80;

        gBRSpriteClips[ 9 ].x = 240;
        gBRSpriteClips[ 9 ].y =   0;
        gBRSpriteClips[ 9 ].w =  80;
        gBRSpriteClips[ 9 ].h =  80;

        gBRSpriteClips[ 10 ].x = 160;
        gBRSpriteClips[ 10 ].y =   0;
        gBRSpriteClips[ 10 ].w =  80;
        gBRSpriteClips[ 10 ].h =  80;

        gBRSpriteClips[ 11 ].x =  80;
        gBRSpriteClips[ 11 ].y =   0;
        gBRSpriteClips[ 11 ].w =  80;
        gBRSpriteClips[ 11 ].h =  80;
    }

    if( !gWLSpriteSheetTexture.loadFromFile( "image/wleft.png" ) )
    {
        printf( "Failed to load walking animation texture!\n" );
        success = false;
    }
    else
    {
        //Set sprite clips
        gWLSpriteClips[ 0 ].x =   0;
        gWLSpriteClips[ 0 ].y =   0;
        gWLSpriteClips[ 0 ].w =  80;
        gWLSpriteClips[ 0 ].h =  80;

        gWLSpriteClips[ 1 ].x =  80;
        gWLSpriteClips[ 1 ].y =   0;
        gWLSpriteClips[ 1 ].w =  80;
        gWLSpriteClips[ 1 ].h =  80;

        gWLSpriteClips[ 2 ].x = 160;
        gWLSpriteClips[ 2 ].y =   0;
        gWLSpriteClips[ 2 ].w =  80;
        gWLSpriteClips[ 2 ].h =  80;

        gWLSpriteClips[ 3 ].x = 240;
        gWLSpriteClips[ 3 ].y =   0;
        gWLSpriteClips[ 3 ].w =  80;
        gWLSpriteClips[ 3 ].h =  80;

        gWLSpriteClips[ 4 ].x = 320;
        gWLSpriteClips[ 4 ].y =   0;
        gWLSpriteClips[ 4 ].w =  80;
        gWLSpriteClips[ 4 ].h =  80;

        gWLSpriteClips[ 5 ].x = 400;
        gWLSpriteClips[ 5 ].y =   0;
        gWLSpriteClips[ 5 ].w =  80;
        gWLSpriteClips[ 5 ].h =  80;

        gWLSpriteClips[ 6 ].x = 480;
        gWLSpriteClips[ 6 ].y =   0;
        gWLSpriteClips[ 6 ].w =  80;
        gWLSpriteClips[ 6 ].h =  80;

        gWLSpriteClips[ 7 ].x = 400;
        gWLSpriteClips[ 7 ].y =   0;
        gWLSpriteClips[ 7 ].w =  80;
        gWLSpriteClips[ 7 ].h =  80;

        gWLSpriteClips[ 8 ].x = 320;
        gWLSpriteClips[ 8 ].y =   0;
        gWLSpriteClips[ 8 ].w =  80;
        gWLSpriteClips[ 8 ].h =  80;

        gWLSpriteClips[ 9 ].x = 240;
        gWLSpriteClips[ 9 ].y =   0;
        gWLSpriteClips[ 9 ].w =  80;
        gWLSpriteClips[ 9 ].h =  80;

        gWLSpriteClips[ 10 ].x = 160;
        gWLSpriteClips[ 10 ].y =   0;
        gWLSpriteClips[ 10 ].w =  80;
        gWLSpriteClips[ 10 ].h =  80;

        gWLSpriteClips[ 11 ].x =  80;
        gWLSpriteClips[ 11 ].y =   0;
        gWLSpriteClips[ 11 ].w =  80;
        gWLSpriteClips[ 11 ].h =  80;
    }

    //Load right texture
    if( !gWRSpriteSheetTexture.loadFromFile( "image/wright.png" ) )
    {
        printf( "Failed to load walking animation texture!\n" );
        success = false;
    }
    else
    {
        //Set sprite clips
        gWRSpriteClips[ 0 ].x =   0;
        gWRSpriteClips[ 0 ].y =   0;
        gWRSpriteClips[ 0 ].w =  80;
        gWRSpriteClips[ 0 ].h =  80;

        gWRSpriteClips[ 1 ].x =  80;
        gWRSpriteClips[ 1 ].y =   0;
        gWRSpriteClips[ 1 ].w =  80;
        gWRSpriteClips[ 1 ].h =  80;

        gWRSpriteClips[ 2 ].x = 160;
        gWRSpriteClips[ 2 ].y =   0;
        gWRSpriteClips[ 2 ].w =  80;
        gWRSpriteClips[ 2 ].h =  80;

        gWRSpriteClips[ 3 ].x = 240;
        gWRSpriteClips[ 3 ].y =   0;
        gWRSpriteClips[ 3 ].w =  80;
        gWRSpriteClips[ 3 ].h =  80;

        gWRSpriteClips[ 4 ].x = 320;
        gWRSpriteClips[ 4 ].y =   0;
        gWRSpriteClips[ 4 ].w =  80;
        gWRSpriteClips[ 4 ].h =  80;

        gWRSpriteClips[ 5 ].x = 400;
        gWRSpriteClips[ 5 ].y =   0;
        gWRSpriteClips[ 5 ].w =  80;
        gWRSpriteClips[ 5 ].h =  80;

        gWRSpriteClips[ 6 ].x = 480;
        gWRSpriteClips[ 6 ].y =   0;
        gWRSpriteClips[ 6 ].w =  80;
        gWRSpriteClips[ 6 ].h =  80;

        gWRSpriteClips[ 7 ].x = 400;
        gWRSpriteClips[ 7 ].y =   0;
        gWRSpriteClips[ 7 ].w =  80;
        gWRSpriteClips[ 7 ].h =  80;

        gWRSpriteClips[ 8 ].x = 320;
        gWRSpriteClips[ 8 ].y =   0;
        gWRSpriteClips[ 8 ].w =  80;
        gWRSpriteClips[ 8 ].h =  80;

        gWRSpriteClips[ 9 ].x = 240;
        gWRSpriteClips[ 9 ].y =   0;
        gWRSpriteClips[ 9 ].w =  80;
        gWRSpriteClips[ 9 ].h =  80;

        gWRSpriteClips[ 10 ].x = 160;
        gWRSpriteClips[ 10 ].y =   0;
        gWRSpriteClips[ 10 ].w =  80;
        gWRSpriteClips[ 10 ].h =  80;

        gWRSpriteClips[ 11 ].x =  80;
        gWRSpriteClips[ 11 ].y =   0;
        gWRSpriteClips[ 11 ].w =  80;
        gWRSpriteClips[ 11 ].h =  80;
    }

    return success;
}
