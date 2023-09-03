edit : main

main : main.o token.o lexer.o parser.o expressionParser.o logger.o codegen.o moduleHash.o
		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/llvm-15/lib
		gcc -g obj/Debug/src/main.o obj/Debug/src/token.o obj/Debug/src/lexer.o obj/Debug/src/parser.o obj/Debug/src/expressionParser.o obj/Debug/src/logger.o obj/Debug/src/codegen.o obj/Debug/src/moduleHash.o -o bin/Debug/julang -Xlinker -lLLVM-15 -lcrypto

main.o :
		gcc -g -c src/main.c -o obj/Debug/src/main.o -Iinclude

token.o :
		gcc -g -c src/token.c -o obj/Debug/src/token.o -Iinclude

lexer.o :
		gcc -g -c src/lexer.c -o obj/Debug/src/lexer.o -Iinclude

parser.o :
		gcc -g -c src/parser.c -o obj/Debug/src/parser.o -Iinclude

expressionParser.o :
		gcc -g -c src/expressionParser.c -o obj/Debug/src/expressionParser.o -Iinclude

logger.o :
		gcc -g -c src/logger.c -o obj/Debug/src/logger.o -Iinclude

codegen.o :
		gcc -g -c src/codegen.c -o obj/Debug/src/codegen.o -Iinclude

moduleHash.o :
		gcc -g -c src/moduleHash.c -o obj/Debug/src/moduleHash.o -Iinclude

clean :
		rm edit main.o