#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include"generare.h"
#include<fstream>
#include<SDL_mixer.h>
#include<cstdlib>

//dimensions of the level
const int LEVEL_WIDTH = 7168;
const int LEVEL_HEIGHT = 480;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

SDL_Rect wall[60];
int k,ke,lives=3;
bool isWin=false;

//music
Mix_Music *sMusic = NULL;

//sound effects
Mix_Chunk *sJump1 = NULL;
Mix_Chunk *sJump2=NULL;
Mix_Chunk *sDeath1 = NULL;
Mix_Chunk *sDeath2=NULL;
Mix_Chunk *sDeath3=NULL;
Mix_Chunk *sKill = NULL;
Mix_Chunk *sWin = NULL;
Mix_Chunk *sIntro=NULL;

//tetura elementelor
class Texture
{
    public:
		//iniializarea variabilelor proprii
		Texture();

		//dealoca memorie
		~Texture();

		//inacrca img de la sursa
		bool loadFromFile( std::string path );

		//dealoca textura
		void free();

		//functia de rendering
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//fct de a obt dimensiunile imagini
		int getWidth();
		int getHeight();

	private:
		//textura in sine
		SDL_Texture* mTexture;

		//dimensiunile imagini
		int mWidth;
		int mHeight;
};

class Sprite
{
    public:
		//sprite dimensions
		static const int SPR_WIDTH = 24;
		static const int SPR_HEIGHT = 30;

		//viteza
		static const int VEL = 5;

		//initializeaza variabilele
		Sprite();

		~Sprite();

		//interpreteaza eventurile
		void handleEvent( SDL_Event& e );

		//misca sprite-ul
		void move( SDL_Rect wall[]);

		//render pentru sprite
		void render( int camX, int camY );
		//saritura
		void jump(SDL_Rect wall[]);
		//cazatura
		void fall(SDL_Rect wall[]);

		void walk(SDL_Rect wall[]);

		void die();

		int getPosX();
		int getPosY();

		int mPosX,mVelX;

		bool isDead;
        //patratul de coliziune
		SDL_Rect mCollider;
    private:
		//pozitia
		int mPosY;

		//viteza pe axe
		int  mVelY;

		bool isJump, midAir;

		int jumpTimes;


};

class Enemy
{
    public:
        static const int EN_WIDTH = 32;
		static const int EN_HEIGHT = 32;

		//viteza
		static const int VEL = 2;

		//initializeaza variabilele
		Enemy()
		{}
		void pozitionare(int x,int y);
		//misca sprite-ul
		void move(SDL_Rect marioCollider);

		void die();
        SDL_Rect mCollider;
        SDL_Rect mDeathCollider;
		//render pentru sprite
		void render(int x);
		bool isDead;
    private:



        int mPosX,mPosY;

        int mVelX;

        int go;

};
Enemy enemy[20];
//initializeaza programul
bool init();

//incarca toate imaginile
bool loadMedia();

//inchide sdl si elibereaza memoria
void close();

//verifica coliziunea dintre 2 obiecte
bool checkCollision( SDL_Rect a, SDL_Rect b );

//fereastra in sine
SDL_Window* gWindow = NULL;

//obiectul de rendering
SDL_Renderer* gRenderer = NULL;

//texturile din joc
Texture gSprTexture,gEnemy;

Texture gBGTexture;

Texture::Texture()
{
	//initializeaza textura
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

Texture::~Texture()
{
	//dealoca
	free();
}

bool Texture::loadFromFile( std::string path )
{
	//scapa de textura preexistenta
	free();

	//textura finala
	SDL_Texture* newTexture = NULL;

	//incarca imaginea de la sursa
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//optimizeaza imaginea incarcata
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//dimensiuneile imagini
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//elimina suprafata
		SDL_FreeSurface( loadedSurface );
	}

	mTexture = newTexture;
	return mTexture != NULL;
}

void Texture::free()
{
	//elibereaza textura daca exista
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}


void Texture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//seteaza spatiu de rendering
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//randeaza pe ecran
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int Texture::getWidth()
{
	return mWidth;
}

int Texture::getHeight()
{
	return mHeight;
}

