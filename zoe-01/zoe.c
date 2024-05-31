/* zoe - the Zoe Opponent Engine
 *
 * An xboard protocol chess engine.
 *
 * James Stanley 2011
 */

#include "zoe.h"

int post = 0;

/* handle the board edit mode */
void edit_mode(Game *g)
{
    static char *piece_letter = "PNBRQK";
    char *line = NULL;
    size_t len = 0;
    int tile;
    uint64_t tilebit;
    int i, j;
    int piece;
    int gameturn = g->turn;
    mode_edit = false;
    /* TODO: "[upon leaving edit mode] for purposes of the draw by repetition
     *        rule, no prior positions are deemed to have occurred."
     */
    g->ep = 9;
    g->quiet_moves = 0;

    if (analyze_on) g->turn = BLACK;

    if (g->turn == BLACK)
    {
        /* make it white's turn so that the evaluation makes sense */
        g->turn = WHITE;
        g->eval = -g->eval;
    }

    printf("turn is %c\n", "WB"[g->turn]);

    /* remove castling rights for each player for each side */
    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++)
            g->can_castle[i][j] = 0;

    while (getline(&line, &len, stdin) != -1)
    {
        printf("# %s\n", line);

        /* strip the endline character */
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        if (strcmp(line, "c") == 0)
        {
            /* switch colour */
            g->turn = !g->turn;
            g->eval = -g->eval;
        }

        else if (strcmp(line, "#") == 0)
        {
            /* clear the board */
            g->eval = 0;
            clear_board(&(g->board));
        }

        else if (strcmp(line, ".") == 0)
        {
            /* leave edit mode */
            free(line);
            break;
        }

        else
        {
            /* change a tile */
            tile = line[1] - 'a' + ((line[2] - '1') * 8);
            tilebit = 1ull << tile;
            /* remove this piece */
            g->eval -= piece_square_score(g->board.mailbox[tile], tile,
                                          !!(g->board.b[WHITE][OCCUPIED] & tilebit));
            g->board.zobrist ^= zobrist[g->board.mailbox[tile]][tile];
            g->board.zobrist ^= zobrist[EMPTY][tile];
            g->board.mailbox[tile] = EMPTY;
            g->board.occupied &= ~tilebit;

            for (i = 0; i < 7; i++)
            {
                g->board.b[BLACK][i] &= ~tilebit;
                g->board.b[WHITE][i] &= ~tilebit;
            }

            /* add a replacement piece if appropriate */
            if (line[0] != 'x')
            {
                if (strchr(piece_letter, line[0]))
                {
                    piece = strchr(piece_letter, line[0]) - piece_letter;
                    g->eval += piece_square_score(piece, tile, g->turn);
                    g->board.zobrist ^= zobrist[EMPTY][tile];
                    g->board.zobrist ^= zobrist[piece][tile];
                    g->board.mailbox[tile] = piece;
                    g->board.occupied |= tilebit;
                    g->board.b[g->turn][piece] |= tilebit;
                    g->board.b[g->turn][OCCUPIED] |= tilebit;
                }

                else
                {
                    /* do what? */
                }
            }
        }
    }

    if (!analyze_on || g->turn == WHITE)
    {
        if (g->turn != gameturn)
        {
            /* toggle the turn back if necessary */
            g->turn = !g->turn;
            g->eval = -g->eval;
        }
    }

    /* allow castling where appropriate */
    if (g->board.b[BLACK][KING] & (1ull << 4))
    {
        if (g->board.b[BLACK][ROOK] & (1ull << 7))
            g->can_castle[BLACK][KINGSIDE] = 1;

        if (g->board.b[BLACK][ROOK] & (1ull))
            g->can_castle[BLACK][QUEENSIDE] = 1;
    }

    if (g->board.b[WHITE][KING] & (1ull << 60))
    {
        if (g->board.b[WHITE][ROOK] & (1ull << 63))
            g->can_castle[WHITE][KINGSIDE] = 1;

        if (g->board.b[WHITE][ROOK] & (1ull << 56))
            g->can_castle[WHITE][QUEENSIDE] = 1;
    }
}

