/* game.c - handles turns, applying moves, promotion, and win detection */
#include <string.h>
#include "../include/board.h"

/* tries to apply a move from (from_row, from_col) to (to_row, to_col)
   
   returns 1 if the move went through, 0 if it was rejected */
int game_apply_move(GameState *game,
                    int from_row, int from_col,
                    int to_row,   int to_col)
{
    /* checks if the game is already over? */
    if (game->winner != NONE)
        return 0;

    Piece *src = &game->board[from_row][from_col];

    /* checks if there is a piece */
    if (src->type == EMPTY)
        return 0;

    /* checks if it belongs to the current player? */
    if (src->color != game->turn)
        return 0;

    /* checks if this is a valid move for the piece */
    if (!is_valid_move(game->board, from_row, from_col, to_row, to_col))
        return 0;

    /* apply the move */
    Piece captured = game->board[to_row][to_col];

    game->board[to_row][to_col]     = game->board[from_row][from_col];
    game->board[from_row][from_col] = (Piece){ EMPTY, NONE };

    /* pawn promotion — white promotes on row 0, black on row 5 */
    Piece *dst = &game->board[to_row][to_col];
    if (dst->type == PAWN) {
        int promotion_row = (dst->color == WHITE) ? 0 : ROWS - 1;
        if (to_row == promotion_row)
            dst->type = QUEEN;
    }

    /* win detection — if the captured piece was a king current player wins the game. */
    if (captured.type == KING)
        game->winner = game->turn;

    /* swap turn */
    game->turn = (game->turn == WHITE) ? BLACK : WHITE;

    return 1;
}
