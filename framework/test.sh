#!/bin/bash

source /home/sramesh/MOCHI/sourceme.sh

ppn=10
MANUAL=0

if [ "$MANUAL" == "0" ]
then
    hostfile=hosts-$SLURM_JOBID
    rm -f $hostfile
    
    for i in `scontrol show hostnames $SLURM_NODELIST`
    do
      for (( j=0; j<$ppn; j++ ))
      do
        echo $i>>$hostfile
      done
    done
    
    nprocs=`wc -l $hostfile | awk '{ print $1 }'`
else
    SLURM_JOBID=whoami
    hostfile=hosts-$SLURM_JOBID
fi

	#Run test
        echo "========================================="
        echo "Running experiment...."
        echo "========================================="
        echo "Running with "$nprocs" client processes"

        date1=$(($(date +%s%N)/1000000))

        echo "Running now....."
	mpirun -np $nprocs -envall -f $hostfile -ppn $ppn ./client 2

            sleep 1
        date2=$(($(date +%s%N)/1000000))
        diff=$(($date2-$date1))
        echo "Total Wall Time: $date2  -  $date1 = $diff millisecs  <======"
        echo "#########################################"
#Cleanup
rm -f $hostfile

