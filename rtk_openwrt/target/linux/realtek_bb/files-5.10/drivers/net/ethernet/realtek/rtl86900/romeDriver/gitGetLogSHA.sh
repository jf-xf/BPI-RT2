#!/bin/bash
if [ -z $(git -C $1 log --pretty="format:%h" -1 -- .) ] ;
	then echo 0
else
	echo $(git -C $1 log --pretty="format:%h" -1 -- .)
fi
