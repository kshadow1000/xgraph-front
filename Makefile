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
leak: calc.c xgraph/expr/expr.c common_symbols.c
	$(CC) -Wall -fsanitize=address -O3 -g calc.c common_symbols.c -o calc xgraph/expr/expr.c -lm
.PHONY:
leakl: list.c xgraph/expr/expr.c common_symbols.c
	$(CC) -Wall -fsanitize=address -O3 -g list.c common_symbols.c -o list xgraph/expr/expr.c -lm
.PHONY:
debug: calc.c xgraph/expr/expr.c common_symbols.c
	$(CC) -Wall -g calc.c common_symbols.c -o calc xgraph/expr/expr.c -lm
.PHONY:
debugl: list.c xgraph/expr/expr.c common_symbols.c
	$(CC) -Wall -g list.c common_symbols.c -o list xgraph/expr/expr.c -lm
.PHONY:
symtestl: symtest.c common_symbols.o xgraph/expr/expr.c
	$(CC) $(CFLAG) symtest.c common_symbols.o -o symtest xgraph/expr/expr.c -g -fsanitize=address -lm
symtest: symtest.c xgraph/expr/xgraph.a common_symbols.o
	$(CC) $(CFLAG) symtest.c common_symbols.o -o symtest $(LFLAG) -g
sorttest: sorttest.c xgraph/expr/xgraph.a common_symbols.o
	$(CC) $(CFLAG) sorttest.c common_symbols.o -o sorttest $(LFLAG)
xgraph/xgraph.a: xgraph
	make -C xgraph
.PHONY:
systable:
	bash systable.sh >systable.c
.PHONY:
clean:
	rm -f draw calc list wave symtest sorttest common_symbols.o systable.c
.PHONY:
cleanall:
	make clean
	make -C xgraph cleanall
