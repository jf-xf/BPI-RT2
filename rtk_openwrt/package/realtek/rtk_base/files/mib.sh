#!/bin/sh 

. /lib/functions.sh

DATABASE="rtkmib"


mib_load()
{
	config_load $DATABASE
	if [ ! $? -eq 0 ]; then
		echo "Cannot load database"
		exit 1
	fi
}

mib_get()
{
	local val
	local name=$1
	config_get val "mib" "$name" "--NOT_VAL--"
	if [ ! $val = "--NOT_VAL--" ]; then
		echo "$val"
		return 0
	fi
	return 1
}

mib_set()
{
	local val=$2
	local name=$1
	mib_get $name > /dev/null
	if [ $? -eq 0 ]; then
		config_set "mib" "$name" "$val"
		uci_set $DATABASE "mib" $name $val &> /dev/null
		if [ $? -eq 0 ]; then
			return 0
		fi
	fi
	return 1
}

mib_commit()
{
	uci_commit $DATABASE
}

dump_mib()
{
	echo $@
}

mib_all_cs()
{
	config_cb()
	{
		option_cb()
		{
			echo "$1=$2"
		}
	}
	mib_load
}

ret=1
case "$1" in
	"get")
		if [ -z $2 ]; then
			return -1
		fi
		mib_load
		name=$2
		val=$(mib_get $name)
		if [ $? -eq 0 ]; then
			echo "$name=$val"
			ret=0
		fi
		;;
	
	"set")
		if [ -z $2 -o -z $3 ]; then
			return -1
		fi
		mib_load
		name=$2
		val=$3
		mib_set $name $val > /dev/null
		if [ $? -eq 0 ]; then
			echo "set $name=$val"
			ret=0
		fi
		;;
		
	"all")
		mib_all_cs
		;;
	
	"commit")
		mib_commit
		;;
esac

exit $ret
