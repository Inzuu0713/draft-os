/* client.c - connects to the server and plays as BLACK */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/board.h"

#define BUF 4096

static int parse_move(const char *input,
                      int *from_row, int *from_col,
                      int *to_row,   int *to_col)
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

int main(void)
{
    printf("\n  5x6 Chess Client\n");

    /* get IP */
    printf("  Enter the server's IP address: ");
    fflush(stdout);
    char ip[64];
    if (!fgets(ip, sizeof(ip), stdin)) return 1;
    ip[strcspn(ip, "\n")] = '\0';

    /* get port */
    printf("  Enter port number (default 12345): ");
    fflush(stdout);
    char port_str[16];
    if (!fgets(port_str, sizeof(port_str), stdin)) return 1;
    int port = atoi(port_str);
    if (port <= 1024 || port > 65535) {
        printf("  Invalid port. Using default 12345.\n");
        port = 12345;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket failed"); return 1; }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        printf("  Invalid IP address.\n");
        return 1;
    }

    printf("  Connecting to %s on port %d...\n", ip, port);
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("  Connection failed");
        return 1;
    }
    printf("  Connected!\n\n");

    /* local game state so client can update board immediately */
    GameState game;
    board_init(&game);

    char buf[BUF];
    char input[64];

    while (1) {
        int n = recv(sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            printf("\n  Disconnected from server.\n");
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);
        fflush(stdout);

        if (strstr(buf, "wins!")) break;

        if (strstr(buf, "YOUR turn (BLACK). Enter move:")) {
            while (1) {
                if (!fgets(input, sizeof(input), stdin)) {
                    close(sock);
                    return 0;
                }

                int from_row, from_col, to_row, to_col;
                if (!parse_move(input, &from_row, &from_col, &to_row, &to_col)) {
                    printf("  Bad input. Try again (example: c5 c4)\n  > ");
                    fflush(stdout);
                    continue;
                }

                if (!game_apply_move(&game, from_row, from_col, to_row, to_col)) {
                    printf("  Illegal move. Try again.\n  > ");
                    fflush(stdout);
                    continue;
                }

                /* show board instantly after client moves */
                printf("\n  You moved: %c%d %c%d\n",
                       'a' + from_col, 6 - from_row,
                       'a' + to_col,   6 - to_row);
                board_print(game.board);
                printf("  Waiting for WHITE to move...\n");
                fflush(stdout);

                send(sock, input, strlen(input), 0);
                break;
            }
        }
    }

    close(sock);
    return 0;
}
