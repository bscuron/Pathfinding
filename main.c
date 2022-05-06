#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncurses.h>
#include <curses.h>
#include <time.h>
#include <string.h>
#define DELAY 1000

typedef enum CELL_TYPE{
    CELL_EMPTY = 1,
    CELL_WALL,
    CELL_PATH,
    CELL_START,
    CELL_END,
} CELL_TYPE;

typedef enum DIRECTION{
    NORTH,
    SOUTH,
    EAST,
    WEST,
} DIRECTION;

int rows, cols;

struct winsize getWinsize(int fd);
void initBoard(int board[rows][cols]);
void fillBoard(int board[rows][cols], int value);
void initColorPairs();
void showBoard(int board[rows][cols]);
int getRand(int min, int max);
int* pop(int* stack, int* slen);
int getUnvisitedNeighbors(int board[rows][cols], int* current, int visited[rows * cols][2], int* vlen, int neighbors[4][2]);
void computePath(int board[rows][cols]);

int main(void){
    srand(time(NULL));

    struct winsize win = getWinsize(STDOUT_FILENO);
    rows = win.ws_row;
    cols = win.ws_col;
    printf("Board:\n\trows: %d\n\tcols: %d\n", rows, cols);

    int board[rows][cols];
    initBoard(board);

    initscr();
    initColorPairs();
    curs_set(0);

    while(1){
        showBoard(board);
        usleep(DELAY);
    }

    return 0;
}

void computePath(int board[rows][cols]){
    int explored[rows * cols][2];
    int elen = 0;
    explored[elen][0] = 0;
    /* int queue[rows * cols][2]; */
    /* int qlen = 0; */
}

struct winsize getWinsize(int fd){
    struct winsize win;
    ioctl(fd, TIOCGWINSZ, &win);
    return win;
}

void initBoard(int board[rows][cols]){
    fillBoard(board, CELL_WALL);
}

void fillBoard(int board[rows][cols], int value){
    for(int y = 0; y < rows; y++){
        for(int x = 0; x < cols; x++){
            board[y][x] = value;
        }
    }
}

void initColorPairs(){
    if(!has_colors()){
        endwin();
        fprintf(stderr, "ERROR: your terminal does not have color support\n");
        exit(1);
    }
    start_color();
    init_pair(CELL_EMPTY, COLOR_WHITE, COLOR_WHITE);
    init_pair(CELL_WALL, COLOR_BLACK, COLOR_BLACK);
    init_pair(CELL_PATH, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(CELL_START, COLOR_GREEN, COLOR_GREEN);
    init_pair(CELL_END, COLOR_RED, COLOR_RED);
}


int* pop(int* stack, int* slen){
    int* cell = malloc(sizeof(int) * 2);
    cell[0] = *(stack + (*slen - 1) * 2 + 0);
    cell[1] = *(stack + (*slen - 1) * 2 + 1);
    *slen -= 1;
    return cell;
}

int getRand(int min, int max){
    return rand() % (max + 1 - min) + min;
}

void showBoard(int board[rows][cols]){
    for(int y = 0; y < rows; y++){
        for(int x = 0; x < cols; x++){
            switch(board[y][x]){
                case CELL_EMPTY:
                    attron(COLOR_PAIR(CELL_EMPTY));
                    mvprintw(y, x, "%c", ' ');
                    attroff(COLOR_PAIR(CELL_EMPTY));
                    break;
                case CELL_WALL:
                    attron(COLOR_PAIR(CELL_WALL));
                    mvprintw(y, x, "%c", ' ');
                    attroff(COLOR_PAIR(CELL_WALL));
                    break;
                case CELL_PATH:
                    attron(COLOR_PAIR(CELL_PATH));
                    mvprintw(y, x, "%c", ' ');
                    attroff(COLOR_PAIR(CELL_PATH));
                    break;
                case CELL_START:
                    attron(COLOR_PAIR(CELL_START));
                    mvprintw(y, x, "%c", ' ');
                    attroff(COLOR_PAIR(CELL_START));
                    break;
                case CELL_END:
                    attron(COLOR_PAIR(CELL_END));
                    mvprintw(y, x, "%c", ' ');
                    attroff(COLOR_PAIR(CELL_END));
                    break;
            }
        }
    }
    refresh();
}
