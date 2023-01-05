#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <curses.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define ROWS 20
#define COLS 20
#define SNAKE_START_LENGTH 5

#define TOP_BOUNDARY 0
#define BOTTOM_BOUNDARY ROWS - 1
#define LEFT_BOUNDARY 0
#define RIGHT_BOUNDARY COLS - 1

#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

#define APPLE 'O'
#define SNAKE 'X'

int score = 0;

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void gameOverAnimation() {

    const char *a[7] = {RED, GRN, YEL, BLU, MAG, CYN, WHT};

    for (int i = 0; i < 3; i++) {
        printf("%s", a[0]);
        printf("Game Over!\n"     RESET);
        usleep(400000);
        system("clear");
        usleep(200000);
    }
    

}

struct Apple
{
    int row;
    int col;
};

struct SnakeSegment
{
    int row;
    int col;
    struct SnakeSegment *next;
};

void drawBoard(char board[ROWS][COLS], struct SnakeSegment *snake, struct Apple apple)
{
    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            board[row][col] = ' ';
        }
    }

    for (int col = LEFT_BOUNDARY; col <= RIGHT_BOUNDARY; col++)
    {
        board[TOP_BOUNDARY][col] = '#';
        board[BOTTOM_BOUNDARY][col] = '#';
    }
    for (int row = TOP_BOUNDARY; row <= BOTTOM_BOUNDARY; row++)
    {
        board[row][LEFT_BOUNDARY] = '#';
        board[row][RIGHT_BOUNDARY] = '#';
    }

    // draw snake
    struct SnakeSegment *current = snake;
    while (current != NULL)
    {
        board[current->row][current->col] = SNAKE;
        current = current->next;
    }

    // draw apple
    board[apple.row][apple.col] = APPLE;
}

void moveSnake(char board[ROWS][COLS], struct SnakeSegment **snake, char direction)
{
    // store the current location of the snake's head
    struct SnakeSegment *current = *snake;
    int oldRow = current->row;
    int oldCol = current->col;

    // move head in desired direction
    switch (direction)
    {
    case UP:
        current->row--;
        break;
    case DOWN:
        current->row++;
        break;
    case LEFT:
        current->col--;
        break;
    case RIGHT:
        current->col++;
        break;
    }

    // check if the new location of the snake's head is already occupied by the snake's body
    struct SnakeSegment *temp = *snake;
    temp = temp->next;
    while (temp != NULL)
    {
        if (temp->row == current->row && temp->col == current->col)
        {
            // the new location is already occupied by the snake's body, so end the game
            gameOverAnimation();
            printf(MAG "Game over! You bit yourself!\n");
            printf(MAG "Score: %d\n", score);
            exit(0);
        }
        temp = temp->next;
    }

    // check if the new location of the snake's head is within the bounds of the board
    if (current->row <= TOP_BOUNDARY || current->row >= BOTTOM_BOUNDARY || current->col <= LEFT_BOUNDARY || current->col >= RIGHT_BOUNDARY)
    {
        gameOverAnimation();
        printf(MAG "Game over! You hit a wall!\n");
        printf(MAG "Score: %d\n", score);
        exit(0);
    }

    // add new segment at head
    struct SnakeSegment *new_segment = malloc(sizeof(struct SnakeSegment));
    new_segment->row = current->row;
    new_segment->col = current->col;
    new_segment->next = *snake;
    *snake = new_segment;

    // remove tail segment
    current = *snake;
    while (current->next->next != NULL)
    {
        current = current->next;
    }
    free(current->next);
    current->next = NULL;
}


int main(int argc, char **argv)
{
    char board[ROWS][COLS];
    char direction = DOWN;
    struct SnakeSegment *snake = NULL;
    struct SnakeSegment *current = NULL;

    // create initial apple
    struct Apple apple;
    apple.row = rand() % (ROWS);
    apple.col = rand() % (COLS);

    // create initial snake
    for (int i = SNAKE_START_LENGTH - 1; i >= 0; i--)
    {
        struct SnakeSegment *new_segment = malloc(sizeof(struct SnakeSegment));
        new_segment->row = 8;
        new_segment->col = i + 8;
        new_segment->next = current;
        current = new_segment;
    }

    snake = current;
    current = NULL;

    while (1)
    {
        // check for input
        if (kbhit())
        {
            char c = getchar();
            if (c == UP || c == DOWN || c == LEFT || c == RIGHT)
            {
                direction = c;
            }
        }

        system("clear");

        moveSnake(board, &snake, direction);
        drawBoard(board, snake, apple);

        // check if snake has collided with apple
        if (snake->row == apple.row && snake->col == apple.col)
        {
            score++;

            // add new segment to snake
            struct SnakeSegment *new_segment = malloc(sizeof(struct SnakeSegment));
            new_segment->row = snake->row;
            new_segment->col = snake->col;
            new_segment->next = snake;
            snake = new_segment;

            // move apple to new location
            apple.row = rand() % (ROWS - 2) + 1;
            apple.col = rand() % (COLS - 2) + 1;
        }

        // draw board
        for (int row = 0; row < ROWS; row++)
        {
            for (int col = 0; col < COLS; col++)
            {
                printf("%c ", board[row][col]);
            }
            printf("\n");
        }
        
        printf("Score: %d\n", score);

        usleep(100000);
    }

    printf(YEL "Score: %d\n", score);


    return 0;
}
