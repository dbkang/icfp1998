/* Copyright (c) 1998 by Andrew W. Appel */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>

/* #define MAXN 8 */

#if (MAXN<=8)
typedef unsigned char shortword;
#else
#if (MAXN<=16)
typedef unsigned short shortword;
#else
typedef unsigned long shortword;
#endif
#endif 

typedef unsigned long word;
typedef struct player *Player;

#define chatty 0

#define MAXHIST 3000
#define HASHT_SIZE 4096
#define STRAIGHT 10000000
#define INFINITY 0x7fffffff
#define WIN    1000000000
#define NEARWIN  10000000
#define POSTPONE       10
#define MAXDEPTH  40
int N;
word leftcursor;  /* 1<<(N-1) */
word allFilled;   /* (1<<N)-1 */
int maxdepth=5;
int pleaseStop=0;
int bitcount[1<<MAXN];
int (*eval2)(Player);
int eval2_8(Player);
int eval2_5(Player);
int eval2_4(Player);
int eval2_N(Player);
int hshift;

int pushHist(void);

void initialize(int n) {
  int i,j;
  word w;
  if (n>MAXN) {fprintf(stderr,"N too large\n"); exit(1);}
  N=n;
  leftcursor =  1<<(N-1);
  allFilled =  (1<<N)-1;
  hshift = N<15?N:15;
  for(i=0;i<(1<<N);i++) {
    for(w=i; w; w>>=1) bitcount[i]+=(w&1);
  }
  eval2 = N<=4 ? eval2_4 : N<=5 ? eval2_5 : N<=8 ? eval2_8 : eval2_N;
  pushHist();
}

int whichAmI=0;

struct player {shortword b[MAXN]; };
struct board {struct player x, o;};

struct histrec {struct histrec *link; 
                 word hash;
                 struct board brd;
                };

struct board theBoard;

struct histrec *hashtable[HASHT_SIZE];

int numInHist=0;

static int Li(int i, Player me, Player you, int force) {
  int consec; word cursor;
  word meRow = me->b[i];
  word youRow = you->b[i];
  word usRow = meRow | youRow;
  if (numInHist==1 && i+i>=N && !force) return 0;
  for(consec=0, cursor=leftcursor;
      cursor & usRow;
      consec++, cursor>>=1);
  if (consec==N) {
   me->b[i] = (meRow>>1) | leftcursor;
   you->b[i] = youRow>>1;
   return 2;
  } else {
   meRow = ((meRow>>(N-consec))<<(N-consec-1)) | (meRow&(1<<(N-consec))-1);
   youRow = ((youRow>>(N-consec))<<(N-consec-1))| (youRow&(1<<(N-consec))-1);
  }
  meRow = leftcursor | meRow;
  me->b[i]=meRow;
  you->b[i]=youRow;
  return 1;
}

static int Ri(int i, Player me, Player you, int force) {
  int consec; word cursor;
  word meRow = me->b[i];
  word youRow = you->b[i];
  word usRow = meRow | youRow;
  if (numInHist==1 && !force) return 0;
  for(consec=0, cursor=1;
      cursor & usRow;
      consec++, cursor<<=1);
  if (consec==N) {
   me->b[i] = (meRow<<1)&allFilled | 1;
   you->b[i] = (youRow<<1)&allFilled;
   return 2;
  } else {
   int maskl = (allFilled<<consec)&allFilled;
   int maskr = (allFilled>>(N-consec));
   meRow = (meRow&maskl) | ((meRow&maskr)<<1);
   youRow = (youRow&maskl) | ((youRow&maskr)<<1);
  }
  meRow = meRow | 1;
  me->b[i]=meRow;
  you->b[i]=youRow;
  return 1;
}

