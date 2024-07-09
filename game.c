#include <time.h>
#include <stdlib.h>
#include <locale.h>

#include <raylib.h>
#include <raymath.h>

#define linhas 16
#define colunas 16

// VARIAVEIS CONSTANTES
const int larguraTela = 600;
const int comprimentoTela = 480;
const char* derrota = "Você perdeu!";
const char* reiniciar = "Aperte espaço para reiniciar.";
const char* vitoria = "Você ganhou!";

const int larguraBloco = larguraTela / colunas;
const int comprimentoBloco = comprimentoTela / linhas;

// ESTRUTURA DE CADA BLOCO/CELULA DA MATRIZ DO JOGO
typedef struct Bloco{
    int i;
    int j;
    bool possuiBomba;
    bool possuiFlag;
    bool revelado;
    int bombasProximas;
}Bloco;

// ESTRUTURA DO TIPO ENUM PARA DEFINIR O ESTADO ATUAL DO JOGADOR
typedef enum GameStatus{
    MENU,
    JOGANDO,
    DERROTA,
    VITORIA
}GameStatus;

GameStatus status;

Bloco grid[linhas][colunas];

Texture2D flagImagem;
Texture2D bombaImagem;
int blocosRevelados;
int bombasExistentes;
float inicioCronometro;
float fimCronometro;

// PROTOTIPAGEM DAS FUNÇÕES
bool IndexValido(int, int);
void DesenharBloco(Bloco);
void RevelarBloco(int, int);
void BlocoFlag(int, int);
int BlocoBombasProximas(int, int);
void IniciarGrid(void);
void LimparGrid(int, int);
void GameInit(void);

int main(){
    setlocale(LC_ALL, "Portuguese");
    Color lightGray = {211, 211, 211, 255};
    InitWindow(larguraTela, comprimentoTela, "Campo Minado");
    SetTargetFPS(60);

    // CARREGANDO AS IMAGENS DA BANDEIRA E DA BOMBA
    flagImagem = LoadTexture("texture/flag.png");
    bombaImagem = LoadTexture("texture/bomba.png");

    // FAZ O JOGO COMEÇAR NO MENU
    status = MENU;

    // ENQUANTO A JANELA NÃO FECHAR O JOGO VAI RODAR
    while(WindowShouldClose() == false){

        // ESTRUTURA DO MENU 
         if (status == MENU) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mousePoint = GetMousePosition();

                Rectangle playButton = { larguraTela / 2 - 50, comprimentoTela / 2 - 50, 100, 50 };
                Rectangle exitButton = { larguraTela / 2 - 50, comprimentoTela / 2 + 10, 100, 50 };

                if (CheckCollisionPointRec(mousePoint, playButton)) {
                    status = JOGANDO;
                    GameInit();
                } else if (CheckCollisionPointRec(mousePoint, exitButton)) {
                    CloseWindow();
                    return 0;
                }
            }
        }

        // "Funções" para registrar o clique do mouse e revelar ou colocar uma flag no bloco
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            Vector2 mPos = GetMousePosition();
            int indexI = mPos.x / larguraBloco;
            int indexJ = mPos.y / comprimentoBloco;

            if(status == JOGANDO && IndexValido(indexI, indexJ) && !grid[indexI][indexJ].revelado){
                RevelarBloco(indexI, indexJ);
            }
        }else if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
            Vector2 mPos = GetMousePosition();
            int indexI = mPos.x / larguraBloco;
            int indexJ = mPos.y / comprimentoBloco;

            if(status == JOGANDO && IndexValido(indexI, indexJ)){
                BlocoFlag(indexI, indexJ);
            }
        }
        
        // Apertar Espaço para reiniciar o jogo
        if(IsKeyPressed(KEY_SPACE)){
            GameInit();
        }

        BeginDrawing();
        ClearBackground(lightGray);

        // "Frontend" do MENU
        if (status == MENU)
        {
            DrawText("Campo Minado", larguraTela / 2 - MeasureText("Campo Minado", 40) / 1.75, comprimentoTela / 4, 55, BLACK);

            Rectangle playButton = { larguraTela / 2 - 50, comprimentoTela / 2 - 50, 100, 50 };
            Rectangle exitButton = { larguraTela / 2 - 50, comprimentoTela / 2 + 10, 100, 50 };

            DrawRectangleRec(playButton, DARKGRAY);
            DrawRectangleRec(exitButton, DARKGRAY);

            DrawText("Jogar", larguraTela / 2 - MeasureText("Jogar", 20) / 2, comprimentoTela / 2 - 35, 20, BLACK);
            DrawText("Sair", larguraTela / 2 - MeasureText("Sair", 20) / 2, comprimentoTela / 2 + 25, 20, BLACK);
        } else {
            for (int i = 0; i < colunas; i++) {
                for (int j = 0; j < linhas; j++) {
                    DesenharBloco(grid[i][j]);
                }
            }
        }
        
        
        // MENSAGEM DE DERROTA E VITÓRIA
        if(status == DERROTA){
            DrawRectangle(0, 0, larguraTela, comprimentoTela, Fade(WHITE, 0.6f));
            DrawText(derrota, larguraTela / 2 - MeasureText(derrota, 30) / 2, comprimentoTela / 2 - 20, 30,  BLACK);
            DrawText(reiniciar, larguraTela / 2 - MeasureText(reiniciar, 30) / 2, comprimentoTela * 0.55f, 30,  BLACK);

            int minutos = (int)(fimCronometro - inicioCronometro) / 60;
            int segundos = (int)(fimCronometro - inicioCronometro) % 60;
            DrawText(TextFormat("Tempo de jogo: %d minutos, %d segundos.", minutos, segundos), 20, comprimentoTela - 40, 20, BLACK);
        }

        if(status == VITORIA){
            DrawRectangle(0, 0, larguraTela, comprimentoTela, Fade(WHITE, 0.9f));
            DrawText(vitoria, larguraTela / 2 - MeasureText(vitoria, 30) / 2, comprimentoTela / 2 - 20, 30,  GREEN);
            DrawText(reiniciar, larguraTela / 2 - MeasureText(reiniciar, 30) / 2, comprimentoTela * 0.55f, 30,  BLACK);

            int minutos = (int)(fimCronometro - inicioCronometro) / 60;
            int segundos = (int)(fimCronometro - inicioCronometro) % 60;
            DrawText(TextFormat("Tempo de jogo: %d minutos, %d segundos.", minutos, segundos), 20, comprimentoTela - 40, 20, BLACK);
        }

        EndDrawing();
    }


    CloseWindow();
    return 0;
}

