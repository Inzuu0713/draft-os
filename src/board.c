/* board.c  board setup and display */
#include <stdio.h>
#include <string.h>
#include "../include/board.h"

/* board_init  */
void board_init(GameState *game)
{
    int r, c;

    /* clear board */
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++) {
            game->board[r][c].type  = EMPTY;
            game->board[r][c].color = NONE;
        }

    /* ── Black row 0 rank 6 */
    game->board[0][0] = (Piece){ ROOK,   BLACK };
    game->board[0][1] = (Piece){ KNIGHT, BLACK };
    game->board[0][2] = (Piece){ KING,   BLACK };
    game->board[0][3] = (Piece){ KNIGHT, BLACK };
    game->board[0][4] = (Piece){ ROOK,   BLACK };

    /* ── Black pawns (row 1 rank 5)── */
    for (c = 0; c < COLS; c++)
        game->board[1][c] = (Piece){ PAWN, BLACK };



    /* ── White pawns (row 4 rank 2) ── */
    for (c = 0; c < COLS; c++)
        game->board[4][c] = (Piece){ PAWN, WHITE };

    /* ── White  (row 5 rank 1) ── */
    game->board[5][0] = (Piece){ ROOK,   WHITE };
    game->board[5][1] = (Piece){ KNIGHT, WHITE };
    game->board[5][2] = (Piece){ KING,   WHITE };
    game->board[5][3] = (Piece){ KNIGHT, WHITE };
    game->board[5][4] = (Piece){ ROOK,   WHITE };

    game->turn   = WHITE;
    game->winner = NONE;
}

/* piece_char */
static const char* piece_char(Piece p) /*CHANGES HERE*/
{
    if (p.type == EMPTY) return "·";

    if (p.color == WHITE) {
        switch (p.type) {
            case PAWN:   return "♟";
            case ROOK:   return "♜";
            case KNIGHT: return "♞";
            case KING:   return "♚";
            case QUEEN:  return "♛";
            default:     return "·";
        }
    } else {
        switch (p.type) {
            case PAWN:   return "♙";
            case ROOK:   return "♖";
            case KNIGHT: return "♘";
            case KING:   return "♔";
            case QUEEN:  return "♕";
            default:     return "·";
        }
    }
}

/* ── board_print */
void board_print(const Board board) /*CHANGES HERE*/
{
    int r, c;
    printf("\n");
    for (r = 0; r < ROWS; r++) {
        printf("  %d  ", ROWS - r);   /* rank label: 6 down to 1 */
        for (c = 0; c < COLS; c++) {
            printf("%s ", piece_char(board[r][c]));
        }
        printf("\n");
    }
    printf("     a b c d e\n\n");
}
