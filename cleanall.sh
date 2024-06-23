#!/bin/bash

pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

# check arguments
if [ $# != 1 ] && [ $# != 0 ]; then
		echo "usage: cleanall.sh [ -s ] [ -q ]"
		echo "   -s : show compilation output"
		echo "   -q: : build quietly"
		exit
fi

# check the argument
if [ $# == 1 ] && !([ $1 == "-q" ] || [ $1 == "-s" ] || [ $1 == "-sq" ] || [ $1 == "-qs" ]); then
		echo "usage: cleanall.sh [ -s ] [ -q ]"
		echo "   -s : show compilation output"
		echo "   -q: : build quietly"
		exit
fi

# quiet by default
MODE=-s
if [ $# == 1 ] && ([ $1 == "-s" ] || [ $1 == "-sq" ] || [ $1 == "-qs" ]); then
		unset MODE
fi

# make all of the components
for DIR in utils crawler indexer querier parallel_indexer
do

		if [ -d ./${DIR} ] ; then
				pushd ./${DIR}
				if [ $# == 1 ] && ([ $1 == "-q" ] || [ $1 == "-sq" ] || [ $1 == "-qs" ]); then
						make ${MODE} clean > /dev/null
				else
						echo [$DIR]
						make ${MODE} clean
				fi
				popd
		fi

done
