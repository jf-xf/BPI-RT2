#!/bin/sh
if [ ! -f /var/config/scfg.lock ]; then
	echo "[scfg.txt]Copy /etc/scfg.txt to /var/config/."
    if [ -f  /etc/scfg.txt ]; then
	cp /etc/scfg.txt /var/config/
    fi
    if [ -f    /etc/default_scfg.txt ]; then
	cp /etc/default_scfg.txt /var/config/
    fi
    if [ -f /etc/default_scfg.bak ]; then
	cp /etc/default_scfg.bak /var/config/
    fi
else
	echo "[scfg.txt]scfg.lock exists, no replace /var/config/scfg.txt, default_scfg.txt, default_scfg.bak"
fi

if [ ! -d /var/config/serdes ];then
	mkdir -p /var/config/serdes
fi

if [ ! -f  /var/config/serdes.lock ]; then
        rm -f /var/config/serdes/*
	cp -f /etc/serdes/* /var/config/serdes/
else
	echo "[serdes txt]serdes.lock exists, no replace /var/config/serdes file"
fi