void Enemy::pozitionare(int x,int y)
{
    mPosX=x;
    mPosY=y;

    mVelX=0;
    go=0;

    isDead=false;

    mDeathCollider.x=x+4;
    mDeathCollider.y=y;
    mDeathCollider.w=EN_WIDTH-8;
    mDeathCollider.h=6;

    mCollider.x=x;
    mCollider.y=y+6;
    mCollider.w=EN_WIDTH;
    mCollider.h=EN_HEIGHT-6;
}

void Enemy::die()
{
    Mix_PlayChannel(-1,sKill,0);
    isDead=true;
    mPosX=0;
    mPosY=0;
    mCollider.x=0;
    mCollider.y=0;
}

void Enemy::move(SDL_Rect marioCollider)
{
    if(go==0)
        mVelX=-VEL;
    if(go==196)
        mVelX=VEL;
    mPosX+=mVelX;
    mCollider.x=mPosX;
    mDeathCollider.x=mPosX+4;
    go-=mVelX;
    if(checkCollision(marioCollider,mCollider))
        {printf("DIE!");die();}
}

void Enemy::render(int x)
{
    gEnemy.render(mPosX-x,mPosY);
}

Sprite::Sprite()
{
    //pozitia initiala a elementului
    mPosX = 32;
    mPosY = 128;

	//creaza obiectul de coliziune
	mCollider.w = SPR_WIDTH;
	mCollider.h = SPR_HEIGHT;

    //initializeaza velocitatea
    mVelX = 0;
    mVelY = 0;

    midAir=true;
    isJump=false;
    isDead=false;
    jumpTimes=0;
}

void Sprite::die()
{
    lives--;
    switch(rand()%3)
    {
        case 0:
            Mix_PlayChannel( -1, sDeath1, 0 );
            break;
        case 1:
            Mix_PlayChannel( -1, sDeath2, 0 );
            break;
        case 2:
            Mix_PlayChannel(-1,sDeath3,0);
            break;
    }

    SDL_Delay(2000);
    isDead=true;
    mPosX = 32;
    mPosY = 128;

    //creaza obiectul de coliziune
	mCollider.x = 32;
	mCollider.y = 128;

    //initializeaza velocitatea
    mVelX = 0;
    mVelY = 0;
}

Sprite::~Sprite()
{
    gSprTexture.free();
    mPosX = 8;
    mPosY = 8;

    //creaza obiectul de coliziune
	mCollider.x = 8;
	mCollider.y = 8;

    //initializeaza velocitatea
    mVelX = 0;
    mVelY = 0;
}

void Sprite::render( int camX, int camY )
{
    //pozitioneaza elementul in functie de camera
	gSprTexture.render( mPosX - camX, mPosY - camY );
}

int Sprite::getPosX()
{
	return mPosX;
}

int Sprite::getPosY()
{
	return mPosY;
}

void Sprite::handleEvent( SDL_Event& e )
{
    //cand se apasa tasta
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        //Ajusteaza viteza
        switch( e.key.keysym.sym )
        {
            case SDLK_UP: isJump=true; break;
            case SDLK_LEFT: mVelX -= VEL; break;
            case SDLK_RIGHT: mVelX += VEL; break;
        }
    }
    //daca tasta e ridicata
    else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        //ajusteaza viteza
        switch( e.key.keysym.sym )
        {
            case SDLK_LEFT: mVelX += VEL; break;
            case SDLK_RIGHT: mVelX -= VEL; break;
        }
    }
}

void Sprite::walk(SDL_Rect wall[])
{
    int i;
    mPosX += mVelX;
	mCollider.x = mPosX;
    for(i=0;i<k;i++)
    {

        if( ( mPosX < 0 ) || ( mPosX + SPR_WIDTH > LEVEL_WIDTH ) || mPosX<camera.x ||checkCollision( mCollider, wall[i] ) )
        {
            //Move back
            mPosX -= mVelX;
            mCollider.x = mPosX;
        }
    }
}

