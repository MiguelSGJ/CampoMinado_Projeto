#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <raylib.h>
#include <raymath.h>
#include <string.h>

#define linhas 16
#define colunas 16

#define MAX_JOGADORES 10

// VARIAVEIS CONSTANTES
const int larguraTela = 600;
const int comprimentoTela = 480;
const char* derrota = "Você perdeu!";
const char* reiniciar = "Aperte espaço para reiniciar.";
const char* home = "Aperte a tecla F9 para voltar ao menu.";
const char* vitoria = "Você ganhou!";

const int larguraBloco = larguraTela / colunas;
const int comprimentoBloco = comprimentoTela / linhas;

// ESTRUTURA DE CADA BLOCO/CELULA DA MATRIZ DO JOGO
typedef struct Bloco {
    int i;
    int j;
    int bombasProximas;
    bool possuiBomba;
    bool possuiFlag;
    bool revelado;
} Bloco;

// ESTRUTURA DO TIPO ENUM PARA DEFINIR O ESTADO ATUAL DO JOGADOR
typedef enum GameStatus {
    MENU,
    DIFICULDADE,
    RANKING,
    JOGANDO,
    DERROTA,
    VITORIA,
    INSERIR_NOME
} GameStatus;

typedef enum Dificuldade {
    FACIL,
    MEDIO,
    DIFICIL
} Dificuldade;

typedef struct Player {
    char nome[50];
    float tempo;
    Dificuldade dificuldade;
} Player;

Player ranking[MAX_JOGADORES];
Dificuldade dificuldade;
GameStatus status;
Bloco grid[linhas][colunas];

Texture2D flagImagem;
Texture2D bombaImagem;
int blocosRevelados;
int tamanhoRanking = 0;
int bombasExistentes;
float inicioCronometro;
float fimCronometro;
float ignoreClickTime = 0.3f;
float transitionTime = 0.3f;
bool mostrarTelaFinal = false; // Nova variável para monitorar se a tela final foi exibida

char nomeJogador[50];
int nomeIndex = 0;

// PROTOTIPAGEM DAS FUNÇÕES
bool IndexValido(int, int);
void DesenharBloco(Bloco);
void RevelarBloco(int, int);
void BlocoFlag(int, int);
int BlocoBombasProximas(int, int);
void IniciarGrid(void);
void LimparGrid(int, int);
void GameInit(void);
void SalvarRanking(Player* ranking, int tamanho);
int CarregarRanking(Player* ranking, int maxTamanho);
void AtualizarRanking(Player* ranking, int* tamanho, Player novoJogador);
void ExibirRanking(Player* ranking, int tamanho);
int comparePlayers(const void* a, const void* b);

