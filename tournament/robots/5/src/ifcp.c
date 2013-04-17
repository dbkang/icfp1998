#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>

int parity = 1;

#ifndef MONOPROC
#define MPROC
#endif

#ifndef TIMEOUT
#define TIMEOUT 28
#endif

#ifndef MAX_SZ
#define MAX_SZ 20
#endif

/*
 * solve 3x3, 4x4
 *
#undef TIMEOUT
#define  PURE_FORCE
#undef MAX_SZ
#define MAX_SZ 4
*/

int start_time;

#ifdef DEBUG
int debug = 0;
#else
#define debug 0
#endif

int squares[MAX_SZ + 1];

int sz;

typedef struct _bd {
	char b[MAX_SZ][MAX_SZ];
	int cnt;
	struct _bd *prev;
	char mover;
} BOARD;

int initial_pcs = 0;
BOARD *initial;

typedef enum {MT, MB, ML, MR} MOVE;

typedef struct {
	MOVE m;
	int r;
} ORDER;

ORDER order[MAX_SZ*4];

typedef struct _R {
	MOVE m;
	int r;
	int score;
	int alpha;
	int beta;
} RESULT;

typedef struct {
	MOVE startm;
	int startr;
	int exclusivem;
} FORCE;

#define BOTTOM(xx) ((xx) & 0x1)
#define HORIZ(xx) ((xx) & 0x2)

int ALARMED;

void
FINISH()
{
	ALARMED = 1;
}


void
move(BOARD *b, BOARD *o, MOVE m, int r, char piece)
{
	int i;
	char carry;

if (o) { *b = *o; }

	b->mover = piece;

	switch (m) {
	case MT:
		for (i = 0; i < sz; ++i) {
			char *p = &b->b[r][i];
			if (*p) {
				carry = *p;
				*p = piece;
				piece = carry;
			} else {
				*p = piece;
				++b->cnt;
				return;
			}
		}
		return;
	case MB:
		for (i = sz - 1; i >= 0; --i) {
			char *p = &b->b[r][i];
			if (*p) {
				carry = *p;
				*p = piece;
				piece = carry;
			} else {
				*p = piece;
				++b->cnt;
				return;
			}
		}
		return;
	case ML:
		for (i = 0; i < sz; ++i) {
			char *p = &b->b[i][r];
			if (*p) {
				carry = *p;
				*p = piece;
				piece = carry;
			} else {
				*p = piece;
				++b->cnt;
				return;
			}
		}
		return;
	case MR:
		for (i = sz - 1; i >= 0; --i) {
			char *p = &b->b[i][r];
			if (*p) {
				carry = *p;
				*p = piece;
				piece = carry;
			} else {
				*p = piece;
				++b->cnt;
				return;
			}
		}
		return;
	}
}

#define POOLSZ 405

BOARD *
copyb(BOARD *b)
{
	static BOARD *pool;
	BOARD *p;
	int i;

	if (!pool) {
		p = (BOARD *)malloc(sizeof(BOARD) * POOLSZ);
		for (i = POOLSZ; --i;  ++p) {
			p->prev = pool;
			pool = p;
		}
	}
	p = pool;
	pool = p->prev;
	*p = *b;
	return p;
}


int
readb(FILE *fp, BOARD *b)
{
	char where, piece;
	MOVE m;
	BOARD *nnn;
	int r;

	if (fscanf(fp, "%d ", &sz) != 1) {
		fprintf(stderr, "invalid input, unable to read board size\n");
		exit(1);
	}
	memset(b, 0, sizeof(*b));
	b->prev = NULL;
	b->mover = -1;
	
	piece = 1;
	b->cnt = 0;
	while (fscanf(fp, "%c%d ", &where, &r) == 2) {
		if (r < 1 || r > sz) {
			fprintf(stderr, "invalid input: row/col %d\n", r);
			exit(1);
		}
		--r;
		switch(where) {
		case 't': case 'T':	m = MT; break;
		case 'b': case 'B':	m = MB; break;
		case 'l': case 'L':	m = ML; break;
		case 'r': case 'R':	m = MR; break;
		default:
			fprintf(stderr, "invalid input: side %c\n", where);
			exit(1);
		}
		move(b, 0, m, r, piece);
		nnn = copyb(b);
		if (b->cnt == initial_pcs) {
			nnn->prev = initial;
			initial = nnn;
		} else {
			initial_pcs = b->cnt;
			nnn->prev = NULL;
			initial = nnn;
		}
		piece *= -1;
	}
	return piece;
}

