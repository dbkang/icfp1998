/* Bradley C. Kuszmaul August 28, 1998
 * pousse.c for ICFP'98 programming contest.
 * (C) 1998 Yale University
 * This program may be distributed under the terms of the Gnu Public License.
 */

#include<assert.h>
#include<malloc.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include"rands.c"

FILE *mystdout=stdout;

extern int memcmp(const void *s1, const void *s2, size_t n);

int N,Ntimes4;
int boardsize;

typedef unsigned int KEY;

KEY key;       /* The incrementally maintained hash key */
signed int pcsqscore=0; /* The incrementally maintained piece-square score */
signed char *board;

int boardstackptr=0;
#define BOARDSTACKDEPTH 1000
signed char *boardstack[BOARDSTACKDEPTH];
KEY          keystack[BOARDSTACKDEPTH];
signed int   pcsqscstack[BOARDSTACKDEPTH];

/* boards on the boardstack from 0 to searchboardstackptr-1 are in the actual game.
 * The idea is that repeats that depend only on repeated positions from the actual game are ok to store into the T-table.
 * Also, a position at depth I can be stored in the T-table if all the repeats were found at depth greater than I. */ 
int searchboardstackptr=0;

int *pcsq;

int debuglevel=0;

struct decoded_move {
  char moveside;
  int  moveoffset;
} *decoded_moves;


#define EMPTY 0
#define COLX  1
#define COLO -1
#define OTHERCOLOR(c) (-(c))

char pa[]={'O',' ','X'};
#define COL2CHAR(col) (pa[(col)+1])

char sidename[]={'L','R','T','B'};

#ifndef NSTATS
int n_searches=0, n_bestfirst=0, n_repeats=0, n_repeat_calls=0, n_false_repeat_keys=0, n_static_evals=0;
#define WHEN_STATS(x) x
#else
#define WHEN_STATS(x) ((void)0)
#endif

#define SCORESIZE 21
#define WIN ((1<<(SCORESIZE-1))-1)
#define LOSE (-WIN)

#ifdef S_TABLE
typedef struct sentry {
  unsigned short keypart:9; /* 9 is enough */
  signed int score:SCORESIZE;
} SENTRY;

#define NSENTRIES (1<<24)
SENTRY sscores[NSENTRIES];
#endif

#ifdef T_TABLE
typedef struct tentry {
  unsigned short keypart:9; /* 9 is enough */
  unsigned char move:6;  /* really 6 bits is enough */
  unsigned char depth:7;
  signed int ubscore:SCORESIZE;   /* how much is enough?   A short? */
  signed int lbscore:SCORESIZE;
} TENTRY;

#define TTABLE_LOGSIZE 23
#define TTABLE_SIZE (1<<TTABLE_LOGSIZE)
TENTRY ttable[TTABLE_SIZE];

TENTRY* tfind (KEY key) {
  int index=key&(TTABLE_SIZE-1);
  return &ttable[index];
}

#define INCCOMMAWHENTTABLE(x) , x
#define WHEN_TTABLE(x) x
#else
#define INCCOMMAWHENTTABLE(x) 
#define WHEN_TTABLE(x) ((void)0)
#endif T_TABLE


