#
# Regular cron jobs for the capture package
#
0 4	* * *	root	[ -x /usr/bin/capture_maintenance ] && /usr/bin/capture_maintenance