printb(FILE *fp, BOARD *b)
{
	int x, y;

	for (y = 0; y < sz; ++y) {
		for (x = 0; x < sz; ++x) {
			switch (b->b[x][y]) {
			case -1:
				putc('O', fp);
				break;
			case 1:
				putc('X', fp);
				break;
			case 0:
				putc('+', fp);
				break;
			}
			putc(' ', fp);
		}
		putc('\n', fp);
	}
	printf("Total: %d\n", b->cnt);
}

int
isdup(BOARD *bd, BOARD *lst)
{
	for (; lst && lst->cnt == bd->cnt; lst = lst->prev)
		if (lst->mover == bd->mover && memcmp(lst->b, bd->b, sizeof(bd->b)) == 0)
			return 1;
	if (bd->cnt != initial_pcs)
		return 0;
	for (lst = initial; lst; lst = lst->prev)
		if (lst->mover == bd->mover && memcmp(lst->b, bd->b, sizeof(bd->b)) == 0)
			return 1;
	return 0;
}

int
gameoverp(BOARD *bd)
{
	int x, y;
	int wins[MAX_SZ];
	int score = 0;

	if (bd->cnt < sz)
		return 0;

	for (x = 0; x < sz; ++x)
		wins[x] = bd->b[x][x];

	for (x = 0; x < sz; ++x) {
		char c = bd->b[x][0];
		if (!c || c != wins[x])
			continue;
		for (y = sz; --y > 0; ) {
			if (bd->b[x][y] != c)
				goto fail;
		}
		score += c;
		fail: ;
	}
	for (y = 0; y < sz; ++y) {
		char c = bd->b[0][y];
		if (!c || c != wins[y])
			continue;
		for (x = sz; --x > 0; ) {
			if (bd->b[x][y] != c)
				goto fail2;
		}
		score += c;
		fail2: ;
	}
	return score;
}

int
score(BOARD *bd, int final, int depth)
{
	int score = 0;
	int cnt[3], *c = cnt + 1;
	int x, y;

	if (final)
		return final * 100000 * parity * (depth + 1);

#if defined(PURE_FORCE)
	return 0;
#endif

	for (y = 0; y < sz; ++y) {
		cnt[0] = cnt[2] = 0;

		switch (sz) {
		case 20:  ++c[bd->b[19][y]];
		case 19:  ++c[bd->b[18][y]];
		case 18:  ++c[bd->b[17][y]];
		case 17:  ++c[bd->b[16][y]];
		case 16:  ++c[bd->b[15][y]];
		case 15:  ++c[bd->b[14][y]];
		case 14:  ++c[bd->b[13][y]];
		case 13:  ++c[bd->b[12][y]];
		case 12:  ++c[bd->b[11][y]];
		case 11:  ++c[bd->b[10][y]];
		case 10:  ++c[bd->b[9][y]];
		case  9:  ++c[bd->b[8][y]];
		case  8:  ++c[bd->b[7][y]];
		case  7:  ++c[bd->b[6][y]];
		case  6:  ++c[bd->b[5][y]];
		case  5:  ++c[bd->b[4][y]];
		case  4:  ++c[bd->b[3][y]];
		case  3:  ++c[bd->b[2][y]];
		case  2:  ++c[bd->b[1][y]];
		case  1:  ++c[bd->b[0][y]];
		}
		if (cnt[2] + cnt[0] == sz)
			score -= (bd->b[0][y] + bd->b[sz-1][y]) * 16;
		score += squares[cnt[2]] - squares[cnt[0]];
	}

	for (x = 0; x < sz; ++x)  {
		register char *p = &bd->b[x][0];

		cnt[0] = cnt[2] = 0;
		switch (sz) {
		case 20:  ++c[p[19]];
		case 19:  ++c[p[18]];
		case 18:  ++c[p[17]];
		case 17:  ++c[p[16]];
		case 16:  ++c[p[15]];
		case 15:  ++c[p[14]];
		case 14:  ++c[p[13]];
		case 13:  ++c[p[12]];
		case 12:  ++c[p[11]];
		case 11:  ++c[p[10]];
		case 10:  ++c[p[9]];
		case  9:  ++c[p[8]];
		case  8:  ++c[p[7]];
		case  7:  ++c[p[6]];
		case  6:  ++c[p[5]];
		case  5:  ++c[p[4]];
		case  4:  ++c[p[3]];
		case  3:  ++c[p[2]];
		case  2:  ++c[p[1]];
		case  1:  ++c[p[0]];
		}
		if (cnt[2] + cnt[0] == sz)
			score -= (bd->b[x][0] + bd->b[x][sz-1]) * 16;
		score += squares[cnt[2]] - squares[cnt[0]];
	}

	return score * parity;
}