void makemove (char moveside, int moveoff, int color) {
  int start,step,end;
  pcsqscstack[boardstackptr]=pcsqscore;
  keystack[boardstackptr]=key;
  memcpy(boardstack[boardstackptr++],board,boardsize);
  assert(boardstackptr<BOARDSTACKDEPTH);
  key^=rand_side_to_move;
  switch (moveside) {
  case 'L':
    start=(moveoff-1)*N;
    step =1;
    end  =moveoff*N-1;
    break;
  case 'R':
    start=moveoff*N-1;
    step =-1;
    end  =(moveoff-1)*N;
    break;
  case 'T':
    start=moveoff-1;
    step =N;
    end  =boardsize-N+moveoff-1;
    break;
  case 'B':
    start=boardsize-N+moveoff-1;
    step =-N;
    end  =moveoff-1;
    break;
  default: abort();
  }
  /*#define INCREMENTALREMOVE_maybe(piece) ({if (piece) { key^=rands[j+((piece+1)<<9)]; pcsqscore-=piece*pcsq[j];}})*/
  /*#define INCREMENTALADD_maybe(piece) ({if (piece) { key^=rands[j+((piece+1)<<9)]; pcsqscore+=piece*pcsq[j];}})*/
#define INCREMENTALREMOVE_forsure(piece,j) ({ key^=rands[j+((piece+1)<<9)]; pcsqscore-=piece*pcsq[j]; })
#define INCREMENTALADD_forsure(piece,j) ({ key^=rands[start+((piece+1)<<9)]; pcsqscore+=piece*pcsq[start]; })
#define INCREMENTALMOVE(piece,to,from) ({ if (piece) { key^=rands[from+((piece+1)<<9)]^rands[to+((piece+1)<<9)]; pcsqscore+=piece*(pcsq[to]-pcsq[from]); }})
  {
    int i,j;
    int newpiece;
    for (i=start;i!=end+step;i+=step) {
      if (board[i]==EMPTY) {
	for (j=i;j!=start;j-=step) {
	  board[j] = newpiece = board[j-step];
	  INCREMENTALMOVE(newpiece,j,j-step);
	}
	board[start]=color;
	INCREMENTALADD_forsure (color,start);
	return;
      }
    }
    /* the whole row is there */
    {
      int oldpiece=board[end];
      INCREMENTALREMOVE_forsure(oldpiece,end);
    }
    for (j=end;j!=start;j-=step) {
      board[j] = newpiece = board[j-step];
      INCREMENTALMOVE(newpiece,j,j-step);
    }
    board[start]=color;
    INCREMENTALADD_forsure(color,start);
  }
}
    
void mkmove (int move, int color) {
  makemove (decoded_moves[move].moveside, decoded_moves[move].moveoffset, color);
}

void unmove (void) {
  signed char *tmp;
  boardstackptr--;
  assert(boardstackptr>=0);
  tmp=board;
  board=boardstack[boardstackptr];
  boardstack[boardstackptr]=tmp;
  key=keystack[boardstackptr];
  pcsqscore=pcsqscstack[boardstackptr];
}

/* Determine whether the current board is a repeat.
 * Return 0 if not a repeat.
 * Return -1 if it is a repeat, but all the repeated indices are in the real game, not on the search tree.
 * If it is a repeat, but one of the repeated indices is in the search tree, return one plus the smallest
 *   boardstack index that is a repeat and is in the tree. */
int isrepeat (void) {
  int i;
  WHEN_STATS(n_repeat_calls++);
  for (i=searchboardstackptr; i<boardstackptr; i++) {
    if (keystack[i]!=key) continue;
    if (memcmp(board,boardstack[i],boardsize)==0) {
      WHEN_STATS(n_repeats++); 
      return i+1;
    } else {
      WHEN_STATS(n_false_repeat_keys++);
    }
  }
  for (i=0; i<searchboardstackptr; i++) {
    if (keystack[i]!=key) continue;
    if (memcmp(board,boardstack[i],boardsize)==0) {
      WHEN_STATS(n_repeats++); 
      return -1;
    } else {
      WHEN_STATS(n_false_repeat_keys++);
    }
  }
  return 0;
}

void printboard (FILE *f) {
  int i;
  fprintf(f," ");
  for(i=0;i<N;i++) fprintf(f,"%d",(i+1)%10);
  fprintf(f,"\n");
  for (i=0;i<N;i++) {
    int j;
    fprintf(f,"%d",(i+1)%10);
    for (j=0;j<N;j++) {
      fprintf(f,"%c",COL2CHAR(board[i*N+j]));
    }
    fprintf(f,"\n");
  }
}

int readmove (char *moveside, int *moveoff) {
  char inp[100],*tail;
  if (!fgets(inp,99,stdin)) return 0; 
  assert(strlen(inp)<98);
  *moveside=inp[0];
  *moveoff=strtol(inp+1,&tail,0);
  /*printf("move=%c%d\n",*moveside,*moveoff);*/
  assert(*tail=='\n');
  return 1;
}

int quit;

void handlealrm (int signum) {
  quit=1;
}

