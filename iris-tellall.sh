#!/bin/sh

BUS=--system

case "$1" in
    --system)	BUS=--system; shift ;;
    --session)	BUS=--session; shift ;;
esac

MSG="$*"

dbus-send $BUS --type=method_call --dest=net.utopiabound.iris / net.utopiabound.iris.TellAll string:"$MSG"