typedef enum {PMIN, PMAX} PLY;

void
findmove(BOARD *b, int depth, PLY ply, RESULT *res, int force)
{
	BOARD b2;
	RESULT child;
	char piece = b->mover * -1;
	int final;
	MOVE m;
	int i, r;

	final = gameoverp(b);

	if (!depth || final) {
		res->score = score(b, final, depth);
		return;
	}

	if (ply == PMAX)
		res->score = res->alpha;
	else
		res->score = res->beta;

	res->m = MT;
	res->r = 0;

	for (i = 0; i < sz * 4; ++i) {
		m = order[i].m;
		if (force >= 0 && m != force)
			continue;
		r = order[i].r;

		/* don't check L1, Ln, R1, Rn if those spaces are
		 * empty, because the equivalent top/bottom moves
		 * will cover it.
		 */
		if (HORIZ(m) && (r == 0 || r == sz-1) &&
			b->b[m == ML ? 0 : sz-1][r] == 0)
			continue;

		move(&b2, b, m, r, piece);
		b2.prev = b;
		if (b2.cnt == b->cnt && isdup(&b2, b))
			continue;

		child.alpha = res->alpha;
		child.beta = res->beta;
		findmove(&b2, depth - 1, 1 - ply, &child, -1);

		if (ply == PMAX) {
			if (child.score > res->alpha) {
				res->score = res->alpha = child.score;
				res->m = m;
				res->r = r;
			}
			if (res->alpha >= res->beta)
				return;
		} else {
			if (child.score < res->beta) {
				res->score = res->beta = child.score;
				res->m = m;
				res->r = r;
			}
			if (res->beta <= res->alpha)
				return;
		}
		if (ALARMED)
			return;
	}
	return;
}

void
preferorder(MOVE m, int r)
{
	int i;

	for (i = 0; i < sz * 4; ++i)
		if (order[i].r == r && order[i].m == m)
			break;
	if (i == 0)
		return;

	for (; i > 0; --i)
		order[i] = order[i - 1];

	order[0].m = m;
	order[0].r = r;
}

void
preporder(BOARD *b)
{
	int i, j;
	int o[MAX_SZ];
	MOVE mo[4];
	int m;

	for (i = j = 0; i < sz; i += 2, ++j)
		o[i] = j;
	for (i = 1, j = sz-1; i < sz; i += 2, --j)
		o[i] = j;
	
	mo[0] = MT;
	mo[1] = ML;
	mo[2] = MB;
	mo[3] = MR;

	for (i = j = 0; i < sz; ++i)
		for (m = 0; m < 4; ++m) {
			order[j].m = mo[m];
			order[j].r = o[i];
			++j;
		}
}

void
driver2(BOARD *root, int piece, int depth)
{
	int i;
	char mout = 'T';
	int rout = '1';
	RESULT res;
	int sout;

#ifdef TIMEOUT
	signal(SIGALRM, FINISH);
	alarm(TIMEOUT);
	start_time = time(0);
#endif

	parity = root->mover * -1;
	for (i = depth < 2 ? depth : 2; i <= depth
#ifdef TIMEOUT
	&& !ALARMED && (time(0)-start_time) < TIMEOUT/2
#endif
		; ++i) {
		res.alpha = INT_MIN;
		res.beta = INT_MAX;
		findmove(root, i, PMAX, &res, -1);
		if (!ALARMED) {
			preferorder(res.m, res.r);
			mout = "TBLR"[res.m];
			rout = res.r + 1;
			sout = res.score;
			if (debug)
				fprintf(stderr, "depth %d at %ds\n", 
					i, time(0)-start_time);
		}
	}
	if (debug)
		fprintf(stderr, "%c%d, score %d\n", mout, rout, sout);

	printf("%c%d\n", mout, rout);
	exit(0);
}

