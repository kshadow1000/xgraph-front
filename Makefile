CC := gcc
CFLAG := -Wall -O3
LFLAG := xgraph/xgraph.a -lc -lm
all: draw calc list wave
draw: main.c xgraph/xgraph.a common_symbols.o
	$(CC) $(CFLAG) main.c common_symbols.o -o draw $(LFLAG)
calc: calc.c xgraph/xgraph.a common_symbols.o
	$(CC) $(CFLAG) calc.c common_symbols.o -o calc $(LFLAG)
list: list.c xgraph/xgraph.a common_symbols.o
	$(CC) $(CFLAG) list.c common_symbols.o -o list $(LFLAG)
wave: wave.c xgraph/xgraph.a
	$(CC) $(CFLAG) -DTEXT_ENABLED wave.c -o wave $(LFLAG)
common_symbols.o: common_symbols.c
	rm -f systable.c
	make systable
	$(CC) $(CFLAG) common_symbols.c -c -o common_symbols.o
.PHONY:
leak:
	$(CC) -Wall -fsanitize=address xgraph/expr/leak.c -g calc.c common_symbols.c -o calc xgraph/expr/expr_*.c -lm
.PHONY:
leakw:
	$(CC) -Wall -fsanitize=address xgraph/expr/leak.c -g wave.c -o wave xgraph/expr/expr_*.c -lm
.PHONY:
leakl:
	$(CC) -Wall -fsanitize=address xgraph/expr/leak.c -g -g list.c common_symbols.c -o list xgraph/expr/expr_*.c -lm
.PHONY:
leakd:
	$(CC) -Wall -fsanitize=address xgraph/expr/leak.c -g -g main.c common_symbols.c -o draw xgraph/expr/expr_*.c xgraph/graph/graph.c xgraph/graph/texts/text.c xgraph/graph/texts/sbmp.c -lm
.PHONY:
debug:
	$(CC) -Wall -g calc.c common_symbols.c -o calc -DNDEBUG=0 xgraph/expr/expr_*.c -lm
.PHONY:
debugl:
	$(CC) -Wall -g list.c common_symbols.c -o list xgraph/expr/expr_*.c -lm
.PHONY:
symtestl:
	$(CC) $(CFLAG) symtest.c common_symbols.o -o symtest xgraph/expr/expr_*.c -g -fsanitize=address -lm
.PHONY:
symtest:
	$(CC) $(CFLAG) symtest.c common_symbols.o -o symtest $(LFLAG) -g
.PHONY:
sorttest:
	$(CC) $(CFLAG) sorttest.c common_symbols.o -o sorttest $(LFLAG)
xgraph/xgraph.a: xgraph
	make -C xgraph
.PHONY:
systable:
	bash systable.sh >systable.c
redoall:
	make cleanall
	make
.PHONY:
redo:
	make clean
	make
.PHONY:
clean:
	rm -f draw calc list wave symtest sorttest common_symbols.o systable.c
.PHONY:
cleanall:
	make clean
	make -C xgraph cleanall
