#!/bin/bash

filetype="wav"
#filename="record_$(date +%H_%M_%m_%d_%Y)"
directory="$HOME/Musica/recordings/"

if ! [ -d "$directory" ];
then
    mkdir "$directory"
fi

if [[ $1 == "start" ]] && [[ $# -eq 2 ]]
then
	filename=$2
        notify-send "Recording started"
        amixer sset 'Capture' 35%
        amixer sset 'Master' 0%
        exec arecord -f cd -d 3600 -t wav |lame --preset 56 -mm - "$directory""$filename" &
elif [[ $1 == "stop" ]]
then
        pkill -f "arecord" && notify-send "Recording stopped"
fi


