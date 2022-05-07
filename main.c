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

const int DELAY_TIMES[10] = {125000, 100000, 75000, 50000, 25000, 15000, 10000, 5000, 2500, 0};
int didx = 5;

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
    int y, x, visited, type, explored, px, py;
} Cell;


int rows, cols;
int quit = 0;
int paused = 0;

struct winsize getWinsize(int fd);
void initBoard(Cell board[rows][cols]);
void initColorPairs();
void showBoard(Cell board[rows][cols]);
int getRand(int min, int max);
void handleKeys(Cell board[rows][cols], Cell** path, int* plen, int* pi);
void generateMaze(Cell board[rows][cols], Cell start, Cell finish);
int getUnvisitedNeighbors(Cell neighbors[4], Cell board[rows][cols], int y, int x);
Cell* bfs(Cell board[rows][cols], Cell start, Cell finish);
int getUnexploredNeighbors(Cell neighbors[4], Cell board[rows][cols], int y, int x);
int getPathLength(Cell* path, Cell start, Cell finish);

int main(void){
    srand(time(NULL));

    struct winsize winsize_ = getWinsize(STDOUT_FILENO);
    rows = winsize_.ws_row;
    cols = winsize_.ws_col;

    Cell board[rows][cols];
    initBoard(board);
    Cell start = {.x = 1, .y = 1, .px = -1, .py = -1};
    Cell finish = {.x = (cols % 2 == 0) ? cols - 1 : cols - 2, .y = (rows % 2 == 0) ? rows - 1 : rows - 2};
    generateMaze(board, start, finish);
    Cell* path = bfs(board, start, finish);
    int plen = getPathLength(path, start, finish);
    int pi = 0;
    
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    initColorPairs();
    while(!quit){
        showBoard(board);
        usleep(DELAY_TIMES[didx]);
        if(!paused && pi < plen){
            if(path[pi].y != start.y || path[pi].x != start.x)
                board[path[pi].y][path[pi].x].type = CELL_PATH;
            pi++;
        }
        handleKeys(board, &path, &plen, &pi);
    }
    free(path);
    return 0;
}

void handleKeys(Cell board[rows][cols], Cell** path, int* plen, int* pi){
    wchar_t code = getch();
    switch(code){
        case -1:
            return;
        case ' ':
            paused = !paused;
            break;
        case KEY_ENTER:
        case '\n':
            paused = 0;
            initBoard(board);
            Cell start = {.x = 1, .y = 1, .px = -1, .py = -1};
            Cell finish = {.x = (cols % 2 == 0) ? cols - 1 : cols - 2, .y = (rows % 2 == 0) ? rows - 1 : rows - 2};
            generateMaze(board, start, finish);
            *path = bfs(board, start, finish);
            *plen = getPathLength(*path, start, finish);
            *pi = 0;
            break;
        case KEY_UP:
            if(++didx > 9) didx = 9;
            break;
        case KEY_DOWN:
            if(--didx < 0) didx = 0;
            break;
        case KEY_LEFT:
            *pi -= *plen / 125;
            if(*pi < 1) *pi = 1;
            for(int i = *pi; i < *plen; i++){
                board[(*path)[i].y][(*path)[i].x].type = CELL_EMPTY;
            }
            break;
        case KEY_RIGHT:
            *pi += *plen / 125;
            if(*pi > *plen - 1) *pi = *plen - 1;
            for(int i = 1; i < *pi; i++){
                board[(*path)[i].y][(*path)[i].x].type = CELL_PATH;
            }
            break;
    }
    int codeDup;
    int dup = 0;
    while((codeDup = getch()) == code) dup = 1;
    if(dup) ungetch(codeDup);
}

int getPathLength(Cell* path, Cell start, Cell finish){
    assert(path[0].x == start.x && path[0].y == start.y);
    int i = 1;
    while(path[i].x != finish.x || path[i].y != finish.y) i++;
    return i;
}

Cell* bfs(Cell board[rows][cols], Cell start, Cell finish){
    Cell* queue = malloc(sizeof(Cell) * rows * cols);
    int qlen = 0;

    board[start.y][start.x].explored = 1;

    queue[qlen++] = start;

    while(qlen > 0){
        Cell current = queue[0];

        if(current.y == finish.y && current.x == finish.x){
            Cell* path = malloc(sizeof(Cell) * rows * cols);
            int plen = 0;
            path[plen++] = current;
            do{
                path[plen++] = board[current.py][current.px];
                current = board[current.py][current.px];
            } while(current.px != start.px || current.py != start.py);

            for(int i = 0; i < plen / 2; i++){
                Cell tmp = path[i];
                path[i] = path[plen - 1 - i];
                path[plen - 1 - i] = tmp;
            }
            free(queue);
            return path;
        }

        for(int i = 0; i < qlen - 1; i++)
            queue[i] = queue[i + 1];
        qlen--;

        Cell neighbors[4];
        int nlen = getUnexploredNeighbors(neighbors, board, current.y, current.x);

        for(int i = 0; i < nlen; i++){
            board[neighbors[i].y][neighbors[i].x].explored = 1;
            neighbors[i].px = current.x;
            neighbors[i].py = current.y;
            board[neighbors[i].y][neighbors[i].x].px = current.x;
            board[neighbors[i].y][neighbors[i].x].py = current.y;
            queue[qlen++] = neighbors[i];
        }
    }
    free(queue);
    return NULL;
}

int getUnexploredNeighbors(Cell neighbors[4], Cell board[rows][cols], int y, int x){
    int count = 0;
    if(y > 0 && !board[y - 1][x].explored && board[y - 1][x].type != CELL_WALL)
        neighbors[count++] = board[y - 1][x];
    if(x > 0 && !board[y][x - 1].explored && board[y][x - 1].type != CELL_WALL)
        neighbors[count++] = board[y][x - 1];
    if(y < rows - 1 && !board[y + 1][x].explored && board[y + 1][x].type != CELL_WALL)
        neighbors[count++] = board[y + 1][x];
    if(x < cols - 1 && !board[y][x + 1].explored && board[y][x + 1].type != CELL_WALL)
        neighbors[count++] = board[y][x + 1];
    return count;
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
            board[y][x].px = -1;
            board[y][x].py = -1;
            board[y][x].visited = 0;
            board[y][x].explored = 0;
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
    mvprintw(rows - 1, 0, "Speed: %d/10, Paused: %s", (didx + 1), paused ? "true" : "false");
    refresh();
}
