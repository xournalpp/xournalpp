#!/bin/bash

filetype="wav"
#filename="record_$(date +%H_%M_%m_%d_%Y)"
#directory="$HOME/Musica/recordings/"

if [[ $1 == "start" ]] && [[ $# -eq 2 ]]
then
	filename=$(echo $2|rev|cut -d'/' -f1|rev)
	directory=$(echo $2|rev|cut -d'/' -f2-|rev)
	directory=$directory/

	if ! [ -d "$directory" ];
	then
	    mkdir "$directory"
	fi

	#exec &> ${directory}${filename}.log  #debug

        notify-send "Recording started"
        amixer sset 'Capture' 35%
        amixer sset 'Master' 0%
        arecord -f cd -t wav |lame --preset 56 -mm - "$directory""$filename" &
elif [[ $1 == "stop" ]]
then
        pkill -f "arecord" && notify-send "Recording stopped"
fi