void
driver(BOARD *root, int piece, int depth)
{
	int p[2];
	int ret[4];
	int i;
	int force;
	RESULT res;
	int dout;
	int sout;
	char mout = 'T';
	int rout = '1';
	int first = 1;

	for (i = 0; i < 4; ++i) {
		if (-1 == pipe(p)) {
			perror("pipe");
			exit(1);
		}
		switch(fork()) {
		case -1:
			perror("fork");
			exit(1);
		default:
			close(p[1]);
			ret[i] = p[0];
			break;
		case 0:
			close(1);
			close(p[0]);
			dup2(p[1], 1);
			close(p[1]);
#ifdef TIMEOUT
			signal(SIGALRM, FINISH);
			alarm(TIMEOUT);
			start_time = time(0);
#endif

			force = i;
#ifdef TIMEOUT
			/* always even */
			if (depth > 4) {
				if (sz < 4)
					i = 10;
				else if (sz < 5)
					i = 7;
				else if (sz < 10)
					i = 6;
				else if (sz < 12)
					i = 5;
				else
					i = 4;
				
				if (i > depth)
					i = depth & ~1U;
			} else
#endif
				i = depth & ~1;

			parity = root->mover * -1;
			res.m = MT;
			res.r = 0;
			for (; i <= depth
#ifdef TIMEOUT
					  && !ALARMED
					  && (time(0)-start_time) < TIMEOUT/2
#endif
					  ; ++i) {
				res.alpha = INT_MIN;
				res.beta = INT_MAX;
				findmove(root, i, PMAX, &res, force);
				if (!ALARMED || first) {
					preferorder(res.m, res.r);
					first = 0;
					mout = "TBLR"[res.m];
					rout = res.r + 1;
					sout = res.score;
					dout = i;
					if (debug)
						fprintf(stderr,
						"%c side depth %d at %ds\n",
						"TBLR"[force], i,
						time(0)-start_time);
				}
			}
			if (debug)
				fprintf(stderr, "%c%d, score %d\n",
					mout, rout, sout);
			printf("%c%d\n%d\n%d\n", mout, rout, sout, dout);
			exit(0);
		}
	}
	sout = INT_MIN;
	for (i = 0; i < 4; ++i) {
		FILE *fp = fdopen(ret[i], "r");
		char mmm = 'T';
		int r = 1, d, s = INT_MIN;

		if (fscanf(fp, "%c%d %d %d", &mmm, &r, &s, &d)==4 && s > sout) {
			mout = mmm;
			rout = r;
			sout = s;
		}
		fclose(fp);
	}
	printf("%c%d\n", mout, rout);
	exit(0);
}

main(int ac, char **av)
{
	BOARD b, *nnn;
	int r;
	MOVE m;
	char piece;
	int final = 0;

	srandom(time(0));

	piece = readb(stdin, &b);

	squares[0] = -5;
	for (r = 1; r <= sz; ++r) {
		squares[r] = r*r*r - 1;
	}
	preporder(&b);

	b.prev = NULL;

	/* I just solved 3x3 totally to see what it liked for an
	 * opening, it can win starting with any of the 4 corners.
	 */
	if (b.cnt == 0) {
		printf("%c%d\n", "TBLR"[random()&3],
			random()&1 ? 1 : sz);
		exit(0);
	}

#ifdef DEBUG
	debug = getenv("DEBUG") != NULL;
#endif
#ifndef MPROC
	driver2(&b, piece, 999);
#else
	driver(&b, piece, 999);
#endif

#ifdef TESTDRIVER
	printb(stdout, &b);
	while (!final) {
		RESULT res;

		res.alpha = INT_MIN;
		res.beta = INT_MAX;
		parity = piece;
		findmove(&b, piece < 0 ? 8 : 4, PMAX, &res, -1);
		preferorder(res.m, res.r);
		move(&b, 0, res.m, res.r, piece);
		printf("%s plays %c%d (score %d):\n", piece < 0 ? "White":"Black", "TBLR"[res.m], res.r+1, res.score);

		nnn = copyb(&b);
		if (nnn->cnt == initial_pcs) {
			nnn->prev = initial;
			initial = nnn;
		} else {
			initial_pcs = nnn->cnt;
			nnn->prev = NULL;
			initial = nnn;
		}
		printb(stdout, &b);
		piece *= -1;
		final = gameoverp(&b);
	}
	exit(0);
#endif
}
