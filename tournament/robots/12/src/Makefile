CAMLC=ocamlc
CAMLCI=ocamlc
CAMLOPT=ocamlopt
CAMLDEP=ocamldep

COMPFLAGS=-g
LINKFLAGS=-g -custom
OPTCOMPFLAGS=-inline 10 -unsafe
OPTLINKFLAGS=-ccopt -static
DEPFLAGS=

OBJS=getopt.cmo misc.cmo timeout.cmo pousse.cmo eval.cmo printer.cmo \
	alphabeta.cmo main.cmo

all:runme.opt install

runme.bytecode: $(OBJS)
	rm $@ || true
	$(CAMLC) $(LINKFLAGS) -o runme unix.cma $(OBJS) -cclib -lunix

runme.opt: $(OBJS:.cmo=.cmx)
	rm $@ || true
	$(CAMLOPT) $(OPTLINKFLAGS) -o runme.opt unix.cmxa $(OBJS:.cmo=.cmx) \
		-cclib -lunix


# Default rules

.SUFFIXES: .c .o .ml .mli .cmo .cmi .cmx

.c.o .ml.cmo:
	$(CAMLC) $(COMPFLAGS) -c $<

.mli.cmi:
	$(CAMLCI) $(COMPFLAGS) -c $<

.ml.cmx:
	$(CAMLOPT) $(OPTCOMPFLAGS) -c $<

clean::
	rm -f runme runme.opt driver gmon.out *.cm[iox] *.o *.s

depend:
	$(CAMLDEP) $(DEPFLAGS) *.mli *.ml > .depend
#        gcc -I/usr/local/lib/ocaml -MM *.c >> .depend

install:
	mv runme.opt ../runme
include .depend