int computestraightscore (void)
     /* return net number of X straights */
{
  int netx=0;
  int i,j;
#ifdef S_TABLE
  SENTRY *e=&sscores[key&(NSENTRIES-1)];
  if (e->keypart==(key>>24)) {
    if ((e->score==WIN) || (e->score==LOSE)) return e->score;
  }
#endif
  for (i=0;i<N;i++) {
    /* look for row straights first */
    int  start=i*N;
    signed char firstpiece=board[start];
    if (!firstpiece) continue;
    for (j=1;j<N;j++) {
      if (board[start+j]!=firstpiece) goto not_in_row;
    }
    netx += firstpiece;
  not_in_row:
  }
  /* if there are row straights for both sides there cannot be any column straihgts */
  /* For now, just do the rows anyway */
  for (i=0;i<N;i++) {
    signed char firstpiece=board[i];
    if (!firstpiece) continue;
    for (j=i+N;j<boardsize;j+=N) {
      if (board[j]!=firstpiece) goto not_in_col;
    }
    netx += firstpiece;
  not_in_col:
  }
#ifdef S_TABLE
  if (netx) {
    e->keypart=key>>24;
    e->score= (netx<0) ? LOSE : WIN;
  }
#endif
  return netx;
}

/* The static evaluator "heurstic" has two components:
 *  A center-control term (pcsq) which quadratically prefers the center.
 *    (Linearly prefering the center seems to produce play that I don't like.)
 * A near-straight term, which is how close to having straights we are.
 *    N-1 straights are very dangerous, since if you get two of them on a dense board, then you can win.
 *    N-2 straights don't seem so dangerous to me, but if the holes have your friends next to them
 *     they get dangerous.  I would expect a horizon effect to cause trouble with properly evaluating these.
 *     Probably should have a quiescence search, but I don't know what the extension should be...
 *   Anyway, I will value rows exponentially with the number of surplus X's there are in the row.
 *    Exponentially grows too fast, for my limited score values (21 bits)
 *     so I will limit the exponential to being 2^10 for large boards.
 */
int compute_near_straights (void) {
  int sum=0;
  int i,j,bi=0;
  int throttle=N<11 ? 0 : N-11; 
  for (i=0;i<N;i++) {
    int net[3]; net[0]=0; net[2]=0;
    for (j=0;j<N;j++)
      net[1+board[bi++]]++; /* what a hack ... */ 
    sum+=1<<(net[2]-throttle);
    sum-=1<<(net[0]-throttle);
  }
  for (j=0;j<N;j++) {
    int net[3]; net[0]=0; net[2]=0;
    bi=j;
    for (j=0;j<N;j++) {
      net[1+board[bi]]++;
      bi+=N;
    }
    sum+=1<<(net[2]-throttle);
    sum-=1<<(net[0]-throttle);
  }
  return sum;
}

int heuristic (void)
     /* return a guess of the score from X's perspective. */
     /* requires that the board not be a win or lose for X. */
{
#ifdef S_TABLE
  SENTRY *e=&sscores[key&(NSENTRIES-1)];
  if (e->keypart==(key>>24)) {
    return e->score;
  } else {
    e->keypart=key>>24;
  }
#endif
  WHEN_STATS(n_static_evals++);
  {
    int result=pcsqscore+compute_near_straights();
#ifdef S_TABLE
    e->score=result;
#endif
    return result;
  }
}

#ifdef T_TABLE
/* return the worse of two stack positions.
 * That is, if both stack positions are psitive return the minimum.
 * If one is negative and one is positive, return the positive.
 * If both are negative, they are equivalent.
 * If both are zero, they are equivalent. */
int worse_stack_pos (int stackposA, int stackposB) {
  if (!stackposA) return stackposB;
  if (!stackposB) return stackposA;
  if (stackposA<0) return stackposB;
  if (stackposB<0) return stackposA;
  if (stackposA<stackposB) return stackposA;
  return stackposB;
}
#endif

