#!/bin/sh

grep -f $1 $2 | ( \
while read l ; do \
	n=${l%%=*}; \
	v=${l#*=}; \
	case $v in \
		y) echo "#define $n 1" ;; \
		m) echo "#define ${n}_MODULE 1" ;; \
		\"*) echo "#define $n $v" ;; \
		[0-9]*) echo "#define $n $v" ;; \
		*) echo "#warning unknown value for $n";;\
	esac; \
done;\
) > $3
