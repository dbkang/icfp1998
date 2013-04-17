#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#ifdef TEST
#define DPRINTF(s) printf s
#define MAXLVL 9
#else
#define DPRINTF(s)
#define MAXLVL 1000
#endif

#define SPACE 0
#define X 1
#define O 2
#define OTHER(n) ((n) ^ X ^ O)

#define TOCHAR(m) ((m) == SPACE ? ' ' : (m) == X ? 'X' : 'O')

#define INF 2000000000

#define WIN 1000000
#define WINL (WIN - 10000)
#define WINNER(p) ((p) >= WINL)

#define bsize BSIZE
#define bsize2 (bsize * bsize)
#define bsizet2 (bsize * 2)
#define boardsize bsize2
#define	bsizem1 (bsize - 1)
#define	bsize4 (bsize * 4)
#define	bsize2b (bsize2 - bsize)
#define	ssize (bsize * sizeof(struct line))

struct line {
	int xscore, oscore;
	int xfull, ofull;
};
struct config {
	unsigned char board[boardsize];
	struct line rowcols[bsizet2];
	int xscore, oscore;
	int xfull, ofull;
	int xtot, otot;
};
#define ROW(bp,i) ((bp)->rowcols[i])
#define COL(bp,i) ((bp)->rowcols[i+bsize])

static __inline int 
getpos(char *p, int i)
{
	return p[i];
}

static __inline void
setpos(char *p, int i, int v)
{
	p[i] = v;
}

struct config *hist;
int hno, hmax;
#define undomove() hno--

void
printboard(struct config *bp)
{
	int i, j, c;

	fprintf(stderr, "                  +");
	for (i = 0; i < bsize; i++)
		fprintf(stderr, "-");
	fprintf(stderr, "+\n");
	for (i = 0; i < bsize; i++) {
		fprintf(stderr, "%2d: %4d:%d/%4d:%d |", i+1, 
			ROW(bp,i).xscore, ROW(bp,i).xfull, 
			ROW(bp,i).oscore, ROW(bp,i).ofull);
		for (j = 0; j < bsize; j++) {
			c = getpos(bp->board, i*bsize+j);
			fprintf(stderr, "%c", TOCHAR(c));
		}
		fprintf(stderr, "|\n");
	}
	fprintf(stderr, "                  +");
	for (i = 0; i < bsize; i++)
		fprintf(stderr, "-");
	fprintf(stderr, "+\n");
	fprintf(stderr, "                   ");
	for (i = 0; i < bsize; i++)
		fprintf(stderr, "%d", (i+1)%10);
	fprintf(stderr, "\n\n");
	for (i = 0; i < bsize; i++) {
		fprintf(stderr, "%2d: %4d:%d/%4d:%d\n", i+1, 
			COL(bp, i).xscore, COL(bp, i).xfull, 
			COL(bp,i).oscore, COL(bp, i).ofull);
	}
	fprintf(stderr, "score x=%d:%d o=%d:%d\n",
		bp->xscore, bp->xfull,
		bp->oscore, bp->ofull);
}

char changed[bsize];

#define TLOOP i = pos; i < bsize2; i += bsize
#define BLOOP i = bsize2b + pos; i >= 0; i -= bsize
#define LLOOP i = pos * bsize, s = i + bsize; i < s; i++
#define RLOOP s = pos * bsize, i = s + bsizem1; i >= s; i--

int sumsize;

#define AMP(x) ((x)*(x))

#define COMPUTE(loop, offs) \
	int i, s, x, o, cur, ix, io; \
	char *b = cp->board; \
	struct line *l = &cp->rowcols[pos+offs]; \
	x = o = ix = io = 0; \
	for (loop) { \
		cur = getpos(b, i); \
		if (cur == X) { \
			io = 0; \
		        ix++; \
			x += AMP(ix); \
		} else if (cur == O) { \
			ix = 0; \
			io++; \
			o += AMP(io); \
		} \
	} \
	cp->xfull -= l->xfull; \
	cp->ofull -= l->ofull; \
	l->xfull = x == sumsize; \
	l->ofull = o == sumsize; \
	cp->xfull += l->xfull; \
	cp->ofull += l->ofull; \
	cp->xscore -= l->xscore; \
	cp->oscore -= l->oscore; \
	l->xscore = x; \
	l->oscore = o; \
	cp->xscore += x; \
	cp->oscore += o; \
	

void
computerow(struct config *cp, int pos)
{
	COMPUTE(LLOOP, 0);
	/*DPRINTF(("computerow: pos=%d bx=%d bo=%d\n", pos, bx, bo));*/
}

void
computecol(struct config *cp, int pos)
{
	COMPUTE(TLOOP, bsize);
	/*DPRINTF(("computecol: pos=%d bx=%d bo=%d\n", pos, bx, bo));*/
}

