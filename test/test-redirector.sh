#!/bin/sh
TERMINAL=gnome-terminal

$TERMINAL -e "redirector ss::8100::8101 mc::8120 sc::8110:localhost:8120 cc:localhost:8101:localhost:8110 cc:localhost:8120:www.bbc.co.uk:80" &
sleep 3
$TERMINAL -e "telnet localhost 8120" &
$TERMINAL -e "telnet localhost 8120" &
sleep 3

wget localhost:8100 -O /dev/null

sleep 5
#killall redirector

