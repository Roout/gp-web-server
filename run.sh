#!/bin/bash

for ((i=1; i<5; i++))
do
	for val in "POST" "GET" "PUT"
	do
		curl -X $val http://localhost:18001 &
	done
done

