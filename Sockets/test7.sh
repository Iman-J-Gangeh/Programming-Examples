#!/bin/bash

./gossip -t 1000 2187 &
sleep 0.1
./gossip -t 1000 2187 &> tmp7.txt &
wait