static int Ti(int i, Player me, Player you, int force) {
  word cursor = 1<<i;
  int k=0;
  word t,c;
  word meCarry = cursor, youCarry=0;
  if (numInHist<4 && !force) {
     if (numInHist==1 && !force) return 0;
     if (((you->b[0]|you->b[N-1])&((1<<(N-1))|1))) {
      if (numInHist==2) return 0;
      if ( ((me->b[0]|me->b[N-1])&((1<<(N-1))|1))
          && (you->b[0]|you->b[N-1]|me->b[0]|me->b[N-1])==((1<<(N-1))|1)
          && (you->b[0]|me->b[0]) && (you->b[N-1]|me->b[N-1]))
        return 0;
     }
  }
  {word w=allFilled;
   while (me->b[k]&cursor) {
     w &= (me->b[k]|you->b[k]);
     k++;
   }
   if (!(you->b[k]&cursor)) {
     word allRight = (1<<i)-1;
     word allLeft = allFilled & ~(cursor|allRight);
     w &= me->b[k];
     if ((w&allRight)==allRight || ((w&allLeft)==allLeft)) 
       if (!force) return 0;
   }
   k=0;
  }
  do {
    t = me->b[k];
    c = t&cursor;
    me->b[k] = t & (~cursor) | meCarry;
    meCarry = c;

    t = you->b[k];
    c = t&cursor;
    you->b[k] = t & (~cursor) | youCarry;
    youCarry = c;

    k++;
  } while ((meCarry|youCarry) && k<N);
  return 1 + (k==N);
}

static int Bi(int i, Player me, Player you, int force) {
  word cursor = 1<<i;
  int k=N-1;
  word meCarry = cursor, youCarry=0;
  word t,c;
  if (numInHist<4 && !force) {
    if (numInHist==1) return 0;
    if (((you->b[0]|you->b[N-1])&((1<<(N-1))|1))) {
      if (numInHist==2) return 0;
      if ( ((me->b[0]|me->b[N-1])&((1<<(N-1))|1))
          && (you->b[0]|you->b[N-1]|me->b[0]|me->b[N-1])==((1<<(N-1))|1)
          && (you->b[0]|me->b[0]) && (you->b[N-1]|me->b[N-1]))
               return 0;
    }
  }
  {word w=allFilled;
   while (me->b[k]&cursor) {
     w &= (me->b[k]|you->b[k]);
     k--;
   }
   if (!(you->b[k]&cursor)) {
     word allRight = (1<<i)-1;
     word allLeft = allFilled & ~(cursor|allRight);
     w &= me->b[k];
     if ((w&allRight)==allRight || ((w&allLeft)==allLeft))
       if (!force) return 0;
   }
   k=N-1;
  }
  do {
    t = me->b[k];
    c = t&cursor;
    me->b[k] = t & (~cursor) | meCarry;
    meCarry = c;

    t = you->b[k];
    c = t&cursor;
    you->b[k] = t & (~cursor) | youCarry;
    youCarry = c;

    k--;
  } while ((meCarry|youCarry) && k>=0);
  return 1 + (k<0);
}

/* returns: 
     0 - this move is redundant with some other legal move
     1 - this move is not redundant, no net gain of X's/O's
     2 - one more token on the board 
*/

int doMove(int i, Player me, Player you, int force) {
  if (i<N+N)
    if (i<N) 
         return Li(i,me,you,force);
    else return Ri(i-N,me,you,force);
  else if (i<N+N+N)
         return Ti(i-(N+N),me,you,force);
    else return Bi(i-(N+N+N),me,you,force);
}

char *Lnames[] =
  {"L1","L2","L3","L4","L5","L6","L7","L8","L9","L10",
   "L11","L12","L13","L14","L15","L16","L17","L18","L19","L20"};
char *Rnames[] =
  {"R1","R2","R3","R4","R5","R6","R7","R8","R9","R10",
   "R11","R12","R13","R14","R15","R16","R17","R18","R19","R20"};
char *Tnames[] =
  {"T1","T2","T3","T4","T5","T6","T7","T8","T9","T10",
   "T11","T12","T13","T14","T15","T16","T17","T18","T19","T20"};
char *Bnames[] =
  {"B1","B2","B3","B4","B5","B6","B7","B8","B9","B10",
   "B11","B12","B13","B14","B15","B16","B17","B18","B19","B20"};


char *nameMove(int i) {
  if (i<N+N)
    if (i<N) 
         return Lnames[i];
    else return Rnames[i-N];
  else if (i<N+N+N)
         return Tnames[N-1-(i-2*N)];
    else return Bnames[N-1-(i-3*N)];
}

struct histrec history[MAXHIST];

word hashBoard() {
  word h=numInHist&1;
  int i;
  for (i=0;i<N;i++) {
    h = h * 65599 + ((theBoard.x.b[i]<<hshift) ^ theBoard.o.b[i]);
    /*    h = (h * 65599 + theBoard.x.b[i]) * 65559 + theBoard.o.b[i]; */
  }
  return h*41;
}

