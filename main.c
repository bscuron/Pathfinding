#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncurses.h>
#include <assert.h>
#define DELAY 1000

typedef enum CELL_TYPE{
    CELL_EMPTY = 1,
    CELL_WALL,
    CELL_PATH,
    CELL_START,
    CELL_END,
} CELL_TYPE;

int rows, cols;

struct winsize getWinsize(int fd);
void initBoard(int board[rows][cols]);
void fillBoard(int board[rows][cols], int value);
void initColorPairs();
void createMaze(int board[rows][cols]);
void showBoard(int board[rows][cols]);

int main(void){

    struct winsize win = getWinsize(STDOUT_FILENO);
    rows = win.ws_row;
    cols = win.ws_col;

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

struct winsize getWinsize(int fd){
    struct winsize win;
    ioctl(fd, TIOCGWINSZ, &win);
    return win;
}

void initBoard(int board[rows][cols]){
    fillBoard(board, CELL_EMPTY);
    createMaze(board);
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

void createMaze(int board[rows][cols]){

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
