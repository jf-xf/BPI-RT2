#!/bin/sh

echo "config sabma password"

echo $2 > /tmp/smb.txt
echo $2 >> /tmp/smb.txt
if [ -x "/usr/sbin/smbpasswd" ]; then
	cat /tmp/smb.txt | smbpasswd -a $1 -s
else
	cat /tmp/smb.txt | pdbedit -a $1 -t
fi
