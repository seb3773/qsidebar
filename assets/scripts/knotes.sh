#!/bin/sh
r=$(dcop knotes KNotesIface newNote "" "")
if echo "$r" | grep -qv "not accessible"; then
dcop knotes KNotesIface resize "$r" 400 300
fi