static int eqBoard(struct board *p, struct board *q) {
  /*
  int i;
  for (i=0; i<N; i++)
    if (p->x.b[i]!=q->x.b[i] || p->o.b[i]!=q->o.b[i]) return 0;
  return 1;
  */
}

/* returns 0 if a duplicate position */
int pushHist(void) {
  word h = hashBoard();
  word index =  h & (HASHT_SIZE - 1);
  struct histrec *p;
  for (p = hashtable[index]; p; p=p->link)
     if (p->hash == h && eqBoard(&p->brd, &theBoard)) return 0;
  p = history + numInHist++;
  memcpy(&p->brd,&theBoard,sizeof(theBoard)); /*  p->brd = theBoard; */
  p->hash = h;
  p->link = hashtable[index];
  hashtable[index] = p;
  return 1;
}

void popHist(void) {
  struct histrec *p = history + --numInHist;
  hashtable[p->hash & (HASHT_SIZE-1)] = p->link;
}

void undoMove() {
  memcpy(&theBoard,&history[numInHist-1].brd,sizeof(theBoard));
  /* theBoard = history[numInHist-1].brd; */
}

void printBoard(FILE *f, struct board *p, int indent) {
  int i,j;
  for(i=0; i<N; i++) {
    for(j=0; j<indent; j++) fprintf(f," ");
    for(j=0; j<N; j++)
      fprintf(f, " %c", (p->x.b[i]>>(N-1-j))&1 ? 'X' :
	                (p->o.b[i]>>(N-1-j))&1 ? 'O' : '.');
    fprintf(f, "\n");
  }
  fprintf(f, "\n");
}

#define memodepth 1
#define MEMOTABSIZE (1<<20)
word memotab[MEMOTABSIZE];
word memoval[MEMOTABSIZE];

static void enterMemo(int depth, int value) {
  word h = history[numInHist-1].hash;
  word index = (h+depth)&(MEMOTABSIZE-1);
  memotab[index] = h;
  memoval[index] = value;
}

static int lookMemo(int depth) {
  word h = history[numInHist-1].hash;
  word index = (h+depth)&(MEMOTABSIZE-1);
  if (memotab[index] == h)
    return memoval[index];
  else return INFINITY;
}

int eval2_N(Player me) {
  static int cols[MAXN];
  int score=0;
  int i,  *j;
  word t;
  for(i=0;i<N;i++) {
    word w = me->b[i];
    int inrow=bitcount[w];
    score+=inrow*inrow;
    for(j=cols+N-1;w;j--,w>>=1) {
       t=w&1;
       (*j)+=t;
    }
  }
  for(i=0;i<N;i++) {
    int t = cols[i];
    score+= t; score += t*t;
    cols[i]=0;
  }
  return score;
}

int eval2_5(Player me) {
  int col0=0,col1=0,col2=0,col3=0,col4=0;
  int score=0;
  int i,t;
  for(i=0;i<5;i++) {
    word w = me->b[i];
    int inrow=bitcount[w];
    score+=inrow*inrow;
    col0+= w&1;
    col1+= w&2;
    col2+= w&4;
    col3+= w&8;
    col4+= w&16;
  }
  t=col0;    score += t; score += t*t;
  t=col1>>1; score += t; score += t*t;
  t=col2>>2; score += t; score += t*t;
  t=col3>>3; score += t; score += t*t;
  t=col4>>4; score += t; score += t*t;
  return score;
}

int eval2_4(Player me) {
  int col0=0,col1=0,col2=0,col3=0;
  int score=0;
  int i,t;
  for(i=0;i<4;i++) {
    word w = me->b[i];
    int inrow=bitcount[w];
    score+=inrow*inrow;
    col0+= w&1;
    col1+= w&2;
    col2+= w&4;
    col3+= w&8;
  }
  t=col0;    score += t; score += t*t;
  t=col1>>1; score += t; score += t*t;
  t=col2>>2; score += t; score += t*t;
  t=col3>>3; score += t; score += t*t;
  return score;
}