// Função para desenhar os blocos com bombas e/ou flags e desenhar a grid do jogo
void DesenharBloco(Bloco bloco){
    if(bloco.revelado){
        if(bloco.possuiBomba){
        Rectangle source = {0, 0, bombaImagem.width, bombaImagem.height};
        Rectangle dest = {bloco.i * larguraBloco, bloco.j * comprimentoBloco, larguraBloco, comprimentoBloco};
        Vector2 origin = {0, 0};

        DrawTexturePro(bombaImagem, source, dest, origin, 0.0f, Fade(RED, 0.8f));
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

// Função para verificar se o index que o usuario clicou é valida
bool IndexValido(int i, int j){
    return i >= 0 && i < colunas && j >= 0 && j <  linhas;
}

// Função para que ao clicar no bloco revele se tem ou não uma bomba
void RevelarBloco(int i, int j){
    if(grid[i][j].possuiFlag || grid[i][j].revelado){
        return;
    }
    
    grid[i][j].revelado = true;

    if(grid[i][j].possuiBomba){
        status = DERROTA;
        fimCronometro = GetTime();

    }else{
        if(grid[i][j].bombasProximas == 0){
            LimparGrid(i, j);
        }
        blocosRevelados++;

        if(blocosRevelados == linhas * colunas - bombasExistentes){
            status = VITORIA;
            fimCronometro = GetTime();
        }
    }
}

// Função para colocar ou tirar a flag
void BlocoFlag(int i, int j){
    if(grid[i][j].revelado){
        return;
    }
    
    grid[i][j].possuiFlag = !grid[i][j].possuiFlag;
}

// Função para contar a quantidade de bombas ao redor do bloco
int BlocoBombasProximas(int i, int j){
    int contagem = 0;
    for(int ioff= -1 ; ioff <= 1; ioff++){ // ioff representa o bloco da linha anterior, da linha do proprio bloco e a proxima linha
        for(int joff= -1 ; joff <= 1; joff++){ // joff representa o bloco da linha anterior, da linha do proprio bloco e a proxima linha
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

// Inicialização da grid
void IniciarGrid(void){
    for(int i=0; i < colunas; i++){
        for(int j=0; j< linhas; j++){
            grid[i][j] = (Bloco) { // Iniciando a struct bloco
                .i = i,
                .j = j,
                .possuiBomba = false,
                .revelado = false,
                .bombasProximas = -1,
            };
        }
    }

    // Colocando Bombas
    bombasExistentes = (int)(linhas * colunas * 0.1f);
    int quantidadeBombas = bombasExistentes;
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

// Função para retirar todos os blocos ao redor que possuem o valor 0 e assim não poluir visualmente o jogo
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

// Inicializar o jogo
void GameInit(){
    IniciarGrid();
    status = JOGANDO;
    blocosRevelados = 0;
    inicioCronometro = GetTime();
}