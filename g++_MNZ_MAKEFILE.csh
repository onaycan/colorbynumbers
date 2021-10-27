#!/bin/csh -f
echo Deleting the old data...
rm MNZ_MASTER.cpp MNZ_MASTER.exe
echo Combining the cpp-s...
cat MNZ_main.cpp >>MNZ_MASTER.cpp

if ( $#argv > 0 ) then
    set source = "$argv[1]"
else
    set source = "MNZ_MASTER"
endif

echo Compiling...
g++ -o ./$source.exe ./$source.cpp -O2 -L/usr/X11R6/lib -lm -lpthread -lX11


