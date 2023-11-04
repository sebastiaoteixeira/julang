CC=gcc
CFLAGS=-Iinclude -I. -I/usr/include/llvm-c-15/ -g

ODIR=obj/Debug/src

LIBS=-lLLVM-15 -lcrypto

_OBJ = main.o token.o lexer.o parser.o expressionParser.o logger.o codegen.o moduleHash.o parserSymbols.o module.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all : julang
	bash incBuildCount.sh


$(ODIR)/%.o: src/%.c
		$(CC) -c -o $@ $< $(CFLAGS)

julang : $(OBJ)
	bash incVersion.sh
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/llvm-15/lib
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
