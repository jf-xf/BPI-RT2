#!/bin/bash
if [ -d ".git" ]; then
  # Control will enter here if $DIRECTORY exists.
  	`git pull`
	GIT_TAG_BEFORE=`git describe --tags | awk -F'-' '{print $1}'`
	#echo "GIT_TAG_BEFORE = $GIT_TAG_BEFORE"
	GIT_TAG_AFTER=$(($GIT_TAG_BEFORE + 1))
	#echo "GIT_TAG_AFTER = $GIT_TAG_AFTER"
	`git tag $GIT_TAG_AFTER`
	git push --tags
else
	echo "Not a git repository."
fi