int ab (int depth, int color, int alpha, int beta, int *move INCCOMMAWHENTTABLE(int *repeatstackpos)) {
#define STORETENTRY(mv,score,repeatpos,islb,isub) WHEN_TTABLE(({     \
  int nokeymatch = e->keypart!=key>>TTABLE_LOGSIZE;                  \
  if ((repeatpos<=0) || (repeatpos>boardstackptr)) {                 \
    if (islb) e->lbscore=score;                                      \
    else if (nokeymatch) e->lbscore=LOSE;                            \
    if (isub) e->ubscore=score;                                      \
    else if (nokeymatch) e->ubscore=WIN;                             \
    e->move=mv; e->depth=depth; e->keypart=key>>TTABLE_LOGSIZE; } }))
#ifdef T_TABLE
  int org_alpha=alpha;
  int     subrepeatstackpos=0, myrepeatstackpos=0;
  TENTRY *e=tfind(key);
  if ((e->keypart==(key>>TTABLE_LOGSIZE)) &&
      (e->depth>=depth)) {
    /*fprintf(stderr,"HIT: key=0x%x depth==%d eidx=0x%x e->keypart=0x%x e->depth=0x%x e->move=%d e->score=%d\n",key, depth, e-&ttable[0], e->keypart, e->depth, e->move, e->score);*/
    if ((e->ubscore==e->lbscore) || /* it is an exact match */
	(e->lbscore>=beta)) {       /*it is known to be >= beta */
      *move=e->move;
      WHEN_TTABLE(*repeatstackpos=0);   /* we never store information in the t_table if it depends on repeats on the search stack. */
      return e->lbscore;
    } else if (e->ubscore<=alpha) {      /* it is known to be <= alpha */
      *move=e->move;
      WHEN_TTABLE(*repeatstackpos=0);   /* we never store information in the t_table if it depends on repeats on the search stack. */
      return e->ubscore;
    }
  }
#endif  
  {
    int repeatpos=isrepeat();
    if (repeatpos) { WHEN_TTABLE(*repeatstackpos=repeatpos); return WIN; }
  }
  {
    int straightscore = computestraightscore(); /* net number of X straights */
    if (straightscore<0) { int result = color==COLO ? WIN : LOSE; STORETENTRY(0,result,0,1,1); return result; }
    if (straightscore>0) { int result = color==COLX ? WIN : LOSE; STORETENTRY(0,result,0,1,1); return result; }
  }
  if (depth==0) {
    /* compute the heuristic, which is from X's perspective the score  */
    int result=heuristic()*color;
#ifdef T_TABLE
    e->keypart=(key>>TTABLE_LOGSIZE);
    e->depth  =depth;
    e->ubscore  = e->lbscore = result;
#endif
    return result;
  }
  {
    int heurmove;
    if (depth>1) {
      /* Use recursive iterative deepening to find a good first move.   Note, no transposition table is used here.  My theory is that there are too many repeats in this game, and the table will be full of bad data. */
      /* $$$ We need to find a way to prefer positions with lots of repeats, since I'll bet this is our only hope:  Confound the other guy's repeat analsys. */
      /* we need to compute the heuristic best move, don't care about the score */
#ifdef T_TABLE
      if ((e->keypart==(key>>TTABLE_LOGSIZE)) &&
	  (e->depth>=depth-1)) {
	/*fprintf(stderr,"MHIT key=0x%x eidx=0x%x e->keypart=0x%x e->depth=0x%x e->move=%d e->score=%d\n",key, e-&ttable[0], e->keypart, e->depth, e->move, e->score);*/
	heurmove=e->move;
      } else
#endif 
	{
	  WHEN_TTABLE(int ignoresubrepeatstackpos);
	  /* use a full window for this */
	  ab (depth-1, color, LOSE, WIN, &heurmove INCCOMMAWHENTTABLE(&ignoresubrepeatstackpos));
	}
      if (quit) return 0;
    } else {
      heurmove = 0;
    }
    {
#define RETURNGOOD(mv,sc,lb,ub) ({ *move=mv; WHEN_STATS(({n_searches++; if (heurmove==mv) n_bestfirst++;})); STORETENTRY(mv,sc,myrepeatstackpos,lb,ub); if (sc+1000>WIN) sc--; if(sc-1000<LOSE) sc++; WHEN_TTABLE(*repeatstackpos=myrepeatstackpos); return sc; })
      int bestscore, bestmove=heurmove;
      int ignoreme;
      int amove;
      mkmove(heurmove,color);
      bestscore = -ab(depth-1, OTHERCOLOR(color), -beta, -alpha, &ignoreme INCCOMMAWHENTTABLE(&subrepeatstackpos));
      WHEN_TTABLE(myrepeatstackpos=worse_stack_pos(myrepeatstackpos,subrepeatstackpos));
      unmove();
      if (quit) return bestscore;
      if (bestscore>=beta) RETURNGOOD(heurmove,bestscore,1,0);
      if (bestscore>alpha) alpha=bestscore;
      for (amove=0;amove<Ntimes4;amove++) {
	int thisscore;
	if (amove==heurmove) continue;
	mkmove(amove,color);
	thisscore = -ab(depth-1, OTHERCOLOR(color), -beta, -alpha, &ignoreme INCCOMMAWHENTTABLE(&subrepeatstackpos));
	WHEN_TTABLE(myrepeatstackpos=worse_stack_pos(myrepeatstackpos,subrepeatstackpos));
	unmove();
	if (quit) return bestscore;
	if (thisscore>=beta) RETURNGOOD(amove,thisscore,1,0);
	if (thisscore>alpha) alpha=thisscore;
	if (thisscore>bestscore) { bestmove=amove; bestscore=thisscore; }
      }
      RETURNGOOD(bestmove,bestscore,(bestscore>org_alpha),1); /* it is always < beta, so it is always an upper bound. */
    }
  }
}

