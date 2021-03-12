#!/bin/bash
output_file=benchmark

for i in {1..5}
do
  ./parallel_cal 5 >> $output_file 
done

for i in {1..5}
do
  ./parallel_cal 10 >> $output_file 
done

for i in {1..5}
do
  ./parallel_cal 50 >> $output_file 
done

for i in {1..5}
do
  ./parallel_cal 100 >> $output_file 
done

for i in {1..5}
do
  ./parallel_cal 200 >> $output_file 
done