void
domove(int dir, int pos, int mark)
{
	int old, i, s, n;
	char *b;
	struct config *cp;

	hno++;
	if (hno >= hmax) {
		DPRINTF(("double hist\n"));
		hmax *= 2;
		hist = realloc(hist, hmax * sizeof(struct config));
	}

	cp = &hist[hno];
	b = cp->board;
	*cp = hist[hno-1];
	if (mark == X)
		cp->xtot++;
	else
		cp->otot++;

#define MOVEBODY {\
	old = getpos(b,i); \
	if (old != mark) { \
		setpos(b, i, mark); \
		changed[n] = 1; \
	} else \
		changed[n] = 0; \
	n++; \
	if (old == SPACE) \
		break; \
	mark = old; \
	} \
	if (old == X) cp->xtot--; \
	else if (old == O) cp->otot--

        /*DPRINTF(("domove %c %d %c\n", dir, pos+1, TOCHAR(mark)));*/
	n = 0;
	switch (dir) {
	case 'T':
		for (TLOOP)
			MOVEBODY;
		for (i = 0; i < n; i++)
			if (changed[i])
				computerow(cp, i);
		computecol(cp, pos);
		break;
	case 'B':
		for (BLOOP)
			MOVEBODY;
		for (i = 0; i < n; i++)
			if (changed[i])
				computerow(cp, bsizem1 - i);
		computecol(cp, pos);
		break;
	case 'L':
		for (LLOOP)
			MOVEBODY;
		for (i = 0; i < n; i++)
			if (changed[i])
				computecol(cp, i);
		computerow(cp, pos);
		break;
	case 'R':
		for (RLOOP)
			MOVEBODY;
		for (i = 0; i < n; i++)
			if (changed[i])
				computecol(cp, bsizem1 - i);
		computerow(cp, pos);
		break;
	default:
		fprintf(stderr, "huh '%c'\n", dir);
		exit(1);
	}
}

static char dirs[] = "TRBL";

int nscore;

int
score(struct config *cp, int mark)
{
	int i, s, d;

#if 1
	nscore++;
#endif

	for (i = hno & 1; i < hno-1; i += 2) {
		if (memcmp(cp->board, hist[i].board, boardsize) == 0)
			return WIN;
	}

	if (cp->xfull > cp->ofull) {
		s = WIN;
	} else if (cp->ofull > cp->xfull) {
		s = -WIN;
	} else {
		s = cp->xscore - cp->oscore;
		d = cp->xtot - cp->otot;
		if (d < 0)
			s -= d*d;
		else
			s += d*d;
	}

	if (mark == X)
		return s;
	else
		return -s;
}

int
ab(int lvl, int mark, int lb, int d)
{
	int di, pos, ans, dir, s;

	s = score(&hist[hno], mark) - lvl;
	if (lvl <= 0)
		return s;
	if (s <= -WINL || s >= WINL)
		return s;

	ans = lb;
	for (di = 0; di < 4; di++) {
		dir = dirs[di];
		for (pos = 0; pos < bsize; pos++) {
			domove(dir, pos, mark);
			s = -ab(lvl-1, OTHER(mark), -d, -ans);
			if (s > ans)
				ans = s;
			undomove();
			if (ans >= d)
				goto end;
		}
	}
end:
	return ans;
}

struct move {
	char dir;
	int pos;
	int score;
};
struct move moves[bsize4], nmoves[bsize4];

void
computemove(char *dirp, int *posp, int mark)
{
	int pos, di, high, lvl, s, i, j, bdir = 0, bpos = 0;
	int dir;
	struct move *mp, *np, *tmp;
	
	mp = moves;
	for (di = 0; di < 4; di++) {
		dir = dirs[di];
		for (pos = 0; pos < bsize; pos++) {
			mp->dir = dir;
			mp->pos = pos;
			mp++;
		}
	}

	mp = moves; np = nmoves;
	for (lvl = 3; lvl < MAXLVL; lvl++) {
		high = -INF;
		for (i = 0; i < bsize4; i++) {
			domove(mp[i].dir, mp[i].pos, mark);
			s = -ab(lvl, OTHER(mark), -INF, -high);
			if (s > high) {
				high = s;
				bdir = mp[i].dir;
				bpos = mp[i].pos;
				if (WINNER(high)) {
					goto done;
				}
			}
			undomove();

			/* sort the move into np */
			for (j = i; j > 0; j--) {
				if (np[j-1].score >= s)
					break;
				np[j] = np[j-1];
			}
			np[j].dir = mp[i].dir;
			np[j].pos = mp[i].pos;
			np[j].score = s;

		}
		*dirp = bdir;
		*posp = bpos;

		DPRINTF(("lvl=%d nscore=%d, high=%d\n", lvl, nscore, high));
		nscore = 0;
		tmp = mp;
		mp = np;
		np = tmp;
	}
done:
	*dirp = bdir;
	*posp = bpos;
	DPRINTF(("high=%d\n", high));
}

char bestdir;
int bestpos;

void
timeout()
{
	printf("%c%d\n", bestdir, bestpos+1);

	exit(0);
}

int
main(int argc, char **argv)
{
	int i, mark, pos, delay;
	char dir;

	hmax = 50;
	hist = malloc(hmax * sizeof(struct config));
	memset(&hist[0], 0, sizeof (struct config));

	for (i = 1; i <= bsize; i++)
		sumsize += AMP(i);

	if (argc == 1)
		delay = 28;
	else
		delay = atoi(argv[1]);
	DPRINTF(("delay=%d\n", delay));

#if 0
	scanf("%d\n", &mark);
	if (mark != bsize) exit(1);
#endif

	for (mark = X; scanf("%c%d\n", &dir, &pos) == 2; mark = OTHER(mark)) {
		domove(dir, pos-1, mark);
	}

	signal(SIGALRM, timeout);
	alarm(delay);

	computemove(&bestdir, &bestpos, mark);

	DPRINTF(("done\n"));
	timeout();
	exit(0);
}
