#
# Regular cron jobs for the oedd package
#
0 4	* * *	root	[ -x /usr/bin/oedd_maintenance ] && /usr/bin/oedd_maintenance