int eval2_8(Player me) {
  int col0=0,col1=0,col2=0,col3=0,col4=0,col5=0,col6=0,col7=0;
  int score=0;
  int i,t;
  for(i=0;i<N;i++) {
    word w = me->b[i];
    int inrow=bitcount[w];
    score+=inrow*inrow;
    col0+= w&1;
    col1+= w&2;
    col2+= w&4;
    col3+= w&8;
    col4+= w&16;
    col5+= w&32;
    col6+= w&64;
    col7+= w&128;
  }
  t=col0;    score += t; score += t*t;
  t=col1>>1; score += t; score += t*t;
  t=col2>>2; score += t; score += t*t;
  t=col3>>3; score += t; score += t*t;
  t=col4>>4; score += t; score += t*t;
  t=col5>>5; score += t; score += t*t;
  t=col6>>6; score += t; score += t*t;
  t=col7>>7; score += t; score += t*t;
  return score;
}

int full_eval(Player me, Player you) {
  int i; word t;
  int score=0;
  word meCols=allFilled, youCols=allFilled;
  for (i=0;i<N;i++) {
    t = me->b[i];
    meCols &= t;
    if (t==allFilled) score+=STRAIGHT;
    t = you->b[i];
    youCols &= t;
    if (t==allFilled) score-=STRAIGHT;
  }
  for(; meCols; meCols>>=1)   if (meCols&1) score+=STRAIGHT;
  for(; youCols; youCols>>=1) if (youCols&1) score-=STRAIGHT;
  if (score >= STRAIGHT) score=WIN;
  else if (score <= -STRAIGHT) score= -WIN;
  return score;
}

static int eval(Player me, Player you) {
  int i; word t;
  word meCols=allFilled, youCols=allFilled;
  int meRows, youRows;
  meRows=youRows=0;
  for (i=0;i<N;i++) {
    t = me->b[i];
    meCols &= t;
    meRows |= (t+1)>>N;
    t = you->b[i];
    youCols &= t;
    youRows |= (t+1)>>N;
  }
  if (meCols|meRows)
     if (youCols|youRows) return full_eval(me,you);
     else return WIN;
  else if (youCols|youRows) return -WIN;
  else return 0;
}


int search(int move, Player me, Player you, 
	   int depth, int lobound, int hibound) {
   int score, valbest, i, val, moveStatus;
   if (pleaseStop) return -INFINITY;
   moveStatus=doMove(move,me,you,0);
   if (!moveStatus)
     return -INFINITY;
   if (pushHist()==0)
     {undoMove(); return -WIN;}
   score = eval(me,you);
   if (score== -WIN || score == WIN)
     valbest=score;
   else if (depth==0)
     valbest = score + eval2(me) - eval2(you);
   else if (depth>memodepth && moveStatus==2 && 
	    (valbest=lookMemo(depth))!=INFINITY)
     {  }
   else {
     valbest = hibound;
     if (valbest >= lobound) {
       for (i=0; i<N*4; i++) {
	 val = -search(i,you,me,depth-1,-valbest,-lobound);
	 if (val < valbest) {
	   valbest = val;
	   if (valbest < lobound) break;
	 }
       }
     }
     if (valbest > NEARWIN) valbest-=POSTPONE;
     else if (valbest < -NEARWIN) valbest+=POSTPONE;
     if (depth > memodepth && moveStatus==2) enterMemo(depth,valbest);
   }
   popHist();
   undoMove();
   return valbest;
}

int bestMove (Player me, Player you, int *valp, 
	      int depth, int *bestsIn, int *bestsOut ) {
  int bestsofar= -1;
  int valbest= -INFINITY;
  int i, valthis;
  int numMoves=N*4;
  for(i=0;i<numMoves;i++) {
    int m = bestsIn[i];
    if (whichAmI && (m%4)!=whichAmI-1)
         valthis= -INFINITY;
    else valthis = search(m,me,you,depth,valbest, NEARWIN);
    if (chatty && valthis>-INFINITY && !pleaseStop) {
      printf("%s=%d ",nameMove(m),valthis); fflush(stdout);
    }
    if (valthis > valbest && !pleaseStop) {
      int j;
      for(j=i;j>0;j--) bestsOut[j]=bestsOut[j-1];
      bestsOut[0]=m;
      bestsofar = m;
      valbest = valthis;
    }
    else bestsOut[i]=m;
  }
  if (chatty) printf("\n");
  *valp = valbest;
  return bestsofar;
}
 
