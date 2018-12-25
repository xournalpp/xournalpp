#!/bin/bash

if [[ $1 == "start" ]] && [[ $# -eq 2 ]]
then
	filename=$(echo $2|rev|cut -d'/' -f1|rev)
	directory=$(echo $2|rev|cut -d'/' -f2-|rev)
	directory=$directory/
	vol=35

	#exec &> ${directory}${filename}.log  #debug

	if ! [ -d "$directory" ];
	then
	    mkdir -p "$directory"
	fi

        notify-send "Recording started with ${vol}% gain"
        amixer sset 'Capture' ${vol}%
        amixer sset 'Master' 0%
	dst="${directory}${filename}"
        arecord -f cd -t wav |lame --preset 56 -mm - "$dst" &
	#cvlc alsa://default --sout "#transcode{acodec=mp3,ab=128,channels=2, samplerate=44100}:duplicate{dst=std{access=file,mux=mp3,dst=\"$dst\"}}}" &

elif [[ $1 == "stop" ]]
then
        pkill -f "arecord" && notify-send "Recording stopped"
	#killall vlc && notify-send "Recording stopped"
fi