int main(int argc, char **argv)
{
    Game game;
    char *line = NULL;
    char *s;
    size_t len = 0;

    /* don't quit when xboard sends SIGINT */
    if (!isatty(STDIN_FILENO))
        signal(SIGINT, SIG_IGN);

    /* force line buffering on stdin and stdout */
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    /* build zobrist tables */
    init_zobrist();
    /* build tables for move generation */
    generate_movetables();
    /* setup the initial game state */
    reset_game(&game);

    /* repeatedly handle commands from xboard */
    while (getline(&line, &len, stdin) != -1)
    {
        printf("# %s", line);

        /* strip the endline character */
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        if (strcmp(line, "protover 2") == 0 || strcmp(line, "protover") == 0 )
        {
            printf("feature colors=1 time=1 sigint=0 sigterm=0 reuse=0 myname=\"Zoe 0.1 by James Stanley\" analyze=1 done=1\n");
        }

        else if (mode_edit && strcmp(line, "a2a3") == 0)
        {
            analyze_on = true;
        }

        else if (strcmp(line, "analyze") == 0)
        {
            analyze_on = true;
            maxtime = INFINITY;
            /* find the best move */
            Move m = best_move(game);
        }

        else if (strcmp(line, "exit") == 0)
        {
            analyze_on = false;
        }

        else if (strcmp(line, "new") == 0)
        {
            /* start a new game */
            reset_game(&game);
        }

        else if (strcmp(line, "force") == 0)
        {
            /* enter force mode where we just ensure that moves are valid */
            game.engine = FORCE;
        }

        else if (strcmp(line, "go") == 0)
        {
            /* the engine becomes the player currently on move */
            game.engine = game.turn;
        }

        else if (strcmp(line, "post") == 0)
        {
            /* turn on thinking output */
            post = 1;
        }

        else if (strcmp(line, "nopost") == 0)
        {
            /* turn off thinking output */
            post = 0;
        }

        else if ((s = strstr(line, "time")))
        {
            time_left_computer = atoi(s + 5);
            maxtime = (clockspersecond / 100) * (time_left_computer / 100);
        }

        else if ((s = strstr(line, "otim")))
        {
            time_left_opponent = atoi(s + 5);
        }

        else if ((s = strstr(line, "level")))
        {
            max_moves = atoi(s + 6);
        }

        else if ((s = strstr(line, "st")))
        {
            maxtime = atoi(s + 3);
            maxtime *= 1000;
        }

        else if ((s = strstr(line, "sd")))
        {
            SEARCHDEPTH = atoi(s + 3);
            maxtime = 1 << 25;
            printf("\ndepth set to %d\n", SEARCHDEPTH);
        }

        else if (strcmp(line, "edit") == 0)
        {
            /* enter edit mode */
            mode_edit = true;
            edit_mode(&game);
        }

        else if (strcmp(line, "quit") == 0)
        {
            printf("# Be seeing you...\n");
            exit(0);
        }

        else if (strcmp(line, "xboard") == 0)
        {
            printf("feature done=0\n");
        }

        else if (strcmp(line, "accepted") == 0)
        {
            ;
        }

        else if (strcmp(line, "rejected") == 0)
        {
            ;
        }

        else if (strcmp(line, ".") == 0)
        {
            ;
        }

        else if (is_xboard_move(line))
        {
            Move m = get_xboard_move(line);

            /* validate and apply the move */
            if (is_valid_move(game, m, 1))
            {
                apply_move(&game, m);
                /* give game information */
                draw_board(&(game.board));
                printf("# current eval = %d\n", game.eval);
            }
        }

        /* play a move if it is now our turn */
        if (game.turn == game.engine)
        {
            /* find the best move */
            Move m = best_move(game);

            /* only do anything if we have a legal move */
            if (m.begin != 64)
            {
                apply_move(&game, m);
                /* give game information */
                draw_board(&(game.board));
                printf("# current eval = %d\n", -game.eval);
                /* tell xboard about our move */
                printf("move %s\n", xboard_move(m));
                printf("# ! move %s\n", xboard_move(m));
                movecount++;

                if (movecount > 50) endgame_reached = true;

                /* claim victory or draw if our opponent has no response */
                MoveScore response = alphabeta(game, -INFINITY, INFINITY, 1);
                printf("best response score = %d (move = %s)\n", response.score, xboard_move(response.move));

                if (response.move.begin == 64)
                {
                    if (response.score == 0)
                        printf("1/2-1/2 {Stalemate}\n");

                    else   /* response.score == -INFINITY */
                    {
                        printf("computer mates\n");

                        if (game.turn == WHITE)
                            printf("0-1 {Black mates}\n");

                        else
                            printf("1-0 {White mates}\n");
                    }
                }
            }
        }
    }

    return 0;
}



