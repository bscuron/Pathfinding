#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncurses.h>
#include <curses.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#define DELAY 100000

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

typedef struct Cell{
    int y, x, visited, type;
    struct Cell* parent;
} Cell;


int rows, cols;
int quit = 0;

struct winsize getWinsize(int fd);
void initBoard(Cell board[rows][cols]);
void initColorPairs();
void showBoard(Cell board[rows][cols]);
int getRand(int min, int max);
void handleKeys(Cell board[rows][cols]);
void generateMaze(Cell board[rows][cols], Cell start, Cell finish);
int getUnvisitedNeighbors(Cell neighbors[4], Cell board[rows][cols], int y, int x);

int main(void){
    srand(time(NULL));

    struct winsize winsize_ = getWinsize(STDOUT_FILENO);
    rows = winsize_.ws_row;
    cols = winsize_.ws_col;
    printf("Board:\n\trows: %d\n\tcols: %d\n", rows, cols);

    Cell board[rows][cols];
    initBoard(board);
    Cell start = {.x = 1, .y = 1};
    Cell finish = {.x = (cols % 2 == 0) ? cols - 1 : cols - 2, .y = (rows % 2 == 0) ? rows - 1 : rows - 2};
    generateMaze(board, start, finish);

    initscr();
    initColorPairs();
    while(!quit){
        showBoard(board);
        usleep(DELAY);
    }

    return 0;
}

void generateMaze(Cell board[rows][cols], Cell start, Cell finish){
    Cell current;
    current.x = start.x;
    current.y = start.y;
    board[current.y][current.x].visited = 1;
    int slen = 0;
    Cell* stack = malloc(sizeof(Cell) * rows * cols);
    stack[slen++] = current;

    while(slen > 0){
        current = stack[slen-- - 1];

        Cell neighbors[4];
        int nlen = getUnvisitedNeighbors(neighbors, board, current.y, current.x);

        if(nlen > 0){
            stack[slen++] = current;

            Cell neighbor = neighbors[getRand(0, nlen - 1)];
           
            if(neighbor.y < current.y){
                board[current.y - 1][current.x].type = CELL_EMPTY;
            } else if(neighbor.y > current.y){
                board[current.y + 1][current.x].type = CELL_EMPTY;
            } else if(neighbor.x > current.x){
                board[current.y][current.x + 1].type = CELL_EMPTY;
            } else if(neighbor.x < current.x){
                board[current.y][current.x - 1].type = CELL_EMPTY;
            }

            board[neighbor.y][neighbor.x].visited = 1;
            stack[slen++] = neighbor;
        }
    }
    free(stack);
    board[start.y][start.x].type = CELL_START;
    board[finish.y][finish.x].type = CELL_END;
}

int getUnvisitedNeighbors(Cell neighbors[4], Cell board[rows][cols], int y, int x){
    int count = 0;

    if(y > 1 && !board[y - 2][x].visited){
        neighbors[count++] = board[y - 2][x];
    }
    if(y < rows - 2 && !board[y + 2][x].visited){
        neighbors[count++] = board[y + 2][x];
    }
    if(x > 1 && !board[y][x - 2].visited){
        neighbors[count++] = board[y][x - 2];
    }
    if(x < cols - 2 && !board[y][x + 2].visited){
        neighbors[count++] = board[y][x + 2];
    }

    return count;
}

struct winsize getWinsize(int fd){
    struct winsize win;
    ioctl(fd, TIOCGWINSZ, &win);
    return win;
}

void initBoard(Cell board[rows][cols]){
    for(int y = 0; y < rows; y++){
        for(int x = 0; x < cols; x++){
            board[y][x].type = (x % 2 == 0 || y % 2 == 0) ? CELL_WALL : CELL_EMPTY;
            board[y][x].x = x;
            board[y][x].y = y;
            board[y][x].visited = 0;
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
    init_pair(CELL_PATH, COLOR_BLUE, COLOR_BLUE);
    init_pair(CELL_START, COLOR_GREEN, COLOR_GREEN);
    init_pair(CELL_END, COLOR_RED, COLOR_RED);
}

int getRand(int min, int max){
    assert(min <= max);
    return rand() % (max + 1 - min) + min;
}

void showBoard(Cell board[rows][cols]){
    for(int y = 0; y < rows; y++){
        for(int x = 0; x < cols; x++){
            switch(board[y][x].type){
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
