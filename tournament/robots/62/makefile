SRCDIR := src
OBJDIR := obj
BINDIR := bin
CC := clang++

$(BINDIR)/main : $(OBJDIR)/main.o $(BINDIR)
	$(CC) -o $(BINDIR)/main $(OBJDIR)/main.o

$(OBJDIR)/main.o : $(SRCDIR)/main.cc $(OBJDIR)
	$(CC) -o $(OBJDIR)/main.o -c $(SRCDIR)/main.cc


$(OBJDIR):
	mkdir $(OBJDIR)

$(BINDIR):
	mkdir $(BINDIR)
