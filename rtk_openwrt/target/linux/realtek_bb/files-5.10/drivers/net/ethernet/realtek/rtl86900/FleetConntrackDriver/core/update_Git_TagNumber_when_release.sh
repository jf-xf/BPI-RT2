#!/bin/bash
if [ -d ".git" ]; then
	`git pull`
  	GIT_TAG_COMMENT=$1

	echo $GIT_TAG_COMMENT
	
	#GIT_TAG_BEFORE=`git describe --tags | awk -F'-' '{print $1}'`	
	GIT_TAG_FC_VERSION=`git tag -l | sort -V -r | grep "2." -m 1 | cut -d '.' -f1`
	GIT_TAG_FC_TAG=`git tag -l | sort -V -r | grep "2." -m 1 | cut -d '.' -f2`
	
        GIT_TAG_NEW=$(($GIT_TAG_FC_TAG + 1))
	GIT_TAG_FINAL=$GIT_TAG_FC_VERSION.$GIT_TAG_NEW
	echo $GIT_TAG_FINAL
	#GIT_TAG_AFTER=`echo $GIT_TAG_BEFORE $GIT_TAG_ADD_NUMBER | awk '{print $1 + $2}'`
	#echo $GIT_TAG_AFTER
	if [ -z "$GIT_TAG_COMMENT" ]
	then
	  `git tag $GIT_TAG_FINAL`
 	  git push --tags
	else
	 `git tag -am "$GIT_TAG_COMMENT" $GIT_TAG_FINAL`
	  git push --tags
	fi
	
else
	echo "Not a git repository."
fi
