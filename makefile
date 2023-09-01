edit : main

main : main.o token.o lexer.o parser.o expressionParser.o logger.o codegen.o
		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/llvm-15/lib
		gcc obj/Debug/src/main.o obj/Debug/src/token.o obj/Debug/src/lexer.o obj/Debug/src/parser.o obj/Debug/src/expressionParser.o obj/Debug/src/logger.o obj/Debug/src/codegen.o -o bin/Debug/julang -Xlinker -lLLVM-15

main.o :
		gcc -c src/main.c -o obj/Debug/src/main.o -Iinclude

token.o :
		gcc -c src/token.c -o obj/Debug/src/token.o -Iinclude

lexer.o :
		gcc -c src/lexer.c -o obj/Debug/src/lexer.o -Iinclude

parser.o :
		gcc -c src/parser.c -o obj/Debug/src/parser.o -Iinclude

expressionParser.o :
		gcc -c src/expressionParser.c -o obj/Debug/src/expressionParser.o -Iinclude

logger.o :
		gcc -c src/logger.c -o obj/Debug/src/logger.o -Iinclude

codegen.o :
		gcc -c src/codegen.c -o obj/Debug/src/codegen.o -Iinclude

clean :
		rm edit main.o