int readMove(FILE *f) {
  int i;
  char buf[100];
  buf[0]=buf[1]=buf[2]=buf[3]=0;
  fgets(buf,98,f);
  if (isdigit(buf[1])) i=buf[1]-'0'; 
    else return -1;
  if (isdigit(buf[2])) i= i*10 + buf[2]-'0'; 
   else if (buf[2]!='\n') return -1;
  switch (buf[0]) {
  case 'L': return i-1;
  case 'R': return N+i-1;
  case 'T': return N+N+N-i;
  case 'B': return N+N+N+N-i;
  default: return -1;
  }
}

int readMoveInteractive(FILE *f) {
 for(;;) {
   int m = readMove(f);
   if (m>=0) return m;
   fprintf(stderr,"Illegal move\n");
 }
} 

#ifdef TEST
#ifdef PARALLEL
struct itimerval tournament_limit = {{0,0},{58,000000}};
#else
struct itimerval tournament_limit = {{0,0},{14,500000}};
#endif
#else 
struct itimerval tournament_limit = {{0,0},{29,500000}};
#endif


void handleSig(int sig) {
  if (chatty) printf("Time's up! ");
  pleaseStop=1;
}

int orders[MAXDEPTH][MAXN*4];

int computeMove(Player me, Player you, int *v) {
  int i, theMove=0, theValue= -INFINITY;
  int depth=3;
  for (i=0;i<N*4;i++)
    orders[depth][i]=i;
  do {
     int v,m;
     if (chatty) printf("Depth %d: ",depth);
     m=bestMove(me,you,&v,depth,orders[depth],orders[depth+1]);
     if (m<0) break;
     if (chatty) printf("\n");
     theValue=v;
     theMove=m;
     depth++;
  } while (depth<MAXDEPTH-3 && !pleaseStop && abs(theValue)<NEARWIN);
  *v=theValue;
  return theMove;
}

static int computeWhoeverMove(int *value) {
     if (numInHist&1)
          return computeMove(&theBoard.x,&theBoard.o,value);
     else return computeMove(&theBoard.o,&theBoard.x,value);
}

FILE *fdopen( int fildes, char *mode);

int runbig(int n) {
  int child;
  int fd[2];
  pipe(fd);
  if (child=fork()) {
    /* parent */
    int c, status;
    FILE *f = fdopen(fd[1],"w");
    fprintf(f,"%d\n",n);
    while ((c=getchar())!=EOF)
      putc(c,f);
    fclose(f);
    waitpid(child,&status,0);
    exit(status);
  } else {
    /* child */
    char *name = n<=16 ? "support/runme16" : "support/runme20";
    dup2(fd[0],0);
    close(fd[0]);
    close(fd[1]);
    execl(name,name,NULL);
  }
}

#ifdef PARALLEL
int doParallel() {
  FILE *f;
  int i, bestmove, bestval, child, fd[2];
  pipe(fd);
  for(i=1;i<=4;i++) {
    if (!fork()) {
      /* child! */
      int m,v;
      char buf[100];
      whichAmI=i;
      signal(SIGALRM,handleSig);
      setitimer(ITIMER_REAL, &tournament_limit, NULL);
      m = computeWhoeverMove(&v);
      sprintf(buf,"%d %d\n",m,v);
      write(fd[1],buf,strlen(buf));
      exit(0);
    }
  }
  bestmove=0; bestval= -INFINITY;
  f=fdopen(fd[0],"r");
  for(i=1;i<=4;i++) {
    int m,v;
    fscanf(f,"%d%d",&m,&v);
    if (v>bestval) {bestmove=m; bestval=v;}
  }
  for(i=1;i<=4;i++) wait();
  return bestmove;
}
#endif

int main(int argc, char **argv) {
     char buf[100];
     int n,m,v;
     fgets(buf,98,stdin);
     n=atoi(buf);
     if (n>MAXN) runbig(n);
     pleaseStop=0;
     setitimer(ITIMER_REAL, &tournament_limit, NULL);
     signal(SIGALRM,handleSig);
     initialize(n);
     while (!feof(stdin)) {
       m=readMove(stdin);
       if (m<0) break;
       if (numInHist&1) 
	    doMove(m,&theBoard.x,&theBoard.o,1);
       else doMove(m,&theBoard.o,&theBoard.x,1);
       assert(pushHist());
     }
#ifdef PARALLEL
     m=doParallel();
#else
     m=computeWhoeverMove(&v);
#endif
     printf("%s\n",nameMove(m));
     exit(0);
}
