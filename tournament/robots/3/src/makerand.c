#include<stdlib.h>
#include<stdio.h>
#include<assert.h>

int main (int argc, char *argv[]) {
  int i;
  assert(argc==1);
  assert(RAND_MAX>0x10000);
  printf("/* RAND_MAX=0x%x */\n",RAND_MAX);
  printf("unsigned int rands[]={");
  for (i=0;i<32*32*2;i++) {
    if (i!=0) printf(",");
    if ((i%16)==0) printf("\n"); else printf(" ");
    printf("0x%04x%04x",(unsigned)random()&0xffff,(unsigned)random()&0xffff);
  }
  printf("};\n");
  printf("#define rand_side_to_move ((unsigned int)0x%04x%04x)\n",
	 (unsigned)random()&0xffff,(unsigned)random()&0xffff);
  return 0;
}
