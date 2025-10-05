#include <raylib.h>
#include <iostream>
#include<stdbool.h>
#include<time.h>
#include<math.h>

#define BOARD_SIZE 8
#define TILE_SIZE 42
#define TILE_TYPES 5
#define SCORE_FONT_SIZE 32
#define MAX_SCORE_POPUPS 32
const char tile_chars[TILE_TYPES]={'#','@','$','%','&'};

char board[BOARD_SIZE][BOARD_SIZE];
bool matched[BOARD_SIZE][BOARD_SIZE];
float falloffset[BOARD_SIZE][BOARD_SIZE]={0};

//center tile
int score=0;
Vector2 grid_origin;
Texture2D background;
Font scorefont;
Vector2 selectedtile={-1,-1};
float fallspeed=8.0f;
float matchdelaytimer=0.0f;
const float MATCH_DELAY_DURATION=0.2f;

float score_Scale=1.0f;
float score_scale_velocity=0.0f;
bool score_animating=false;

float score_scale=1.0f; 

Music background_music;
Sound match_sound;

typedef enum{
    STATE_IDLE,
    STATE_ANIMATING,
    STATE_MATCH_DELAY
}TileState;

TileState tilestate;

typedef struct{
    Vector2 position;
    int amount;
    float lifetime;
    float alpha;
    bool active;
}ScorePopup;

ScorePopup scorepopups[MAX_SCORE_POPUPS]={0};  

// int score=200;
//return random character
char random_tile(){
    return tile_chars[rand()%TILE_TYPES];
}

void swaptiles(int x1,int y1,int x2,int y2){
    char temp=board[x1][y1];
    board[x1][y1]=board[x2][y2];
    board[x2][y2]=temp;
}

bool areTilesAdjacent(Vector2 a,Vector2 b){
    return abs((int)a.x-(int)b.x)+ abs((int)a.y-(int)b.y)==1;
}

