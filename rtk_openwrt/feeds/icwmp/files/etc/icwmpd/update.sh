#!/bin/sh
. /lib/functions.sh

log() {
	echo "$@" |logger -t cwmp.update -p info
}

handle_icwmp_update() {
	local cwmp_enable
	config_load cwmp

	config_get_bool cwmp_enable cpe enable 1
	if [ "$cwmp_enable" = "0" ]; then
		return 0
	fi

	status="$(ubus call tr069 status |jsonfilter -qe '@.last_session.status')"
	if [ "$status" != "running" ]; then
		log "Trigger out of bound inform, since last inform status was failure"
		ubus -t 10 call tr069 inform >/dev/null 2>&1
		# Handle timeout or tr069 object not found
		if [ "$?" -eq 7 ] || [ "$?" -eq 4 ]; then
			log "Restarting icwmp tr069 object"
			/etc/init.d/icwmpd restart
		fi
	fi
}

handle_icwmp_update $@