//////////////////// search.c  ////////////////////////


/* sort the list of moves to put the ones most likely to be good first */
static void sort_moves(Move *moves, int nmoves, Game *game)
{
    MoveScore table = hash_retrieve(game->board.zobrist, 0, -INFINITY,
                                    INFINITY, game->turn);
    int firstcap = 0;
    int firstnot = 0;
    Move tmp;

    while (1)
    {
        /* look for the next non-capture move */
        while ((game->board.occupied & (1ull << moves[firstnot].end))
                && firstnot < nmoves)
            firstnot++;

        if (firstnot >= nmoves)
            break;

        /* look for the next capture move */
        while (!(game->board.occupied & (1ull << moves[firstcap].end))
                && firstcap < nmoves)
            firstcap++;

        if (firstcap >= nmoves)
            break;

        /* swap the capture with the non-capture */
        tmp = moves[firstcap];
        moves[firstcap] = moves[firstnot];
        moves[firstnot] = tmp;
        firstnot++;
    }
}

/* return the best move from the current position along with it's score */
MoveScore alphabeta(Game game, int alpha, int beta, int depth)
{
    Move moves[121];/* 121 moves is enough for anybody */
    int nmoves;
    int move;
    Move m;
    MoveScore best, new;
    Game orig_game;
    int legal_move = 0;
    int hashtype = ATMOST;
    int i;
    clock_t start = clock();
    nodes = 0;
    nodes++;
    /* store a copy of the game */
    orig_game = game;
    /* try to retrieve the score from the transposition table */
    new = hash_retrieve(orig_game.board.zobrist, depth, alpha, beta,
                        orig_game.turn);

    if (new.move.begin != 64)
    {
        /* TODO: ensure that the move is valid (i.e. that this zobrist key is
         * not just a coincidence).
         */
        return new;
    }

    /* store lower bound on best score */
    if (newdepth > 5 && depth < newdepth - 1)
        best.score = alpha;

    else
        best.score = -INFINITY;

    /* if at a leaf node, return position evaluation */
    if (depth <= 0)
    {
        best.move.begin = 64;
        best.score = game.eval;
        best.pv[0].begin = 64;
        hash_store(orig_game.board.zobrist, depth, EXACTLY, best,
                   orig_game.turn);
        return best;
    }

    /* get a list of valid moves */
    generate_movelist(&game, moves, &nmoves);
    /* sort the moves */
    sort_moves(moves, nmoves, &game);

    /* for each of the moves */
    for (move = 0; move < nmoves; move++)
    {
        m = moves[move];
        /* reset the game state */
        game = orig_game;
        /* make the move */
        apply_move(&game, m);

        /* don't search this move if the king is left in check */

        if (depth > newdepth)
        {
            if (king_in_check(&(game.board), !game.turn))
            {
                king_check = true;
                //   printf("%s leaves the king in check\n", xboard_move(m));
                continue;
            }
        }

        /* if this is the first legal move, store it as the best so that
         * we at least have a move to play, and remember that we have found
         * a legal move.
         */
        if (!legal_move)
        {
            best.move = m;
            legal_move = 1;
        }

        /* search the next level; we need to do a full search from the top
         * level in order to get the pv for each move.
         */

        if (depth == SEARCHDEPTH)
            new = alphabeta(game, -INFINITY, INFINITY, depth - 1);

        else
            new = alphabeta(game, -beta, -best.score, depth - 1);

        new.score = -new.score;

        /* show the expected line of play from this move at top level */

        if (king_check)
        {
            printf("%2i %i %.2f %i ", depth, new.score, (float) (CLOCKS_PER_SEC) / (clock() - start), nodes);
            //	printf("%s: ", xboard_move(m));
            printf("# pv: ");

            for (i = 0; i < 16 && new.pv[i].begin < 64; i++)
            {
                printf("%s ", xboard_move(new.pv[i]));
            }

            printf("%i\n", new.score);
            king_check = false;
        }

        /* beta cut-off */
        if (new.score >= beta)
        {
            best.move = m;
            best.score = beta;
            hash_store(orig_game.board.zobrist, depth, ATLEAST, best,
                       orig_game.turn);
            return best;
        }

        /* new best move? */
        if (new.score > best.score)
        {
            /* best movescore has the move we should play and the score we
             * got from the child alphabeta.
             */
            best.move = m;
            best.score = new.score;
            /* we know the score for this node is exactly best.score */
            hashtype = EXACTLY;

            /* copy the pv from the best move */
            for (i = 0; i < 15; i++)
            {
                best.pv[i + 1] = new.pv[i];
            }

            best.pv[0] = m;
        }
    }

    /* reset to original game */
    game = orig_game;

    /* no legal moves? checkmate or stalemate */
    if (!legal_move)
    {
        /* adding (depth - SEARCHDEPTH) ensures that we drag out a forced
         * loss for as long as possible, and also that we force a win as
         * quickly as possible.
         */
        if (king_in_check(&(game.board), game.turn))
            best.score = -INFINITY + (depth - newdepth);//(depth - SEARCHDEPTH);

        else
            best.score = 0;

        best.move.begin = 64;
    }

    else
    {
        /* we found a legal move and more searching was done, so we have a
         * lower bound on the score.
         */
        hash_store(orig_game.board.zobrist, depth, hashtype, best,
                   orig_game.turn);
    }

    /* TODO: deal with post mode */

    /* show the pv */

    if (depth > newdepth)
    {
        printf("%2i %i %.2f %i ", depth, best.score, (float) (CLOCKS_PER_SEC) / (clock() - start), nodes);
//printf("%s: ", xboard_move(m));
        printf("# pv: ");

        for (i = 0; i < 16 && best.pv[i].begin < 64; i++)
        {
            printf("%s ", xboard_move(best.pv[i]));
        }

        printf("%i\n", best.score);
    }

    return best;
}


