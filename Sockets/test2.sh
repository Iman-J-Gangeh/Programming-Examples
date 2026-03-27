#!/bin/bash

./in1.sh | ./gossip -u Leia -t 5000 2189 > tmp2a.txt &
sleep 0.1
./in2.sh | ./gossip -u Luke -t 5000 2190 localhost:2189 > tmp2b.txt &
wait
