#!/bin/bash

pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

# check arguments
if [ $# != 1 ] && [ $# != 0 ]; then
		echo "usage: makeall.sh [ -s ]"
		echo "   -h : help"
		exit
fi

# quiet by default
MODE=-s
if [ $# == 1 ] && [ $1 == "-s" ]; then
		unset MODE
fi

# revert to the tse directory
cd ..

# make all of the components
for DIR in utils crawler indexer querier parallel_indexer
do

		if [ -d ./${DIR} ] ; then
				pushd ./${DIR}
				echo [$DIR]
				make ${MODE} clean
				popd
		fi

done

