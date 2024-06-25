#include <time.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>

#define linhas 16
#define colunas 16

const int larguraTela = 600;
const int comprimentoTela = 480;

const int larguraBloco = larguraTela / colunas;
const int comprimentoBloco = comprimentoTela / linhas;

typedef struct Bloco{
    int i;
    int j;
    bool possuiBomba;
    bool possuiFlag;
    bool revelado;
    int bombasProximas;
}Bloco;

Bloco grid[linhas][colunas];

Texture2D flagImagem;

bool IndexValido(int, int);
void DesenharBloco(Bloco);
void RevelarBloco(int, int);
void BlocoFlag(int, int);
int BlocoBombasProximas(int, int);
void IniciarGrid(void);
void LimparGrid(int, int);

int main(){

    Color lightGray = {211, 211, 211, 255};
    InitWindow(larguraTela, comprimentoTela, "Campo Minado");
    SetTargetFPS(60);

    flagImagem = LoadTexture("texture/flag.png");

    IniciarGrid();

    while(WindowShouldClose() == false){

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            Vector2 mPos = GetMousePosition();
            int indexI = mPos.x / larguraBloco;
            int indexJ = mPos.y / comprimentoBloco;

            if(IndexValido(indexI, indexJ)){
                RevelarBloco(indexI, indexJ);
            }
        }else if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
            Vector2 mPos = GetMousePosition();
            int indexI = mPos.x / larguraBloco;
            int indexJ = mPos.y / comprimentoBloco;

            if(IndexValido(indexI, indexJ)){
                BlocoFlag(indexI, indexJ);
            }
        }
        BeginDrawing();
        ClearBackground(lightGray);

        for(int i= 0; i < colunas; i++){
            for(int j=0; j < linhas; j++){
                DesenharBloco(grid[i][j]);
            }
        }

        EndDrawing();
    }


    CloseWindow();
    return 0;
}

void DesenharBloco(Bloco bloco){
    if(bloco.revelado){
        if(bloco.possuiBomba){
            DrawRectangle(bloco.i * larguraBloco, bloco.j * comprimentoBloco, larguraBloco, comprimentoBloco, RED);
        }else{
            DrawRectangle(bloco.i * larguraBloco, bloco.j * comprimentoBloco, larguraBloco, comprimentoBloco, GRAY);
            if(bloco.bombasProximas > 0){
            DrawText(TextFormat("%d", bloco.bombasProximas), bloco.i * larguraBloco + 8, bloco.j * comprimentoBloco + 8, comprimentoBloco - 8, BLACK);
            }
        }
    }else if(bloco.possuiFlag){
        Rectangle source = {0, 0, flagImagem.width, flagImagem.height};
        Rectangle dest = {bloco.i * larguraBloco, bloco.j * comprimentoBloco, larguraBloco, comprimentoBloco};
        Vector2 origin = {0, 0};

        DrawTexturePro(flagImagem, source, dest, origin, 0.0f, Fade(LIGHTGRAY, 0.8f));
    }

    DrawRectangleLines(bloco.i * larguraBloco, bloco.j * comprimentoBloco, larguraBloco, comprimentoBloco, BLACK);
}

bool IndexValido(int i, int j){
    return i >= 0 && i < colunas && j >= 0 && j <  linhas;
}

void RevelarBloco(int i, int j){
    if(grid[i][j].possuiFlag || grid[i][j].revelado){
        return;
    }
    
    grid[i][j].revelado = true;

    if(grid[i][j].possuiBomba){
        // perde
    }else{
        if(grid[i][j].bombasProximas == 0){
            LimparGrid(i, j);
        }
    }
}

void BlocoFlag(int i, int j){
    if(grid[i][j].revelado){
        return;
    }
    
    grid[i][j].possuiFlag = !grid[i][j].possuiFlag;
}

int BlocoBombasProximas(int i, int j){
    int contagem = 0;
    for(int ioff= -1 ; ioff <= 1; ioff++){
        for(int joff= -1 ; joff <= 1; joff++){
            if(ioff == 0 && joff == 0){
                continue;
            }

            if(!IndexValido(i + ioff, j + joff)){
                continue;
            }
            if(grid[i + ioff][j + joff].possuiBomba){
                    contagem++;
            }            
        }
    }
    return contagem;
}

void IniciarGrid(void){
    for(int i=0; i < colunas; i++){
        for(int j=0; j< linhas; j++){
            grid[i][j] = (Bloco) {
                .i = i,
                .j = j,
                .possuiBomba = false,
                .revelado = false,
                .bombasProximas = -1,
            };
        }
    }

    // Colocando Bombas
    int quantidadeBombas = (int)(linhas * colunas * 0.1f);
    while(quantidadeBombas > 0){
        int i = rand() % colunas;
        int j = rand() % linhas;

        if(!grid[i][j].possuiBomba){
            grid[i][j].possuiBomba = true;
            quantidadeBombas--;
        }
    }

    for(int i=0; i < colunas; i++){
        for(int j=0; j < linhas; j++){
            if(!grid[i][j].possuiBomba){
                grid[i][j].bombasProximas = BlocoBombasProximas(i, j);
            }
        }
    }
}

void LimparGrid(int i, int j){
    for(int ioff= -1 ; ioff <= 1; ioff++){
        for(int joff= -1 ; joff <= 1; joff++){
            if(ioff == 0 && joff == 0){
                continue;
            }

            if(!IndexValido(i + ioff, j + joff)){
                continue;
            }
            if(!grid[i + ioff][j + joff].revelado){
                RevelarBloco(i + ioff, j + joff);
            }            
        }
    }
}