#!/bin/sh

MEMORY_THRESHOLD=20480
MEMORY_THRESHOLD_FILE="/var/run/.memory_threshold"
MEMORY_INFO=`cat /proc/meminfo | grep "Mem\|^Cached\|Buffers\|Dirty"`

get_memory_info()
{
	local item=$1
	local mem=${MEMORY_INFO/*${item}/}
	if [ ! "${mem}" = "${MEMORY_INFO}" ]; then
		mem=${mem//kB*}
	else
		mem=0
	fi
	echo ${mem}
}

do_drop_cache()
{
	local ac=$1
	if [ "${ac}" = "" ]; then
		ac=1
	fi
	echo "== System Drop Cache, level($ac) =="
	echo "== Memory free(${FREE_MEMORY} KB), threshold(${MEMORY_THRESHOLD} KB) ==" 
	sync
	echo $ac > /proc/sys/vm/drop_caches
}

FREE_MEMORY=`get_memory_info MemFree:`

if [ -f "${MEMORY_THRESHOLD_FILE}" ]; then
	threshold=`cat ${MEMORY_THRESHOLD_FILE}`
	MEMORY_THRESHOLD=${threshold}
fi

if [ "$1" = "force" ]; then
	do_drop_cache 1
elif [ "$1" = "force2" ]; then
	do_drop_cache 3
elif [ ${FREE_MEMORY} -gt ${MEMORY_THRESHOLD} ]; then
	TOTAL_MEMORY=`get_memory_info MemTotal:`
	CACHED_MEMORY=`get_memory_info Cached:`
	BUFFERS_MEMORY=`get_memory_info Buffers:`
	DIRTY_MEMORY=`get_memory_info Dirty:`
	mem=$((${CACHED_MEMORY}+${BUFFERS_MEMORY}+${DIRTY_MEMORY}))
	if [ ${mem} -ge ${FREE_MEMORY} ]; then
		do_drop_cache 1
	fi
else
	do_drop_cache 3
fi
