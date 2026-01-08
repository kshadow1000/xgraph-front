#! /bin/bash
for i in {1..100}
do
	gcc -Wall -DEXPR_SYMNEXT=$i symtest.c common_symbols.c -o symtest xgraph/expr.c -lc -lm
	v=1
	for j in {1..20}
	do
		x="$(./symtest 100 0 2>/dev/null)"
		v="$(./calc "$v+$x")"
	done
	echo -ne "$(./calc "floor($v)")" '\t'
	echo "EXPR_SYMNEXT=$i"

done