void Sprite::fall(SDL_Rect wall[])
{
    int i,a,ok=1;
    bool collided;
    mPosY += 8;
    mCollider.y = mPosY;
    for(i=0;i<ke;i++)
    {
        if(!enemy[i].isDead)
            if(checkCollision(mCollider,enemy[i].mCollider))
                {
                    die();
                }
            else
                if(checkCollision(mCollider,enemy[i].mDeathCollider))
                {
                    enemy[i].die();
                }
    }
    for(i=0;i<k;i++)
        {
            collided=checkCollision(mCollider,wall[i]);
            if(collided&&mCollider.h+mCollider.y>=wall[i].y)
            {
                ok=0;
            }
            if( ( mPosY < 0 ) || ( mPosY + SPR_HEIGHT > SCREEN_HEIGHT )|| collided )
            {
                //Move back
                if(mPosY+SPR_HEIGHT>=480)
                    die();
                mPosY -= 8;
                mCollider.y = mPosY;
            }
        }
    if(ok)
        midAir=true;
    else midAir=false;
}

void Sprite::jump(SDL_Rect wall[])
{
    int i;
    mPosY -= jumpTimes;
    mCollider.y = mPosY;
    //respinge obiectul
    for(i=0;i<k;i++)
        {
            if( ( mPosY < 0 ) || ( mPosY + SPR_HEIGHT > SCREEN_HEIGHT ) || checkCollision( mCollider, wall[i] ) )
            {
                //Move back
                mPosY += jumpTimes;
                mCollider.y = mPosY;
            }
        }
    jumpTimes-=4;
    if(jumpTimes<=0)
        jumpTimes=0,isJump=false;
}


void Sprite::move( SDL_Rect wall[] )
{
    printf("%d\n",mVelX);
    if(mVelX!=0)
        walk(wall);
    if(isJump)
        if(!midAir)
            if(jumpTimes==0)
            {
                jumpTimes=32;
                switch(rand()%3)
                {
                    case 0:
                        Mix_PlayChannel(-1,sJump1,0);
                        break;
                    case 1:
                        Mix_PlayChannel( -1, sJump2, 0 );
                        break;
                    case 2:
                        break;
                }

            }

    if(jumpTimes)
        jump(wall);
    else
        fall(wall);
}

void genereaza_inamici(Enemy enemy[],int &k)
{
    k=0;
    int x,y;
    std::ifstream f("inamici.in");
    while(f>>x>>y)
    {
        enemy[k].pozitionare(x,y);
        k++;
    }
    f.close();
}

void win()
{
    Mix_PlayChannel(-1,sWin,0);
    SDL_Delay(10000);
}