void printdecodedmove (FILE *f,int amove) {
  fprintf(f,"%c%d",decoded_moves[amove].moveside,decoded_moves[amove].moveoffset);
}

void search_for (int seconds, int color, int *move) {
  int bestmove = 0;  /* make sure we have some legal move to return */
  int bestscore;
  int prevdepthbestmove = bestmove;
  int depth;
  searchboardstackptr=boardstackptr;
  quit=0;
  signal(SIGALRM,handlealrm);
  alarm(seconds);
  if (isrepeat() || computestraightscore()) { *move=0;  /*printf("quit(REP %d)\n",bestmove);*/ return; }
  for (depth=1;1;depth++) {
    int amove;
    int thisscore;
    int ignoreme INCCOMMAWHENTTABLE(ignorerepeatdepth);
    if (quit) { *move=bestmove;  if (debuglevel) fprintf(stderr,"quit(L1 %d)\n",bestmove); return; }
    mkmove(prevdepthbestmove,color);
    bestscore = -ab(depth, OTHERCOLOR(color), LOSE, WIN, &ignoreme INCCOMMAWHENTTABLE(&ignorerepeatdepth));
    /*printdecodedmove(prevdepthbestmove); printf("=%d (",bestscore); printdecodedmove(ignoreme); printf(")\n");*/
    unmove();
    if (quit) { *move=bestmove;  if (debuglevel) fprintf(stderr,"quit(L2 %d)\n",bestmove); return; }
    for (amove=0; amove<Ntimes4; amove++) {
      if (amove==prevdepthbestmove) continue; /* already did this move */
      /* the movearray is filled in with our heuristic move ordering. */
      mkmove(amove,color);
      thisscore = -ab(depth, OTHERCOLOR(color), -bestscore, WIN, &ignoreme INCCOMMAWHENTTABLE(&ignorerepeatdepth));
      unmove();
      /*printdecodedmove(amove); printf("=%d\n",thisscore);*/
      if (quit) { *move=bestmove; if (debuglevel) fprintf(stderr,"quit(L3 %d)\n",bestmove); return; }
      if (thisscore>bestscore) {
	if (debuglevel) {
	  fprintf(stderr,"Improved to "); printdecodedmove(stderr,amove); fprintf(stderr," s=%d from %d\n",thisscore,bestscore);
	}
	bestscore=thisscore;
	bestmove =amove;
      }
    }
    if (debuglevel) { fprintf(stderr,"D=%d s=%d m=",depth,bestscore); printdecodedmove(stderr,bestmove); fprintf(stderr,"\n"); }
    prevdepthbestmove=bestmove;
    if (bestscore>=WIN-999 || bestscore<=LOSE+999) { *move=bestmove; return; }
  }
}

