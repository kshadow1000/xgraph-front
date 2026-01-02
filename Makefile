CC := gcc
CFLAG := -Wall -O3
LFLAG := xgraph/lib/xgraph.a -lc -lm
all: draw calc list wave
draw: main.c xgraph/lib/xgraph.a xgraph/header/xdraw.h xgraph common_symbols.c
	$(CC) $(CFLAG) main.c common_symbols.c -o draw $(LFLAG)
calc: calc.c xgraph/lib/xgraph.a xgraph/header/expr.h xgraph common_symbols.c
	$(CC) $(CFLAG) calc.c common_symbols.c -o calc $(LFLAG)
.PHONY:
leak: calc.c xgraph/expr.c xgraph/header/expr.h xgraph common_symbols.c
	$(CC) -Wall -fsanitize=address -g calc.c common_symbols.c -o calc xgraph/expr.c -lm
.PHONY:
debug: calc.c xgraph/expr.c xgraph/header/expr.h xgraph common_symbols.c
	$(CC) -Wall -g calc.c common_symbols.c -o calc xgraph/expr.c -lm
list: list.c xgraph/lib/xgraph.a xgraph/header/expr.h xgraph common_symbols.c
	$(CC) $(CFLAG) list.c common_symbols.c -o list $(LFLAG)
symtest: symtest.c xgraph/lib/xgraph.a xgraph/header/expr.h xgraph common_symbols.c
	$(CC) $(CFLAG) symtest.c common_symbols.c -o symtest $(LFLAG)
sorttest: sorttest.c xgraph/lib/xgraph.a xgraph/header/expr.h xgraph common_symbols.c
	$(CC) $(CFLAG) sorttest.c common_symbols.c -o sorttest $(LFLAG)
wave: wave.c xgraph/lib/xgraph.a
	$(CC) $(CFLAG) -DTEXT_ENABLED wave.c xgraph/lib/xgraph.a -o wave $(LFLAG)
xgraph:
	make -C xgraph
xgraph/expr.c:
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
clean:
	rm -f draw calc list wave
.PHONY:
cleanall:
	make clean
	make -C xgraph cleanall
