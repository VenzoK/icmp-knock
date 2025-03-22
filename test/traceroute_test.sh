#!/bin/bash

TARGET=$1
MAX_HOPS=30

echo "Running icmp-knock utility..."
./icmp-knock $TARGET > icmp-knock_output.txt

echo "Running traceroute..."
traceroute -I -m $MAX_HOPS $TARGET > traceroute_output.txt
echo "icmp-knock output:"
cat icmp-knock_output.txt
echo "traceroute output:"
cat traceroute_output.txt
rm icmp-knock_output.txt traceroute_output.txt

