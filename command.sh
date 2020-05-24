#!/bin/bash
g++ -I../console ../console/console.linux.cpp ../console/advancedConsole.cpp ide.cpp -lncurses -lpthread -fpermissive -w -g 2> out.txt
if [ $? -neq 0 ]
  then
    less out.txt
  else
    ./a.out
fi
