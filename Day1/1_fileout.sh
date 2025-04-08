#!/bin/bash

#echo "Hello Workd!"

#word="Hello Workd!"
#echo $word

for file in $(ls `pwd`); do
	echo "${file}"
done
