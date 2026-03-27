#!/bin/bash

./gossip -t 5000 2187 > tmp1a.txt &
sleep 0.1
./in1.sh | ./gossip -u Leia -t 5000 2188 localhost:2187 > tmp1b.txt &
wait
