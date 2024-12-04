#!/bin/bash

if [ $# -lt 1 ]; then
    INPUT=input
else
    INPUT=$1
fi

OLDIFS=$IFS
IFS=$'\n'


part1() {
    MULS=( $(cat $1 | grep -o -E 'mul\([0-9]+,[0-9]+\)') )

    TOTAL=0

    for i in ${MULS[@]}; do
        SUMMAND=$(echo $i | sed -E 's/mul\(//;s/\)//;s/,/\*/')
        TOTAL=$(echo $SUMMAND + $TOTAL | bc)
    done

    echo $TOTAL
}

part2() {
    cat $1 | grep -Eo "mul\([0-9]+,[0-9]+\)|do\(\)|don't\(\)" \
        | sed 's/^mul(\([0-9]\+\),\([0-9]\+\))$/\1 \2/' \
        | awk '
            BEGIN { d = 1 }
            /^do\(.*$/ { d=1; next }
            /^don.*$/ { d = 0; next }
            d { p += $1 * $2 }
            END { printf("%d\n", p) }
            '
}

part1 $INPUT
part2 $INPUT
