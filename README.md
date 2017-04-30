IRIS Messenging Daemon
==

This implements a way to notify listing Bonjour Jabber clients on a
network of events.

This accepts dbus messages and passes them along to all available users.

Messages can be formatted with HTML

$ iris-tellall "<b>NAGIOS:</b> host <i>router</i> lost"