void addscorepopup(int x,int y,int amount,Vector2 grid_origin){
    for(int i=0;i<MAX_SCORE_POPUPS;i++){
        if(!scorepopups[i].active){
            scorepopups[i].position=(Vector2){
                grid_origin.x+x*TILE_SIZE+TILE_SIZE/2,
                grid_origin.y+y*TILE_SIZE+TILE_SIZE/2
            };
            scorepopups[i].amount=amount;
            scorepopups[i].lifetime=1.0f;
            scorepopups[i].alpha=1.0f;
            scorepopups[i].active=true;
            break;
        }
    }
}
bool find_matches(){
    bool found=false;
    for(int i=0;i<BOARD_SIZE;i++){
        for(int j=0;j<BOARD_SIZE;j++){
            matched[i][j]=false;
        }
    }
    //horizontal matches
    for(int i=0;i<BOARD_SIZE;i++){
        for(int j=0;j<BOARD_SIZE-2;j++){
            char t=board[i][j];
            if(t==board[i][j+1] && t==board[i][j+2]){
                matched[i][j]=matched[i][j+1]=matched[i][j+2]=true;
                score+=10;
                found=true;
                PlaySound(match_sound);
                score_animating=true;
                score_scale=2.0f;
                score_scale_velocity=-2.5f;
                addscorepopup(j,i,10,grid_origin);
            }
        }
    }

    //vertical matches
    for(int i=0;i<BOARD_SIZE;i++){
        for(int j=0;j<BOARD_SIZE-2;j++){
            char t=board[j][i];
            if(t==board[j+1][i] && t==board[j+2][i]){
                matched[j][i]=matched[j+1][i]=matched[j+2][i]=true;
                score+=10;
                found=true;
                PlaySound(match_sound);
                score_animating=true;
                score_scale=2.0f;
                score_scale_velocity=-2.5f;
                addscorepopup(j,i,10,grid_origin);
            }
        }
    }
    return found;
}
//collapse matched tiles and replace empty spaces
void resolve_matches(){
    for(int j=0;j<BOARD_SIZE;j++){
        int write_i=  BOARD_SIZE-1;
        for(int i=BOARD_SIZE-1;i>=0;i--){
            if(!matched[i][j]){
                if(i!=write_i){
                    board[write_i][j]=board[i][j];
                    falloffset[write_i][j]=(write_i-i)*TILE_SIZE;
                    board[i][j]=' ';
                }
                // board[write_j][i]=board[j][i];
                write_i--;
            }
        }
        while(write_i>=0){
            board[write_i][j]=random_tile();
            falloffset[write_i][j]=(write_i+1)*TILE_SIZE;
            write_i--;
        }
    }
    tilestate=STATE_ANIMATING;
}
//initialise board and fill it with random tiles
void init_board()
{
    for(int i=0;i<BOARD_SIZE;i++)
    {
        for(int j=0;j<BOARD_SIZE;j++)
        {
            board[i][j]=random_tile();
        }
    }
    int grid_width=BOARD_SIZE*TILE_SIZE;
    int grid_height=BOARD_SIZE*TILE_SIZE;
    grid_origin=(Vector2){
        (GetScreenWidth()-grid_width)/2,
        (GetScreenHeight()-grid_height)/2
    };

    if(find_matches()){
        resolve_matches();
    }
    else{
        tilestate=STATE_IDLE;
    }
}

    
int main()
{
    const int screen_width=800;
    const int screen_height=450;
    InitWindow(screen_width,screen_height,"Raylib match");
    SetTargetFPS(60);
    srand(time(NULL));

    InitAudioDevice();
    background=LoadTexture("assets/space.png");
    scorefont=LoadFontEx("assets/PressStart2P.ttf",SCORE_FONT_SIZE,NULL,0);
    background_music=LoadMusicStream("assets/bgm_old.mp3");
    match_sound =LoadSound("assets/match.mp3");
    PlayMusicStream(background_music);
    init_board();
    Vector2 mouse={0,0};
    while(!WindowShouldClose())
    {
        UpdateMusicStream(background_music);
        //game logic
        mouse=GetMousePosition();
        if(tilestate==STATE_IDLE && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            int x=(mouse.x-grid_origin.x)/TILE_SIZE;
            int y=(mouse.y-grid_origin.y)/TILE_SIZE;
            if(x>=0 && x<BOARD_SIZE && y>=0 && y<BOARD_SIZE){
                // selectedtile={x ,y};
                Vector2 current_tile=(Vector2){x,y};
                if(selectedtile.x<0){
                    selectedtile=current_tile;
                }
                else{
                    if(areTilesAdjacent(selectedtile,current_tile)){
                        swaptiles(selectedtile.y,selectedtile.x,current_tile.y,current_tile.x);
                        if(find_matches()){
                            resolve_matches();
                        }
                        else{
                            swaptiles(selectedtile.y,selectedtile.x,current_tile.y,current_tile.x);
                        }
                    }
                    selectedtile=(Vector2){-1,-1};
                }
            }
        }


        // if(find_matches()){
        //     resolve_matches();
        // }

        if(tilestate==STATE_ANIMATING){
            bool stillanimating=false;
            for(int i=0;i<BOARD_SIZE;i++)
            {
                for(int j=0;j<BOARD_SIZE;j++)
                {
                    if(falloffset[i][j]>0){
                        falloffset[i][j]-=fallspeed;
                        if(falloffset[i][j]<0){
                            falloffset[i][j]=0;
                        }
                        else{
                            stillanimating=true;
                        }
                    }
                }
            }
            if(!stillanimating){
                tilestate=STATE_MATCH_DELAY;
                matchdelaytimer=MATCH_DELAY_DURATION;
            }
        }

        if(tilestate==STATE_MATCH_DELAY){
            matchdelaytimer-=GetFrameTime();
            if(matchdelaytimer<=0.0f){
                if(find_matches()){
                    resolve_matches();
                }
                else{
                    tilestate=STATE_IDLE;
                }
            }
        }

        //update score popups array
        for(int i=0;i<MAX_SCORE_POPUPS;i++){
            if(scorepopups[i].active){
                scorepopups[i].lifetime-=GetFrameTime();
                scorepopups[i].position.y-=30*GetFrameTime();
                scorepopups[i].alpha=scorepopups[i].lifetime;
                if(scorepopups[i].lifetime<=0.0f){
                    scorepopups[i].active=false;
                }

            }
        }

        if(score_animating){
            score_scale+=score_scale_velocity*GetFrameTime();
            if(score_scale<=1.0f){
                score_scale=1.0f;
                score_animating=false;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(
            background,
            (Rectangle){
                0,0,background.width,background.height
            },
            (Rectangle){
                0,0,GetScreenWidth(),GetScreenHeight()
            },
            (Vector2){0,0},
            0.0f,
            WHITE
        );

        DrawRectangle(
            grid_origin.x,
            grid_origin.y,
            BOARD_SIZE*TILE_SIZE,
            BOARD_SIZE*TILE_SIZE,
            Fade(DARKGRAY,0.60f)
        );

        for(int i=0;i<BOARD_SIZE;i++)
        {
            for(int j=0;j<BOARD_SIZE;j++)
            {
                Rectangle rect={
                    grid_origin.x+(j*TILE_SIZE),//x posn
                    grid_origin.y+(i*TILE_SIZE),//y posn
                    TILE_SIZE,//width
                    TILE_SIZE//height
                };
                DrawRectangleLinesEx(rect,1,DARKGRAY);
                
                if(board[i][j]!=' '){
                    DrawTextEx(
                        GetFontDefault(),
                        TextFormat("%c",board[i][j]),
                        (Vector2){
                            rect.x+12,rect.y+8-falloffset[i][j]
                        },
                        20,1,
                        matched[i][j]?GREEN:WHITE
                    );
                }
            }
        }
        //draw selected tile
        if(selectedtile.x>=0){
            DrawRectangleLinesEx((Rectangle){
                grid_origin.x+(selectedtile.x*TILE_SIZE),
                grid_origin.y+(selectedtile.y*TILE_SIZE),
                TILE_SIZE,TILE_SIZE
            },2,YELLOW);
        }
        DrawTextEx(
            scorefont,
            TextFormat("SCORE:%d",score),
            (Vector2){20,20},//keep in topleft
            SCORE_FONT_SIZE*score_scale,
            1.0f,
            YELLOW
        );

        for(int i=0;i<MAX_SCORE_POPUPS;i++){
            if(scorepopups[i].active){
                Color c=Fade(YELLOW,scorepopups[i].alpha);
                DrawText(
                    TextFormat("+%d",scorepopups[i].amount),
                    scorepopups[i].position.x,
                    scorepopups[i].position.y,
                    20,c);
            }
        }
        // DrawText(TextFormat("SCORE:%d",score),20,20,24,YELLOW);
        EndDrawing();
    }
    StopMusicStream(background_music);
    UnloadMusicStream(background_music);
    UnloadSound(match_sound);
    UnloadTexture(background);
    UnloadFont(scorefont);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}