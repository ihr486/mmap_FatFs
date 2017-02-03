#!/bin/bash
set -Ceu

for i in {1..10}; do
  sudo ./fatfs $(($i*1000)) test${i}a.txt
  sudo ./fatfs $(($i*1000)) test${i}b.txt
  sudo ./fatfs $(($i*1000)) test${i}c.txt
  sudo ./fatfs $(($i*1000)) test${i}d.txt
done
for i in {2..10}; do
  sudo ./fatfs $(($i*10000)) test${i}e.txt
  sudo ./fatfs $(($i*10000)) test${i}f.txt
  sudo ./fatfs $(($i*10000)) test${i}g.txt
  sudo ./fatfs $(($i*10000)) test${i}h.txt
done
