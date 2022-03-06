#!/bin/bash

rootdir=~/HW3/ChampSim_CRC2
policy=adaptive-ship-lru
binname=adaptive-ship-lru-m1
# 10 mil
warmup=10000000
# 100 mil
sim=100000000
# location of traces
tracesource=/datasets/cs240c-wi22-a00-public/data/Assignment2-gz/
# output folder
outfolder=output_adaptive-ship-lru

# test file
enable_test_withprefetch=false
test_warmup=1000000
test_sim=10000000
test_trace=$rootdir/trace/bzip2_10M.trace.gz

# flags
enable_withprefetch=true

# Single-core with prefetcher
if [ "$enable_withprefetch" = true ] || [ "$enable_test_withprefetch" = true ] ; then
    echo "Compiling $binname, 1-core, with L1/L2 prefetchers"
    echo "g++ -Wall --std=c++11 -o $rootdir/bin/$binname-config2 $rootdir/$policy/$binname.cc $rootdir/lib/config2.a"
    g++ -Wall --std=c++11 -o $rootdir/bin/$binname-config2 $rootdir/$policy/$binname.cc $rootdir/lib/config2.a
fi

# Test run
if [ "$enable_test_withprefetch" = true ] ; then
    echo "Test run with L1/L2 prefetchers of $binname-config2"
    folder="$rootdir/trace/"
    cleantrace=${test_trace#$folder}
#output="$binname-config2-w$test_warmup-s$test_sim-$cleantrace.txt"
    output="testtrace1.txt"
    rm -f $outfolder/$output
    echo "$rootdir/bin/$binname-config2 -warmup_instructions $test_warmup -simulation_instructions $test_sim -traces $test_trace > $outfolder/$output"
    $rootdir/bin/$binname-config2 -warmup_instructions $test_warmup -simulation_instructions $test_sim -traces $test_trace > $outfolder/$output
fi

# config2: 1-core, L1/L2 prefetch
if [ "$enable_withprefetch" = true ] ; then
  for trace in ${tracesource}*.trace.gz; do
    # Chop off the "/datasets/cs240c-wi22-a00-public/data/Assignment2-gz/"
    cleantrace=${trace#$tracesource}
    output="$binname-config2-w$warmup-s$sim-$cleantrace.txt"
    if ! [ -f "$outfolder/$output" ] ;then
      echo "$rootdir/bin/$binname-config2 -warmup_instructions $warmup -simulation_instructions $sim -traces $trace > $outfolder/$output"
      $rootdir/bin/$binname-config2 -warmup_instructions $warmup -simulation_instructions $sim -traces $trace > "$outfolder/$output" &
    fi
  done
fi
