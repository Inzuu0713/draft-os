/* moves.c  —  Movement rules for all pieces
   5×6 custom rules:
     Pawn   — 1 square forward. No en passant. Can capture opposing pieces in standard chess fashion 
     (one square diagonally forward). Cannot capture the king but can be promoted to a queen to do so.
     Rook   — Standard movements (vertically and horizontally straight).
     Knight —  L-shaped, but only one square vertically and horizontally. 
     King   —  Standard chess movements. Cannot be checked or checkmated, 
     but can be captured by any opposing piece except the pawn.
     Queen  — Standard movements. Obtained through promotion only. */
#include "../include/board.h"

/* ── Helpers ── */

static int in_bounds(int row, int col)
{
    return row >= 0 && row < ROWS && col >= 0 && col < COLS;
}

static int is_empty(const Board board, int row, int col)
{
    return board[row][col].type == EMPTY;
}

static int is_enemy(const Board board, int row, int col, Color my_color)
{
    return board[row][col].color != NONE
        && board[row][col].color != my_color;
}

/* Add a destination to the move list if it is a legal
   target (empty or capturable enemy).
   Return 1 if empty square (slide continues),
   return 0 if occupied (slide stops).         */
static int try_add(const Board board, int row, int col,
                   Color my_color, int is_pawn,
                   MoveList *list,
                   int from_row, int from_col)
{
    if (!in_bounds(row, col)) return 0;

    Piece target = board[row][col];

    if (target.type == EMPTY) {
        list->moves[list->count++] = (Move){ from_row, from_col, row, col };
        return 1;   /* square was empty so slider may continue */
    }

    /* Occupied square */
    if (target.color != my_color) {
        /* Enemy piece — can capture UNLESS it is the king and we are a pawn */
        if (!(is_pawn && target.type == KING)) {
            list->moves[list->count++] = (Move){ from_row, from_col, row, col };
        }
    }
    return 0;   /* occupied sO slider must stop regardless */
}

/* ── Pawn moves ── */
static void pawn_moves(const Board board, int row, int col,
                       Color color, MoveList *list)
{
    /* White moves up (row decreases), black moves down (row increases) */
    int dir = (color == WHITE) ? -1 : 1;
    int fwd = row + dir;

    /* Forward one square — only if empty */
    if (in_bounds(fwd, col) && is_empty(board, fwd, col))
        list->moves[list->count++] = (Move){ row, col, fwd, col };

    /* Diagonal captures — enemy piece that is NOT a king */
    int dc;
    for (dc = -1; dc <= 1; dc += 2) {
        int nc = col + dc;
        if (!in_bounds(fwd, nc)) continue;
        Piece target = board[fwd][nc];
        if (target.color != NONE
            && target.color != color
            && target.type  != KING) {
            list->moves[list->count++] = (Move){ row, col, fwd, nc };
        }
    }
}

/* ── Rook moves ── */
static void rook_moves(const Board board, int row, int col,
                       Color color, MoveList *list)
{
    /* Four straight directions: up, down, left, right */
    int dirs[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
    int d, step;

    for (d = 0; d < 4; d++) {
        int dr = dirs[d][0], dc = dirs[d][1];
        for (step = 1; ; step++) {
            int nr = row + dr * step;
            int nc = col + dc * step;
            if (!in_bounds(nr, nc)) break;
            /* try_add returns 0 when square is occupied — stop sliding */
            if (!try_add(board, nr, nc, color, 0, list, row, col)) break;
        }
    }
}

/* ── Knight moves ──*/
/* 5×6 reduced knight */
static void knight_moves(const Board board, int row, int col,
                         Color color, MoveList *list)
{
    int jumps[4][2] = { {-1,-1}, {-1,1}, {1,-1}, {1,1} };
    int j;

    for (j = 0; j < 4; j++) {
        int nr = row + jumps[j][0];
        int nc = col + jumps[j][1];
        if (!in_bounds(nr, nc)) continue;
        /* Knights can land on empty or enemy squares */
        if (is_empty(board, nr, nc) || is_enemy(board, nr, nc, color))
            list->moves[list->count++] = (Move){ row, col, nr, nc };
    }
}

/* ── King moves ── */
static void king_moves(const Board board, int row, int col,
                       Color color, MoveList *list)
{
    int dr, dc;
    for (dr = -1; dr <= 1; dr++) {
        for (dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = row + dr;
            int nc = col + dc;
            if (!in_bounds(nr, nc)) continue;
            if (is_empty(board, nr, nc) || is_enemy(board, nr, nc, color))
                list->moves[list->count++] = (Move){ row, col, nr, nc };
        }
    }
}

/* ── Queen moves ──*/
/* Queen = straight (H or V) and diagonal  */
static void queen_moves(const Board board, int row, int col,
                        Color color, MoveList *list)
{
    /* All 8 directions */
    int dirs[8][2] = {
        {-1, 0}, {1, 0}, {0,-1}, {0, 1},   /* straight */
        {-1,-1}, {-1,1}, {1,-1}, {1, 1}    /* diagonal */
    };
    int d, step;

    for (d = 0; d < 8; d++) {
        int dr = dirs[d][0], dc = dirs[d][1];
        for (step = 1; ; step++) {
            int nr = row + dr * step;
            int nc = col + dc * step;
            if (!in_bounds(nr, nc)) break;
            if (!try_add(board, nr, nc, color, 0, list, row, col)) break;
        }
    }
}

/* get_valid_moves Main entry point. Returns all legal destinations for the piece at (row, col).*/
MoveList get_valid_moves(const Board board, int row, int col)
{
    MoveList list;
    list.count = 0;

    Piece p = board[row][col];
    if (p.type == EMPTY) return list;

    switch (p.type) {
        case PAWN:   pawn_moves  (board, row, col, p.color, &list); break;
        case ROOK:   rook_moves  (board, row, col, p.color, &list); break;
        case KNIGHT: knight_moves(board, row, col, p.color, &list); break;
        case KING:   king_moves  (board, row, col, p.color, &list); break;
        case QUEEN:  queen_moves (board, row, col, p.color, &list); break;
        default: break;
    }

    return list;
}

/* is_valid_move? Returns 1 if moving from (from_row,from_col) to (to_row,to_col) is in the piece's valid move list. */
int is_valid_move(const Board board,
                  int from_row, int from_col,
                  int to_row,   int to_col)
{
    MoveList list = get_valid_moves(board, from_row, from_col);
    int i;
    for (i = 0; i < list.count; i++) {
        if (list.moves[i].to_row == to_row &&
            list.moves[i].to_col == to_col)
            return 1;
    }
    return 0;
}