int main() {
    setlocale(LC_ALL, "Portuguese");
    Color lightGray = { 211, 211, 211, 255 };
    InitWindow(larguraTela, comprimentoTela, "Campo Minado");
    SetTargetFPS(60);

    // CARREGANDO AS IMAGENS DA BANDEIRA E DA BOMBA
    flagImagem = LoadTexture("texture/flag.png");
    bombaImagem = LoadTexture("texture/bomba.png");

    // CARREGANDO RANKING
    tamanhoRanking = CarregarRanking(ranking, MAX_JOGADORES);

    // FAZ O JOGO COMEÇAR NO MENU
    status = MENU;

    // ENQUANTO A JANELA NÃO FECHAR O JOGO VAI RODAR
    while (WindowShouldClose() == false) {

        // ESTRUTURA DO MENU 
        if (status == MENU) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mousePoint = GetMousePosition();

                Rectangle playButton = { larguraTela / 2 - 50, comprimentoTela / 2 - 50, 100, 50 };
                Rectangle rankingButton = { larguraTela / 2 - 50, comprimentoTela / 2 + 20, 100, 50 };
                Rectangle exitButton = { larguraTela / 2 - 50, comprimentoTela / 2 + 90, 100, 50 };

                if (CheckCollisionPointRec(mousePoint, playButton)) {
                    status = DIFICULDADE;
                    transitionTime = GetTime();
                } else if (CheckCollisionPointRec(mousePoint, rankingButton)) {
                    status = RANKING;
                } else if (CheckCollisionPointRec(mousePoint, exitButton)) {
                    CloseWindow();
                    return 0;
                }
            }
        }

        if (status == DIFICULDADE) {
            float currentTime = GetTime();
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mousePoint = GetMousePosition();

                Rectangle easyButton = { larguraTela / 2 - 50, comprimentoTela / 2 - 90, 100, 50 };
                Rectangle mediumButton = { larguraTela / 2 - 50, comprimentoTela / 2 - 20, 100, 50 };
                Rectangle hardButton = { larguraTela / 2 - 50, comprimentoTela / 2 + 70, 100, 50 };

                if (CheckCollisionPointRec(mousePoint, easyButton)) {
                    dificuldade = FACIL;
                    status = JOGANDO;
                    GameInit();
                } else if (CheckCollisionPointRec(mousePoint, mediumButton)) {
                    dificuldade = MEDIO;
                    status = JOGANDO;
                    GameInit();
                } else if (CheckCollisionPointRec(mousePoint, hardButton)) {
                    dificuldade = DIFICIL;
                    status = JOGANDO;
                    GameInit();
                }
            }
        }

        // "Funções" para registrar o clique do mouse e revelar ou colocar uma flag no bloco
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mPos = GetMousePosition();
            int indexI = mPos.x / larguraBloco;
            int indexJ = mPos.y / comprimentoBloco;

            if (status == JOGANDO && IndexValido(indexI, indexJ) && !grid[indexI][indexJ].revelado) {
                RevelarBloco(indexI, indexJ);
            } else if ((status == DERROTA || status == VITORIA) && !mostrarTelaFinal) {
                mostrarTelaFinal = true;
            }
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            Vector2 mPos = GetMousePosition();
            int indexI = mPos.x / larguraBloco;
            int indexJ = mPos.y / comprimentoBloco;

            if (status == JOGANDO && IndexValido(indexI, indexJ)) {
                BlocoFlag(indexI, indexJ);
            }
        }

        // Apertar Espaço para reiniciar o jogo
        if (IsKeyPressed(KEY_SPACE)) {
            GameInit();
        }

        // Apertar Home para voltar ao menu
        if (IsKeyPressed(KEY_F9)) {
            status = MENU;
        }

        // Entrada de texto para o nome do jogador
        if (status == INSERIR_NOME) {
            int key = GetKeyPressed();
            if ((key >= 32) && (key <= 125) && (nomeIndex < 49)) {
                nomeJogador[nomeIndex] = (char)key;
                nomeIndex++;
                nomeJogador[nomeIndex] = '\0';
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                nomeIndex--;
                if (nomeIndex < 0) nomeIndex = 0;
                nomeJogador[nomeIndex] = '\0';
            }
            if (IsKeyPressed(KEY_ENTER) && nomeIndex > 0) { // Apenas se o jogador tiver inserido algum nome
                Player novoJogador;
                strcpy(novoJogador.nome, nomeJogador);
                novoJogador.tempo = fimCronometro - inicioCronometro;
                novoJogador.dificuldade = dificuldade;
                if (novoJogador.tempo > 0) { // Adiciona ao ranking apenas se o tempo for maior que zero
                AtualizarRanking(ranking, &tamanhoRanking, novoJogador);
                SalvarRanking(ranking, tamanhoRanking);
                }
                status = RANKING;
            }
        }

        BeginDrawing();
        ClearBackground(lightGray);

        // Frontend das diversas janelas
        if (status == MENU) {
            DrawText("Campo Minado", larguraTela / 2 - MeasureText("Campo Minado", 40) / 1.75, comprimentoTela / 4, 55, BLACK);

            Rectangle playButton = { larguraTela / 2 - 50, comprimentoTela / 2 - 50, 100, 50 };
            Rectangle rankingButton = { larguraTela / 2 - 50, comprimentoTela / 2 + 20, 100, 50 };
            Rectangle exitButton = { larguraTela / 2 - 50, comprimentoTela / 2 + 90, 100, 50 };

            DrawRectangleRec(playButton, DARKGRAY);
            DrawRectangleRec(rankingButton, DARKGRAY);
            DrawRectangleRec(exitButton, DARKGRAY);

            DrawText("Jogar", larguraTela / 2 - MeasureText("Jogar", 20) / 2, comprimentoTela / 2 - 35, 20, BLACK);
            DrawText("Ranking", larguraTela / 2 - MeasureText("Ranking", 20) / 2, comprimentoTela / 2 + 35, 20, BLACK);
            DrawText("Sair", larguraTela / 2 - MeasureText("Sair", 20) / 2, comprimentoTela / 2 + 105, 20, BLACK);
        }

        if (status == DIFICULDADE) {
            DrawText("Escolha a dificuldade", larguraTela / 2 - MeasureText("Escolha a dificuldade", 30) / 2, comprimentoTela / 4 - 25, 30, BLACK);

            Rectangle easyButton = { larguraTela / 2 - 50, comprimentoTela / 2 - 90, 100, 50 };
            Rectangle mediumButton = { larguraTela / 2 - 50, comprimentoTela / 2 - 20, 100, 50 };
            Rectangle hardButton = { larguraTela / 2 - 50, comprimentoTela / 2 + 50, 100, 50 };

            DrawRectangleRec(easyButton, DARKGRAY);
            DrawRectangleRec(mediumButton, DARKGRAY);
            DrawRectangleRec(hardButton, DARKGRAY);

            DrawText("Fácil", larguraTela / 2 - MeasureText("Fácil", 20) / 2, comprimentoTela / 2 - 75, 20, BLACK);
            DrawText("Médio", larguraTela / 2 - MeasureText("Médio", 20) / 2, comprimentoTela / 2 - 5, 20, BLACK);
            DrawText("Difícil", larguraTela / 2 - MeasureText("Difícil", 20) / 2, comprimentoTela / 2 + 65, 20, BLACK);
        }

        if (status == RANKING) {
            DrawText("Ranking", 50, 50, 30, BLACK);
            ExibirRanking(ranking, tamanhoRanking);
            DrawText(home, 55, comprimentoTela - 40, 25, BLACK); // Adicionando instrução para voltar ao menu
        }

        if (status == DERROTA && mostrarTelaFinal) {
            DrawRectangle(0, 0, larguraTela, comprimentoTela, Fade(WHITE, 0.6f));
            DrawText(derrota, larguraTela / 2 - MeasureText(derrota, 30) / 2, comprimentoTela / 2 - 50, 30, BLACK);
            DrawText(reiniciar, larguraTela / 2 - MeasureText(reiniciar, 30) / 2, comprimentoTela * 0.45f, 30, BLACK);
            DrawText(home, larguraTela / 2 - MeasureText(home, 30) / 2 + 10, comprimentoTela * 0.85f, 20, BLACK);

            int minutos = (int)(fimCronometro - inicioCronometro) / 60;
            int segundos = (int)(fimCronometro - inicioCronometro) % 60;
            DrawText(TextFormat("Tempo de jogo: %d minutos, %d segundos.", minutos, segundos), 20, comprimentoTela - 40, 20, RED);
        }

        if (status == VITORIA && mostrarTelaFinal) {
            DrawRectangle(0, 0, larguraTela, comprimentoTela, Fade(WHITE, 0.9f));
            DrawText(vitoria, larguraTela / 2 - MeasureText(vitoria, 30) / 2, comprimentoTela / 2 - 50, 30, GREEN);
            DrawText(reiniciar, larguraTela / 2 - MeasureText(reiniciar, 30) / 2, comprimentoTela * 0.45f, 30, BLACK);

            int minutos = (int)(fimCronometro - inicioCronometro) / 60;
            int segundos = (int)(fimCronometro - inicioCronometro) % 60;
            DrawText(TextFormat("Tempo de jogo: %d minutos, %d segundos.", minutos, segundos), 20, comprimentoTela - 40, 20, RED);

            if(GetKeyPressed()){
                status = INSERIR_NOME;
            }
        }

        if (status == INSERIR_NOME) {
            DrawText("Digite seu nome:", larguraTela / 2 - MeasureText("Digite seu nome:", 20) / 2, comprimentoTela / 2 - 50, 20, BLACK);
            DrawText(nomeJogador, larguraTela / 2 - MeasureText(nomeJogador, 20) / 2, comprimentoTela / 2, 20, BLACK);
        }

        // Garantindo que só vai desenhar a grid quando o status for envolvendo o jogo (JOGANDO, DERROTA e VITORIA)
        if (status == JOGANDO || (status == DERROTA && !mostrarTelaFinal) || (status == VITORIA && !mostrarTelaFinal)) {
            for (int i = 0; i < colunas; i++) {
                for (int j = 0; j < linhas; j++) {
                    DesenharBloco(grid[i][j]);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// Função para desenhar os blocos com bombas e/ou flags e desenhar a grid do jogo
void DesenharBloco(Bloco bloco) {
    if (bloco.revelado) {
        if (bloco.possuiBomba) {
            Rectangle source = { 0, 0, bombaImagem.width, bombaImagem.height };
            Rectangle dest = { bloco.i * larguraBloco, bloco.j * comprimentoBloco, larguraBloco, comprimentoBloco };
            Vector2 origin = { 0, 0 };

            DrawTexturePro(bombaImagem, source, dest, origin, 0.0f, Fade(RED, 0.8f));
        } else {
            DrawRectangle(bloco.i * larguraBloco, bloco.j * comprimentoBloco, larguraBloco, comprimentoBloco, GRAY);
            if (bloco.bombasProximas > 0) {
                DrawText(TextFormat("%d", bloco.bombasProximas), bloco.i * larguraBloco + 8, bloco.j * comprimentoBloco + 8, comprimentoBloco - 8, BLACK);
            }
        }
    } else if (bloco.possuiFlag) {
        Rectangle source = { 0, 0, flagImagem.width, flagImagem.height };
        Rectangle dest = { bloco.i * larguraBloco, bloco.j * comprimentoBloco, larguraBloco, comprimentoBloco };
        Vector2 origin = { 0, 0 };

        DrawTexturePro(flagImagem, source, dest, origin, 0.0f, Fade(LIGHTGRAY, 0.8f));
    }

    DrawRectangleLines(bloco.i * larguraBloco, bloco.j * comprimentoBloco, larguraBloco, comprimentoBloco, BLACK);
}

// Função para verificar se o index que o usuário clicou é válido
bool IndexValido(int i, int j) {
    return i >= 0 && i < colunas && j >= 0 && j < linhas;
}

// Função para que ao clicar no bloco revele se tem ou não uma bomba
void RevelarBloco(int i, int j) {
    if (grid[i][j].possuiFlag || grid[i][j].revelado) {
        return;
    }

    grid[i][j].revelado = true;

    if (grid[i][j].possuiBomba) {
        status = DERROTA;
        fimCronometro = GetTime();

    } else {
        if (grid[i][j].bombasProximas == 0) {
            LimparGrid(i, j);
        }
        blocosRevelados++;

        if (blocosRevelados == linhas * colunas - bombasExistentes) {
            status = VITORIA;
            fimCronometro = GetTime();
            mostrarTelaFinal = true;
        }
    }
}

// Função para colocar ou tirar a flag
void BlocoFlag(int i, int j) {
    if (grid[i][j].revelado) {
        return;
    }

    grid[i][j].possuiFlag = !grid[i][j].possuiFlag;
}

// Função para contar a quantidade de bombas ao redor do bloco
int BlocoBombasProximas(int i, int j) {
    int contagem = 0;
    for (int ioff = -1; ioff <= 1; ioff++) { // ioff representa o bloco da linha anterior, da linha do próprio bloco e a próxima linha
        for (int joff = -1; joff <= 1; joff++) { // joff representa o bloco da linha anterior, da linha do próprio bloco e a próxima linha
            if (ioff == 0 && joff == 0) {
                continue;
            }

            if (!IndexValido(i + ioff, j + joff)) {
                continue;
            }
                        if (grid[i + ioff][j + joff].possuiBomba) {
                contagem++;
            }            
        }
    }
    return contagem;
}

// Inicialização da grid
void IniciarGrid(void) {
    for (int i = 0; i < colunas; i++) {
        for (int j = 0; j < linhas; j++) {
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
    switch (dificuldade) {
        case FACIL:
            bombasExistentes = (int)(linhas * colunas * 0.1f);
            break;
        case MEDIO:
            bombasExistentes = (int)(linhas * colunas * 0.15f);
            break;
        case DIFICIL:
            bombasExistentes = (int)(linhas * colunas * 0.20f);
            break;
    }

    int quantidadeBombas = bombasExistentes;
    while (quantidadeBombas > 0) {
        int i = rand() % colunas;
        int j = rand() % linhas;

        if (!grid[i][j].possuiBomba) {
            grid[i][j].possuiBomba = true;
            quantidadeBombas--;
        }
    }

    // Colocando a quantidade de bombas próximas
    for (int i = 0; i < colunas; i++) {
        for (int j = 0; j < linhas; j++) {
            if (!grid[i][j].possuiBomba) {
                grid[i][j].bombasProximas = BlocoBombasProximas(i, j);
            }
        }
    }
}

// Função para retirar todos os blocos ao redor que possuem o valor 0 e assim não poluir visualmente o jogo
void LimparGrid(int i, int j) {
    for (int ioff = -1; ioff <= 1; ioff++) {
        for (int joff = -1; joff <= 1; joff++) {
            if (ioff == 0 && joff == 0) {
                continue;
            }

            if (!IndexValido(i + ioff, j + joff)) {
                continue;
            }
            if (!grid[i + ioff][j + joff].revelado) {
                RevelarBloco(i + ioff, j + joff);
            }            
        }
    }
}

// Inicializar o jogo
void GameInit() {
    IniciarGrid();
    status = JOGANDO;
    blocosRevelados = 0;
    inicioCronometro = GetTime();
    mostrarTelaFinal = false; // Resetando a variável para a nova partida
    nomeIndex = 0; // Resetando o índice do nome do jogador
    memset(nomeJogador, 0, sizeof(nomeJogador)); // Limpando o nome do jogador
}

void SalvarRanking(Player* ranking, int tamanho) {
    FILE* file = fopen("ranking/ranking.dat", "wb");
    if (file != NULL) {
        fwrite(ranking, sizeof(Player), tamanho, file);
        fclose(file);
    }
}

int CarregarRanking(Player* ranking, int maxTamanho) {
    FILE* file = fopen("ranking/ranking.dat", "rb");
    int tamanho = 0;
    if (file != NULL) {
        tamanho = fread(ranking, sizeof(Player), maxTamanho, file);
        fclose(file);
    }

    int validCount = 0;
    for(int i=0; i < tamanho; i++){
        if(ranking[i].tempo > 0){
            ranking[validCount++] = ranking[i];
        }
    }
    
    return validCount;
}

void AtualizarRanking(Player* ranking, int* tamanho, Player novoJogador) {
    if (novoJogador.tempo <= 0){
        return;
    }
    
    if (*tamanho < MAX_JOGADORES) {
        ranking[*tamanho] = novoJogador;
        (*tamanho)++;
    } else {
        if (ranking[MAX_JOGADORES - 1].tempo > novoJogador.tempo) {
            ranking[MAX_JOGADORES - 1] = novoJogador;
        }
    }

    qsort(ranking, *tamanho, sizeof(Player), comparePlayers);

    if (*tamanho > MAX_JOGADORES) {
        *tamanho = MAX_JOGADORES;
    }

    SalvarRanking(ranking, *tamanho);
}

int comparePlayers(const void* a, const void* b) {
    Player* playerA = (Player*)a;
    Player* playerB = (Player*)b;
    return (playerA->tempo > playerB->tempo) - (playerA->tempo < playerB->tempo);
}

void ExibirRanking(Player* ranking, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        char dificuldade[10];
        switch (ranking[i].dificuldade) {
            case FACIL: strcpy(dificuldade, "Fácil"); break;
            case MEDIO: strcpy(dificuldade, "Médio"); break;
            case DIFICIL: strcpy(dificuldade, "Difícil"); break;
        }
        DrawText(TextFormat("%d. %s - %.2f s - %s", i + 1, ranking[i].nome, ranking[i].tempo, dificuldade), 50, 100 + 30 * i, 20, BLACK);
    }
}

