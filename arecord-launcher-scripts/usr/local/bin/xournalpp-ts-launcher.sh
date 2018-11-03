#!/bin/bash

record_audio()
{
    directory="$HOME/Musica/recordings/"

    if ! [ -d "$directory" ];
    then
        mkdir "$directory"
    fi

    xournalpp-ts -f $directory $1
}

main()
{
       record_audio $1
}

main "$@"
