#!/bin/bash
#$(repo forall $1 -c git describe --tags | awk -F'-' '{print $1}' )
if [ -z $(git --git-dir=$1/.git --work-tree=$1/ describe --tags | awk -F'-' '{print $2}') ] ; 
	then echo 0 
else
	echo $(git --git-dir=$1/.git --work-tree=$1/ describe --tags | awk -F'-' '{print $2}')
fi
