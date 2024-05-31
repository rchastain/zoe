/* Header for the zoe chess engine
 *
 * James Stanley 2011
 */

#ifndef ZOE_H_INC
#define ZOE_H_INC

#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/timeb.h>
#include <sys/time.h>


static int SEARCHDEPTH = 96;

static int depth;

static int nodes;

#define bool int
#define true 1
#define false 0

static bool king_check = false;
static bool analyze_on = false;
static bool mode_edit = false;
static bool endgame_reached = false;

static int   max_moves        = 0;
static int   minutes          = 0;
static int   seconds          = 0;
static int   start_time       = 0;
static int   maxtime          = 0;
static int   stop_time        = 0;
static int time_left_opponent = 0;
static int time_left_computer = 0;
static int newdepth = 0;
static int movecount = 0;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
static int clockspersecond = CLOCKS_PER_SEC;
#define random rand

// portable getline (windows doesn't have it)
/* This code is public domain -- Will Hartung 4/9/09 */

static size_t getline(char **lineptr, size_t *n, FILE *stream)
{
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL)
    {
        return -1;
    }

    if (stream == NULL)
    {
        return -1;
    }

    if (n == NULL)
    {
        return -1;
    }

    bufptr = *lineptr;
    size = *n;
    c = fgetc(stream);

    if (c == EOF)
    {
        return -1;
    }

    if (bufptr == NULL)
    {
        bufptr = malloc(128);

        if (bufptr == NULL)
        {
            return -1;
        }

        size = 128;
    }

    p = bufptr;

    while (c != EOF)
    {
        if ((p - bufptr) > (size - 1))
        {
            size = size + 128;
            bufptr = realloc(bufptr, size);

            if (bufptr == NULL)
            {
                return -1;
            }
        }

        *p++ = c;

        if (c == '\n')
        {
            break;
        }

        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;
    return p - bufptr - 1;
}

#else
static int clockspersecond = CLOCKS_PER_SEC / 1000;
#endif


#define WHITE 0
#define BLACK 1
#define FORCE 2

#define KINGSIDE  0
#define QUEENSIDE 1

#define PAWN     0
#define KNIGHT   1
#define BISHOP   2
#define ROOK     3
#define QUEEN    4
#define KING     5
#define OCCUPIED 6
#define EMPTY    7

#define EXACTLY 0
#define ATLEAST 1
#define ATMOST  2

#define INFINITY (1 << 30)

#define HT_SIZE (1 << 19)

typedef struct Board
{
    uint8_t mailbox[64];
    uint64_t b[2][7];
    uint64_t occupied;
    uint64_t zobrist;
} Board;

typedef struct Game
{
    Board board;
    uint8_t can_castle[2][2];
    uint8_t quiet_moves;
    uint8_t turn;
    uint8_t engine;
    uint8_t ep;
    int eval;
} Game;

typedef struct Move
{
    uint8_t begin;
    uint8_t end;
    uint8_t promote;
} Move;

typedef struct MoveScore
{
    Move move;
    int score;
    Move pv[16];
} MoveScore;

typedef struct HashEntry
{
    uint64_t key;
    uint8_t depth;
    uint8_t type;
    uint8_t colour;
    MoveScore move;
} HashEntry;

/* bitscan.c */
int bsf(uint64_t n);
int bsr(uint64_t n);
int count_ones(uint64_t n);

/* board.c */
extern uint64_t king_moves[64];
extern uint64_t knight_moves[64];

void reset_board(Board *board);
void clear_board(Board *board);
int consistent_board(Board *board);
void draw_board(Board *board);
void draw_bitboard(uint64_t board);
void generate_movetables(void);
uint64_t rook_moves(Board *board, int tile);
uint64_t bishop_moves(Board *board, int tile);
uint64_t pawn_moves(Board *board, int tile);
int is_threatened(Board *board, int tile);
int king_in_check(Board *board, int colour);

/* game.c */
void reset_game(Game *game);

/* hash.c */
extern uint64_t zobrist[8][64];

void init_zobrist(void);
void hash_store(uint64_t key, uint8_t depth, uint8_t type, MoveScore move,
                int colour);
MoveScore hash_retrieve(uint64_t key, uint8_t depth, int alpha, int beta,
                        int colour);

/* move.c */
char *xboard_move(Move m);
int is_xboard_move(const char *move);
Move get_xboard_move(const char *move);
void apply_move(Game *game, Move m);
void generate_movelist(Game *game, Move *moves, int *nmoves);
uint64_t generate_moves(Game *game, int tile);
int is_valid_move(Game game, Move m, int print);
int piece_square_score(int piece, int square, int colour);

/* search.c */
MoveScore alphabeta(Game game, int alpha, int beta, int depth);
Move best_move(Game game);

/* zoe.c */
extern int post;

#endif
