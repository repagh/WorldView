#!/bin/bash

# This nifty command gets the dir name we are running in
DIRECTORY=${PWD##*/}

# Default for all non-configured platforms
CAMERA_CONFIG=test_camera.xml
LOGLEVEL=info
LOGCLASS=all
CS=test

# check that the protocol is installed in flightgear
if [ -n "${FG_ROOT}" ]; then
    if [ ! -f ${FG_ROOT}/Protocol/duecavis.xml ]; then
	echo "Please install duecavis.xml in your FG_ROOT ${FG_ROOT}/Protocol"
	exit 1
    fi
elif [ -d /usr/share/flightgear ]; then
    if [ ! -f /usr/share/flightgear/Protocol/duecavis.xml ]; then
	echo "Please install duecavis.xml in /usr/share/flightgear/Protocol"
	exit 1
    fi
elif  [ -d /usr/share/games/flightgear ]; then
    if [ ! -f /usr/share/games/flightgear/Protocol/duecavis.xml ]; then
	echo "Please install duecavis.xml in /usr/share/games/flightgear/Protocol"
	exit 1
    fi
else
    echo "Cannot find your flightgear root, please set FG_ROOT"
    exit 1
fi

# On the lab, cameras/projectors all have different FOV/frustum and offset
if [ "$DIRECTORY" == "dutmms14" ]; then
    CAMERA_CONFIG=hmilab-camera-front.xml
    LOGLEVEL=alert
    CS=mms14
fi

if [ "$DIRECTORY" == "dutmms15" ]; then
    CAMERA_CONFIG=hmilab-camera-left.xml
    LOGLEVEL=alert
    CS=mms15
fi

if [ "$DIRECTORY" == "dutmms16" ]; then
    CAMERA_CONFIG=hmilab-camera-right.xml
    LOGLEVEL=alert
    CS=mms16
fi

UDP_PORT=5501
UDP2_PORT=5502

BGAMP="&"
if [ "$1" = 'nodueca' ]; then
    BGAMP=""
fi

#    --generic=socket,out,100,127.0.0.1,${UDP2_PORT},udp,duecavis \
fgfs \
    --generic=socket,in,100,127.0.0.1,${UDP_PORT},udp,duecavis \
    --generic=socket,out,100,127.0.0.1,${UDP2_PORT},udp,duecavis \
    --callsign=${CS} \
    --multiplay=in,100,127.0.0.1,5001 \
    --prop:int:/sim/multiplay/debug-level=0 \
    --airport=EHAM \
    --aircraft=A380  \
    --fdm=external \
    --disable-real-weather-fetch \
    --start-date-lat=2024:08:23:14:00:00 \
    --enable-clouds3d \
    --prop:bool:/sim/menubar/visibility=false \
    --enable-splash-screen \
    --enable-terrasync \
    --disable-random-objects \
    --disable-random-buildings \
    --disable-random-vegetation \
    --disable-sound \
    --disable-rembrandt \
    --disable-distance-attenuation \
    --fog-nicest \
    --enable-specular-highlight \
    --prop:/local-weather/autostart=0 \
    --prop:bool:/sim/menubar/visibility=false \
    --disable-mouse-pointer \
    --disable-save-on-exit \
    --disable-splash-screen  \
    --log-level=$LOGLEVEL \
    --log-class=$LOGCLASS \
    --log-dir="." \
    --enable-ai-models \
    --disable-ai-traffic $BGAMP

if [ "$1" = 'nodueca' ]; then
    echo "not starting dueca"
else
    ./dueca_run.x

    killall fgfs
fi

# --metar="XXXX 012345Z 15001KT 0800 BKN02 OVC005 OVC020 08/06 Q0990" \