/* return the best move from the current position along with it's score */
MoveScore iterative_deepening(Game game)
{
    int d;
    MoveScore best;

    /* iteratively deepen until the maximum depth is reached */
    for (d = 1; d <= SEARCHDEPTH; d++)
    {
        best = alphabeta(game, -INFINITY, INFINITY, d);

        /* if we have no legal moves, return now */
        if (best.move.begin == 64)
            return best;

        /* if this is a mate, return now */
        if (best.score == INFINITY)
        {
            printf("\ncomputer mates\n");
            printf("# Mate in %d.\n", d / 2);
            return best;
        }

        int elasped_clock = clock();

        if (elasped_clock > stop_time)
        {
            printf("\nallotted search time reached - terminate search.\n");
            break;
        }

        newdepth = d;
        /* TODO: stop searching if the entire tree is searched */
        /* TODO: stop searching if time limit reached */
    }

    return best;
}

/* return the best move for the current player */
Move best_move(Game game)
{
    clock_t start = clock();
    start_time = clock();
    printf("\nstart time = %d\n", start_time);
    stop_time = start_time + maxtime;
    printf("stop time = %d\n", stop_time);
    nodes = 0;
    MoveScore best = iterative_deepening(game);
    printf("# %.2f n/s\n", (float)(nodes * CLOCKS_PER_SEC) / (clock() - start));

    if (best.move.begin == 64)   /* we had no legal moves */
    {
        if (best.score == 0)
            printf("1/2-1/2 {Stalemate}\n");

        else   /* best.score == -INFINITY */
        {
            if (game.turn == WHITE)
                printf("0-1 {Black mates}\n");

            else
                printf("1-0 {White mates}\n");
        }
    }

    return best.move;
}
