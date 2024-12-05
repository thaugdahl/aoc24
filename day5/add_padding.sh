#!/bin/bash

if [ $# -lt 1 ]; then
    INPUT=input
else
    INPUT=$1
fi

OUTPUT_FILE=${INPUT}_padded
