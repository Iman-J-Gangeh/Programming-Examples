#!/bin/bash

./in1.sh | ./gossip -u Leia -t 5000 2191 > tmp3a.txt &
sleep 0.1
./in2.sh | ./gossip -u Luke -t 5000 2192 localhost:2191 > tmp3b.txt &
sleep 0.1
./in3.sh | ./gossip -u Han -t 5000 2193 localhost:2191 > tmp3c.txt &
sleep 0.1
./in4.sh | ./gossip -u Threepio -t 5000 2194 localhost:2191 > tmp3d.txt &
wait