bool loadMedia()
{
	bool success = true;



	if(!gEnemy.loadFromFile("goombas_0.bmp"))
    {
        printf( "Failed to load dot texture!\n" );
		success = false;
    }

	//incarca elementul
	if( !gSprTexture.loadFromFile( "mario2.png" ) )
	{
		printf( "Failed to load dot texture!\n" );
		success = false;
	}

	//load background
	if( !gBGTexture.loadFromFile( "map.png" ) )
	{
		printf( "Failed to load background texture!\n" );
		success = false;
	}

	sMusic = Mix_LoadMUS( "music/overworld.wav" );
	if( sMusic == NULL )
	{
		printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	//Load sound effects
	sJump1 = Mix_LoadWAV( "music/HA_1.wav" );
	if( sJump1 == NULL )
	{
		printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	sJump2 = Mix_LoadWAV( "music/HA_2.wav" );
	if( sJump2 == NULL )
	{
		printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	sDeath1 = Mix_LoadWAV( "music/fall.wav" );
	if( sDeath1 == NULL )
	{
		printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	sDeath2 = Mix_LoadWAV( "music/mamamia.wav" );
	if( sDeath2 == NULL )
	{
		printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	sDeath3 = Mix_LoadWAV( "music/pizza.wav" );
	if( sDeath3 == NULL )
	{
		printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	sKill = Mix_LoadWAV( "music/haha.wav" );
	if( sKill == NULL )
	{
		printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	sIntro = Mix_LoadWAV( "music/its.wav" );
	if( sIntro == NULL )
	{
		printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	sWin = Mix_LoadWAV( "music/levelend.wav" );
	if( sWin == NULL )
	{
		printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	return success;
}

void close()
{
    Mix_FreeChunk( sJump1 );
	Mix_FreeChunk( sJump2 );
	Mix_FreeChunk( sWin );
	Mix_FreeChunk( sKill );
    Mix_FreeChunk( sDeath1 );
	Mix_FreeChunk( sDeath2 );
	Mix_FreeChunk( sDeath3 );
	Mix_FreeChunk( sIntro );
	sJump1 = NULL;
	sJump2 = NULL;
	sWin = NULL;
	sKill = NULL;
	sDeath1 = NULL;
	sDeath2 = NULL;
	sDeath3 = NULL;
	sIntro = NULL;

	Mix_FreeMusic( sMusic );
	sMusic = NULL;
	//elibereaza imaginile
	gSprTexture.free();
	gBGTexture.free();

	//distruge ferestrele
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//inchide sdl-ul
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision( SDL_Rect a, SDL_Rect b )
{
    //laturile patratelor de coliziune
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    //calculeaza laturile lui A
    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;

    //calculeaza laturile lui B
    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;

    //verifica daca exista coliziunea
    if( bottomA <= topB )
    {
        return false;
    }

    if( topA >= bottomB )
    {
        return false;
    }

    if( rightA <= leftB )
    {
        return false;
    }

    if( leftA >= rightB )
    {
        return false;
    }

    //exista coliziune daca e true
    return true;
}

bool init()
{
	bool success = true;

	//Initializeaza sdl
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{

		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Creaza fereastra
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//creaza motorul de rendering
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initializeaza culoarea renderului
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initializeaza incarcarea PNG-urilor
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}

				if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
				{
					printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}
int main( int argc, char* args[] )
{

	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{
		    k=0;
		    genereaza_obstacole(wall,k);
		    Sprite mario;
            while(lives&&!isWin)
            {
                camera.x=0;
                camera.y=0;
                //Main loop flag
                bool quit = false;
                mario.isDead=false;
                //Event handler
                SDL_Event e;

                int i,cameraAux,ok=1;

                ke=0;

                genereaza_inamici(enemy,ke);
                Mix_PlayChannel(-1,sIntro,0);
                Mix_PlayMusic(sMusic,-1);

                mario.mVelX=0;
                while( (!quit && !mario.isDead)&& !isWin )
                {
                    //Handle events
                    while( SDL_PollEvent( &e ) != 0 )
                    {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        mario.handleEvent( e );
                    }

                    for(i=0; i<ke; i++)
                        if(!enemy[i].isDead)
                            enemy[i].move(mario.mCollider);
                    mario.move(wall);

                    //Centreaza camera
                    cameraAux=camera.x;
                    camera.x = ( mario.getPosX() + Sprite::SPR_WIDTH / 2 ) - SCREEN_WIDTH / 2;
                    camera.y = ( mario.getPosY() + Sprite::SPR_HEIGHT / 2 ) - SCREEN_HEIGHT / 2;
                    //centreaza camera in background
                    if(camera.x<cameraAux)
                    {
                        camera.x=cameraAux;
                    }

                    if( camera.x < 0 )
                    {
                        camera.x = 0;
                    }
                    if( camera.y < 0 )
                    {
                        camera.y = 0;
                    }
                    if( camera.x > LEVEL_WIDTH - camera.w )
                    {
                        camera.x = LEVEL_WIDTH - camera.w;
                    }
                    if( camera.y > LEVEL_HEIGHT - camera.h )
                    {
                        camera.y = LEVEL_HEIGHT - camera.h;
                    }

                    //sterge ecranul
                    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                    SDL_RenderClear( gRenderer );

                    //randeaza background-ul
                    gBGTexture.render( 0, 0, &camera );
                    for(i=0; i<ke; i++)
                        if(!enemy[i].isDead)
                            enemy[i].render(camera.x);

                    mario.render( camera.x, camera.y );

                    SDL_SetRenderDrawColor( gRenderer,0x00, 0x00, 0x00, 0xFF );

                    SDL_RenderPresent( gRenderer );

                    if(mario.mPosX>6336)
                    {
                        Mix_HaltMusic();
                        isWin=true;
                        win();
                    }
                }


            }
            mario.~Sprite();
		}
	}
	close();

	return 0;
}
