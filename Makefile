CC := gcc
CFLAG := -Wall -O3
LFLAG := xgraph/lib/xgraph.a -lc -lm
all: draw calc list wave
draw: main.c xgraph/lib/xgraph.a xgraph/header/xdraw.h xgraph common_symbols.o
	$(CC) $(CFLAG) main.c common_symbols.o -o draw $(LFLAG)
calc: calc.c xgraph/lib/xgraph.a xgraph/header/expr.h xgraph common_symbols.o
	$(CC) $(CFLAG) calc.c common_symbols.o -o calc $(LFLAG)
common_symbols.o: xgraph/lib/xgraph.a xgraph/header/expr.h xgraph common_symbols.c
	rm -f systable.c
	make systable
	$(CC) $(CFLAG) common_symbols.c -c -o common_symbols.o
.PHONY:
leak: calc.c xgraph/expr.c common_symbols.c
	$(CC) -Wall -fsanitize=address -O3 -g calc.c common_symbols.c -o calc xgraph/expr.c -lm
.PHONY:
leakl: list.c xgraph/expr.c common_symbols.c
	$(CC) -Wall -fsanitize=address -O3 -g list.c common_symbols.c -o list xgraph/expr.c -lm
.PHONY:
debug: calc.c xgraph/expr.c common_symbols.c
	$(CC) -Wall -g calc.c common_symbols.c -o calc xgraph/expr.c -lm
.PHONY:
debugl: list.c xgraph/expr.c common_symbols.c
	$(CC) -Wall -g list.c common_symbols.c -o list xgraph/expr.c -lm
list: list.c xgraph/lib/xgraph.a xgraph/header/expr.h xgraph common_symbols.o
	$(CC) $(CFLAG) list.c common_symbols.o -o list $(LFLAG)
symtest: symtest.c xgraph/lib/xgraph.a xgraph/header/expr.h xgraph common_symbols.o
	$(CC) $(CFLAG) symtest.c common_symbols.o -o symtest $(LFLAG)
sorttest: sorttest.c xgraph/lib/xgraph.a xgraph/header/expr.h xgraph common_symbols.o
	$(CC) $(CFLAG) sorttest.c common_symbols.o -o sorttest $(LFLAG)
wave: wave.c xgraph/lib/xgraph.a
	$(CC) $(CFLAG) -DTEXT_ENABLED wave.c xgraph/lib/xgraph.a -o wave $(LFLAG)
xgraph:
	make -C xgraph
xgraph/xdraw.c:
	make -C xgraph
xgraph/lib/xgraph.a: xgraph/expr.c xgraph/xdraw.c
	make -C xgraph
xgraph/header/xdraw.h:
	make -C xgraph
xgraph/header/expr.h:
	make -C xgraph
.PHONY:
systable:
	bash systable.sh >systable.c
.PHONY:
clean:
	rm -f draw calc list wave common_symbols.o systable.c
.PHONY:
cleanall:
	make clean
	make -C xgraph cleanall
