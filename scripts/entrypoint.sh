#!/bin/bash

export PATH="$PATH:/opt/go/bin"

case "$1" in
    mpich-bugs)
        PHASE="$2"
        /scripts/injection/fixedbitcode.sh phase${PHASE} /results/mpich_phase${PHASE}_fixes.txt mpich

        /mpi-error-prop/post/irllvm-wpds-xml/frontend \
            -c /scripts/standard.cfg \
            -o /d/mpich_wpds.xml \
            -b /d/mpich-install-phase${PHASE}-none-return/lib/libmpi.so.12.1.6.bc 1>&2

        /mpi-error-prop/post/main/compress-wpds /d/mpich_wpds.xml > /d/mpich_wpds_c.xml

        # Run error-prop on the WPDS
        /mpi-error-prop/post/main/main \
        --verbose \
        --error-codes=/mpi-error-prop/error-codes/mpi-ec-redefined.txt \
        --tentative \
        --query=/d/mpich_wpds_c.xml 2>&1> /d/mpich_reports.txt

        # Parse the error-prop results into a csv.
	    cd /d
	    python3 /scripts/report_parser.py /d/mpich_reports.txt 1>&2
	    cat /d/parsed_unsaved.csv

        ;;

    mvapich-bugs)
        PHASE="$2"
        /scripts/injection/fixedbitcode.sh phase${PHASE} /results/mvapich_phase${PHASE}_fixes.txt mvapich
        
        /mpi-error-prop/post/irllvm-wpds-xml/frontend \
            -c /scripts/standard.cfg \
            -o /d/mvapich_wpds.xml \
            -b /d/mvapich-install-phase${PHASE}-none-return/lib/libmpi.so.12.1.1.bc 1>&2

        /mpi-error-prop/post/main/compress-wpds /d/mvapich_wpds.xml > /d/mvapich_wpds_c.xml

        # Run error-prop on the WPDS
        /mpi-error-prop/post/main/main \
        --verbose \
        --error-codes=/mpi-error-prop/error-codes/mpi-ec-redefined.txt \
        --tentative \
        --query=/d/mvapich_wpds_c.xml 2>&1> /d/mvapich_reports.txt

        # Parse the error-prop results into a csv.
	    cd /d
	    python3 /scripts/report_parser.py /d/mvapich_reports.txt 1>&2
	    cat /d/parsed_unsaved.csv
        ;;

    fault-injection)
	/scripts/injection/fault-injection.sh $2 $3 $4 $5 $6 $7 $8 $9
	;;
esac
