#!/bin/sh
# param: msec proc pidfile
_exit(){
	#echo "Got SIGNAL!"
	if [ "$pid" ]; then
		#echo "kill $!"
		kill $pid
	fi
	rm $pid_file
	exit 1
}
trap _exit SIGTERM

mseconds=$1
proc_file=$2
pid_file=$3
seconds=$((mseconds/1000))

# prevent timer process is still there
if [ -f "$pid_file" ]; then
	kill -SIGTERM $(cat $pid_file)
fi
# echo pid to file
echo $$ > $pid_file
# wait for seconds
sleep $seconds &
pid=$!
wait $pid
# echo 0 to proc led brightness
echo 0 > $proc_file

# rm pid file
rm $pid_file
