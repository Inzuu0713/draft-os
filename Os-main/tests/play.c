/* play.c - interactive terminal game, two players on one machine

   how to run:
     make play

   how to enter a move:
     type the square of the piece, a space, then the destination
     example: c2 c3  moves whatever is on c2 to c3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/board.h"

/* converts input like "c2 c3" into row and column numbers
   returns 1 if the input was valid, 0 if not */
static int parse_input(const char *input,
                       int *from_row, int *from_col,
                       int *to_row,   int *to_col)
{
    char fc, tc;
    int  fr, tr;

    /* expects: letter + number + space + letter + number, e.g. "c2 c3" */
    if (sscanf(input, " %c%d %c%d", &fc, &fr, &tc, &tr) != 4)
        return 0;

    /* convert file letter to column: a=0, b=1, c=2, d=3, e=4 */
    *from_col = fc - 'a';
    *to_col   = tc - 'a';

    /* convert rank number to row: rank 6 = row 0, rank 1 = row 5 */
    *from_row = 6 - fr;
    *to_row   = 6 - tr;

    /* make sure everything is inside the board */
    if (*from_col < 0 || *from_col >= COLS) return 0;
    if (*to_col   < 0 || *to_col   >= COLS) return 0;
    if (*from_row < 0 || *from_row >= ROWS) return 0;
    if (*to_row   < 0 || *to_row   >= ROWS) return 0;

    return 1;
}

int main(void)
{
    GameState game;
    board_init(&game);

    char input[32];

    printf("\n  ╔══════════════════════════════╗\n");
    printf("  ║       5 x 6  C H E S S      ║\n");
    printf("  ║  Capture the king to win!   ║\n");
    printf("  ╚══════════════════════════════╝\n");
    printf("\n  How to move: type  from  to  (e.g. c2 c3)\n");
    printf("  Uppercase = white, lowercase = black\n");

    while (game.winner == NONE) {

        board_print(game.board);

        printf("  %s's turn.  Enter move: ",
               game.turn == WHITE ? "WHITE" : "BLACK");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        /* type q to quit */
        if (input[0] == 'q' || input[0] == 'Q') {
            printf("\n  Game quit.\n\n");
            break;
        }

        int from_row, from_col, to_row, to_col;
        if (!parse_input(input, &from_row, &from_col, &to_row, &to_col)) {
            printf("  Bad input. Try again (example: c2 c3)\n");
            continue;
        }

        int ok = game_apply_move(&game, from_row, from_col, to_row, to_col);
        if (!ok) {
            printf("  Illegal move. Try again.\n");
        }
    }

    /* print the final board and announce the winner */
    if (game.winner != NONE) {
        board_print(game.board);
        printf("  *** %s wins! The king has been captured! ***\n\n",
               game.winner == WHITE ? "WHITE" : "BLACK");
    }

    return 0;
}