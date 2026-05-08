/* board.h 
coordinates:
     Rows : 0 - 5 (0, 1 black) (4, 5 white)
     Cols : 0 = file a, 4 = file e
*/

#ifndef BOARD_H
#define BOARD_H

/* 5x6 def */
#define ROWS 6
#define COLS 5

/* piece type */
typedef enum {
    EMPTY  = 0,
    PAWN   = 1,
    ROOK   = 2,
    KNIGHT = 3,
    KING   = 4,
    QUEEN  = 5
} PieceType;

/* player piece */
typedef enum {
    NONE  = 0,
    WHITE = 1,
    BLACK = 2
} Color;

/* a single square — piece type and its color */
typedef struct {
    PieceType type;
    Color     color;
} Piece;

/* 5x6 board */
typedef Piece Board[ROWS][COLS];

/* piece from/to move */
typedef struct {
    int from_row, from_col;
    int to_row,   to_col;
} Move;

/* 30 legal moves (fizxed) */
#define MAX_MOVES 30
typedef struct {
    Move moves[MAX_MOVES];
    int  count;
} MoveList;

/* current game update */
typedef struct {
    Board board;
    Color turn;    /* which player   */
    Color winner;  /* who wins */
} GameState;

/* func board.c */
void board_init(GameState *game);
void board_print(const Board board);

/* func moves.c */
MoveList get_valid_moves(const Board board, int row, int col);
int is_valid_move(const Board board, int from_row, int from_col, int to_row,   int to_col);

/* func game.c */
int game_apply_move(GameState *game, int from_row, int from_col, int to_row,   int to_col);
/* returns 1 if legal, 0 if not */

#endif 