void createboard (void) {
  int i;
  boardsize=N*N;
  Ntimes4  =4*N;
  board=(signed char*)calloc(boardsize,sizeof(char));
  for(i=0;i<BOARDSTACKDEPTH;i++) {
    boardstack[i]=(signed char*)calloc(boardsize,sizeof(char));
  }
  /*printf("setting up decoded moves\n");*/
  decoded_moves=(struct decoded_move *)calloc(Ntimes4, sizeof(struct decoded_move));
  for(i=0;i<Ntimes4;i++) {
    int lowbits     =i&3;
    int midbit      =i&4;
    int highbits    =i>>3;
    decoded_moves[i].moveside  =sidename[lowbits];
    decoded_moves[i].moveoffset= midbit ? ((N+1)>>1)+highbits+1 :  ((N+1)>>1)-highbits;
    /*printf("m%d: %c%d\n",i,decoded_moves[i].moveside,decoded_moves[i].moveoffset);*/
  }
  pcsq = (int*)calloc(boardsize,sizeof(int));
  {
    int bi=0;
    /*printf("pcsq:\n");*/
    for (i=0;i<N;i++) {
      int j;
      for (j=0;j<N;j++) {
	int idist = (i<(N>>1)) ? i : N-1-i;
	int jdist = (j<(N>>1)) ? j : N-1-j;
	int totaldist = idist+jdist;
	pcsq[bi]=600+(totaldist*(totaldist+1)*(totaldist+2))/6;
	/*printf("%2d ",pcsq[bi]);*/
	bi++;
      }
      /*printf("\n");*/
    }
  }
}

#ifndef NOMAIN
int main (int argc, char *argv[]) {
  int nmoves=1, mnum;
  char inp[100],*end;
  int color=COLX;
  char moveside;
  int  moveoff;
  /*fprintf(stderr,"I wrote to stderr, I have %d args\n",argc);*/
#ifndef NDEBUG
  assert(argc<=3);
  if (argc>=2) {
    nmoves=strtol(argv[1],&end,0);
    assert((!*end) && (end!=argv[1]));
  }
  if (argc==3) {
    /*printf("argv[2]=%s",argv[2]);*/
    debuglevel=strtol(argv[2],&end,0);
    assert((!*end) && (end!=argv[1]));
    fprintf(stderr,"Debug level=%d\n",debuglevel);
  }
  if (debuglevel) fprintf(stderr,"WIN=%d\n",WIN);
#endif
  if (!fgets(inp,99,stdin)) abort();
  assert(strlen(inp)<98);
  N=strtol(inp,&end,0);
  assert(*end=='\n' && end!=inp);
  /*printf("boardsize=%d\n",N);*/
  createboard();
  while (readmove(&moveside,&moveoff)) {
    /*printf("Move=%c%d\n",moveside,moveoff);*/
    makemove(moveside,moveoff,color);
    color=-color;
    /*printboard();*/
  }
  /*printf("Done reading moves\n");*/
  if (debuglevel&2) printboard(stderr);
  for (mnum=0;mnum<nmoves;mnum++) {
    int move,heu;
    search_for(29,color,&move);
#ifndef NSTATS
    if (debuglevel)
      fprintf(stderr,"move: %c%d  bestrate=%f (%d/%d)  rep=%d/%d (false=%d)   stat=%d\n",decoded_moves[move].moveside,decoded_moves[move].moveoffset,
	     (double)n_bestfirst/(double)n_searches, n_bestfirst, n_searches, n_repeats, n_repeat_calls, n_false_repeat_keys, n_static_evals);
#endif
    printf("%c%d\n",decoded_moves[move].moveside,decoded_moves[move].moveoffset);
    makemove(decoded_moves[move].moveside,decoded_moves[move].moveoffset,color);
    color=-color;
    if (debuglevel&2) printboard(stderr);
    heu=heuristic();
    if (debuglevel) fprintf(stderr,"BSD=%d\n",boardstackptr);
    if (isrepeat()) {
      if (debuglevel) fprintf(stderr,"Repeat!\n");
      return 0;
    }
    if (computestraightscore()) {
      if (debuglevel) fprintf(stderr,"Straight!\n");
      return 0;
    }
  }
  return 0;
}
#endif
