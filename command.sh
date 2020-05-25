#!/bin/bash
g++ -I../console ../console/console.linux.cpp ../console/advancedConsole.cpp ide.cpp -lncursesw -lpthread -fpermissive -w -g 2> out.txt
if [ $? -eq 0 ]
  then
    ./a.out
  else
    less out.txt
fi
