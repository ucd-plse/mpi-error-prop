#!/bin/sh

#./run-one-test.sh dirname

path="/afs/cs.wisc.edu/u/c/r/crubio/trunk"

rm -f $1/*.out $1/*.wpds $/*.passed

gcc -E -o $1/source.i $1/source.c

for mode in transfer copy; do
    # error transformation enabled
    $path/cil/obj/x86_LINUX/cilly.asm.exe --dowpds --negative --$mode --transf --queryfile $1/$mode.wpds --schema $path/post/wpds-xml/error-prop.xsd --functionfile $1/entry-points.txt --error-codes $path/error-codes.txt $1/source.i

    xmllint --noout --schema $path/post/wpds-xml/error-prop.xsd $1/$mode.wpds 2>/dev/null

    # running dereference analysis
    $path/post/main/main --error-codes=$path/error-codes.txt --temps --tentative --dereference --query=$1/$mode.wpds > $1/$mode.out
done
