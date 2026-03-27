#!/bin/bash

./gossip -t 5000 2195 > tmp4a.txt &
sleep 0.1
./in1.sh | ./gossip -u Leia -t 5000 2196 localhost:2195 > tmp4b.txt &
sleep 0.1
./in2.sh | ./gossip -u Luke -t 5000 2197 localhost:2196 > tmp4c.txt &
sleep 0.1
./in3.sh | ./gossip -u Han -t 5000 2198 localhost:2196 > tmp4d.txt &
sleep 0.1
./in4.sh | ./gossip -u Threepio -t 5000 2199 localhost:2197 > tmp4e.txt &
wait
