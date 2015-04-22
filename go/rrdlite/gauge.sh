#!/bin/sh
alias rrdtool='/opt/rrdtool-1.5.0-rc2/bin/rrdtool'
START=1425900000
STEP=10
HEARTBEAT=500
rrdtool create ./gauge.rrd -b $START -s $STEP DS:idle:GAUGE:$HEARTBEAT:U:U RRA:AVERAGE:0.5:5:10

#rrdtool update ./gauge.rrd 1425900010:1
#rrdtool update ./gauge.rrd 1425900020:1
#rrdtool update ./gauge.rrd 1425900030:1
#rrdtool update ./gauge.rrd 1425900040:1
#rrdtool update ./gauge.rrd 1425900050:1
#
#rrdtool  dump ./gauge.rrd  | tee gauge.dump

#rrdtool update ./gauge.rrd 1425900360:98.6
#rrdtool update ./gauge.rrd 1425900420:98.7

#echo run update ./gauge.rrd 1425900120:98.2
#echo run update ./gauge.rrd 1425900180:98.3
#echo run update ./gauge.rrd 1425900240:98.4
#echo run update ./gauge.rrd 1425900300:98.5
#echo run update ./gauge.rrd 1425900360:98.6
#echo run update ./gauge.rrd 1425900420:98.7
ddd
#gdb
#rrdtool update ./gauge.rrd 1425900300:98.5
#rrdtool update ./gauge.rrd 1425900360:98.6
#rrdtool update ./gauge.rrd 1425900420:98.7

#rrdtool fetch ./gauge.rrd 

