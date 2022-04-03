#!/bin/bash
loops=$1
for (( c=0; c< $loops; c++ ))
do
 ./test_scalability 152.3.53.169 10 &
done
