#!/bin/bash

echo "echo 0"
echo 0 > /dev/fifo_proc

echo "echo 1"
echo 1 > /dev/fifo_proc

echo "echo 2"
echo 2 > /dev/fifo_proc

echo "result of cat"
cat /dev/fifo_proc
