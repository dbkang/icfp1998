#PROFFLAGS=-pg
ALLCFLAGS=-Wall -O3 -fpack-struct
CFLAGS=-g $(ALLCFLAGS)
FINALFLAGS =  -DNDEBUG -DNSTATS
default: pousse pousseNT autotest
pousse: pousse.c rands.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -static pousse.c -o pousse -DT_TABLE
pousseNT: pousse.c rands.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -static pousse.c -o pousseNT -DS_TABLE
runme: pousse.c rands.c
	$(CC) $(ALLCFLAGS) $(FINALFLAGS) -static pousse.c -o runme -DS_TABLE
	rm -f makerand
	strip runme
autotest: autotest.c pousse.c rands.c
	$(CC) $(CFLAGS) autotest.c -o autotest
test: pousse
	pousse<test1
test3: pousse
	echo "4"|pousse 40
rands.c: makerand
	makerand>rands.c

tarit:
	install -d submit.dir/src/
	install -d submit.dir/support/
	install readme submit.dir/
	install pousse.c Makefile makerand.c submit.dir/src/
	(cd submit.dir/src;gmake runme;mv runme ..)
	(cd submit.dir;tar cf ../submission.tar runme readme src support)
	(cd submit.dir;tar cf ../submission-toc.tar runme readme support)
	gzip submission.tar

cleantar:
	rm -rf submit.dir
clean: cleantar
	rm pousse *.o rand.c

dotest4: pousse
	(pousse 1 1<test4)
dotest5: pousse
	echo pousse 1 1 "<test5"|csh
	echo It would be bad to move R2 at this point.
