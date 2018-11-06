#!/bin/bash

audiopath="$HOME/Musica/recordings/"

if [[ $# -lt 1 ]]
then
	xournalpp -f $audiopath
else
	xournalpp -f $audiopath "$1"
fi
