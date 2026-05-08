/* server.c - hosts the 5x6 chess game over a network
   the server is WHITE and moves first.
   the client is BLACK.

    to run the code:
     make server PORT=5000 or ./bin/server 5000
   then tell the client player your IP address so they can connect.
   find your IP with:  hostname -I */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/board.h"

static const char *piece_char(Piece p)
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

static void send_board(int sock, const Board board)
{
    char buf[1024];
    int  pos = 0;
    int  r, c;

    pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");
    for (r = 0; r < ROWS; r++) {
        pos += snprintf(buf + pos, sizeof(buf) - pos, "  %d  ", ROWS - r);
        for (c = 0; c < COLS; c++) {
            pos += snprintf(buf + pos, sizeof(buf) - pos, "%s ", piece_char(board[r][c]));
        }
        pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");
    }
    pos += snprintf(buf + pos, sizeof(buf) - pos, "     a b c d e\n\n");

    send(sock, buf, pos, 0);
}

static void send_msg(int sock, const char *msg)
{
    send(sock, msg, strlen(msg), 0);
}

static int recv_line(int sock, char *buf, int size)
{
    int  total = 0;
    char ch;
    while (total < size - 1) {
        int n = recv(sock, &ch, 1, 0);
        if (n <= 0) return 0;
        if (ch == '\n') break;
        buf[total++] = ch;
    }
    buf[total] = '\0';
    return 1;
}

static int parse_move(const char *input, int *from_row, int *from_col, int *to_row, int *to_col)
{
    char fc, tc;
    int  fr, tr;

    if (sscanf(input, " %c%d %c%d", &fc, &fr, &tc, &tr) != 4)
        return 0;

    *from_col = fc - 'a';
    *to_col   = tc - 'a';
    *from_row = 6 - fr;
    *to_row   = 6 - tr;

    if (*from_col < 0 || *from_col >= COLS) return 0;
    if (*to_col   < 0 || *to_col   >= COLS) return 0;
    if (*from_row < 0 || *from_row >= ROWS) return 0;
    if (*to_row   < 0 || *to_row   >= ROWS) return 0;

    return 1;
}

int main(int argc, char *argv[])
{
    int port = 12345;
    if (argc == 2)
        port = atoi(argv[1]);

    if (port <= 1024 || port > 65535) {
        printf("  Invalid port. Using default 12345.\n");
        port = 12345;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        return 1;
    }

    listen(server_fd, 1);

    printf("\n  5x6 Chess Server\n");
    printf("  Waiting for opponent to connect on port %d...\n", port);
    printf("  (share your IP with the other player — find it with: hostname -I)\n\n");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept failed");
        return 1;
    }

    printf("  Opponent connected! Starting game...\n\n");

    GameState game;
    board_init(&game);

    send_msg(client_fd, "\n  Connected! You are BLACK. Wait for WHITE to move.\n");

    char input[64];

    while (game.winner == NONE) {

        board_print(game.board);

        if (game.turn == WHITE) {
            printf("  YOUR turn (WHITE). Enter move: ");
            fflush(stdout);

            if (!fgets(input, sizeof(input), stdin)) break;

            if (input[0] == 'q' || input[0] == 'Q') {
                send_msg(client_fd, "\n  Server quit the game.\n");
                break;
            }

            int from_row, from_col, to_row, to_col;
            if (!parse_move(input, &from_row, &from_col, &to_row, &to_col)) {
                printf("  Bad input. Try again (example: c2 c3)\n");
                continue;
            }

            if (!game_apply_move(&game, from_row, from_col, to_row, to_col)) {
                printf("  Illegal move. Try again.\n");
                continue;
            }

            char move_msg[64];
            snprintf(move_msg, sizeof(move_msg),
                     "  WHITE moved: %c%d %c%d\n",
                     'a' + from_col, 6 - from_row,
                     'a' + to_col,   6 - to_row);
            send_msg(client_fd, move_msg);
            send_board(client_fd, game.board);

        } else {
            printf("  Waiting for BLACK to move...\n");
            send_msg(client_fd, "  YOUR turn (BLACK). Enter move: ");

            if (!recv_line(client_fd, input, sizeof(input))) {
                printf("  Opponent disconnected.\n");
                break;
            }

            int from_row, from_col, to_row, to_col;
            if (!parse_move(input, &from_row, &from_col, &to_row, &to_col)) {
                send_msg(client_fd, "  Bad input. Try again (example: c2 c3)\n");
                continue;
            }

            if (!game_apply_move(&game, from_row, from_col, to_row, to_col)) {
                send_msg(client_fd, "  Illegal move. Try again.\n");
                continue;
            }

            printf("  BLACK moved: %c%d %c%d\n",
                   'a' + from_col, 6 - from_row,
                   'a' + to_col,   6 - to_row);
        }
    }

    if (game.winner != NONE) {
        board_print(game.board);
        const char *winner = (game.winner == WHITE) ? "WHITE" : "BLACK";
        printf("  *** %s wins! The king has been captured! ***\n\n", winner);

        char msg[64];
        snprintf(msg, sizeof(msg),
                 "\n  *** %s wins! The king has been captured! ***\n\n", winner);
        send_msg(client_fd, msg);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
