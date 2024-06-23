#!/bin/bash

pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

# Function to check if the argument is a numeric value and greater than 0
isPositiveInteger() {
    if [[ "$1" =~ ^[1-9][0-9]*$ ]]; then
        return 0
    else
        return 1
    fi
}

# function to check if the argument is a numeric value and greater than or equal to 0
isZeroOrUp() {
    if [[ "$1" =~ ^[0-9][0-9]*$ ]]; then
        return 0
    else
        return 1
    fi
}

# check number of arguments
if [ $# != 2 ] && [ $# != 3 ] && [ $# != 4 ]; then
		 echo "usage: run.sh seedURL maxDepth [-p] [-q] [numThreads]"
		 echo "    seedURL : URL from which to initiate crawl"
		 echo "    maxDepth : maximum depth for crawler"
		 echo "    -p : use parallel indexer"
		 echo "    -q : load queries quietly from file"
		 echo "    numThreads: number of parallel threads to open - should only be present with -p"
		 exit
fi

# check to make sure max depth is a positive integer
if ! isZeroOrUp "$2"; then
		 echo "usage: run.sh seedURL maxDepth [-p] [-q] [numThreads]"
		 echo "    seedURL : URL from which to initiate crawl"
		 echo "    maxDepth : maximum depth for crawler"
		 echo "    -p : use parallel indexer"
		 echo "    -q : load queries quietly from file"
		 echo "    numThreads: number of parallel threads to open - should only be present with -p"
		 exit
fi

# make sure third argument is -p, -q, -qp or -pq if there are 3 or 4 arguments
if [ $# -gt 2 ] && !([ "$3" == "-p" ] || [ "$3" == "-q" ] || [ "$3" == "-qp" ] || [ "$3" == "-pq" ]); then
		echo "usage: run.sh seedURL maxDepth [-p] [-q] [numThreads]"
		echo "    seedURL : URL from which to initiate crawl"
		echo "    maxDepth : maximum depth for crawler"
		echo "    -p : use parallel indexer"
		echo "    -q : load queries quietly from file"
		echo "    numThreads: number of parallel threads to open - should only be present with -p"
		exit
fi

# make sure there is a 4th argument if -p option is activated and check the 4th argument
if [ $# -gt 2 ] && ([ "$3" == "-p" ] || [ "$3" == "-qp" ] || [ "$3" == "-pq" ]); then
		if [ $# != 4 ] || ([ $# == 4 ] && ! isPositiveInteger "$4"); then
			 		 echo "usage: run.sh seedURL maxDepth [-p] [-q] [numThreads]"
					 echo "    seedURL : URL from which to initiate crawl"
					 echo "    maxDepth : maximum depth for crawler"
					 echo "    -p : use parallel indexer"
					 echo "    -q : load queries quietly from file"
					 echo "    numThreads: number of parallel threads to open - should only be present with -p"
					 exit
		fi
fi

# Ensure that a 4th argument only exists with a -p flag
if ([ $# -gt 3 ]) && !([ "$3" == "-p" ] || [ "$3" == "-qp" ] || [ "$3" == "-pq" ]); then
		echo "usage: run.sh seedURL maxDepth [-p] [-q] [numThreads]"
		echo "    seedURL : URL from which to initiate crawl"
		echo "    maxDepth : maximum depth for crawler"
		echo "    -p : use parallel indexer"
		echo "    -q : load queries quietly from file"
		echo "    numThreads: number of parallel threads to open - should only be present with -p"
		exit
fi
		
echo "Building..."

# ensure that the project has been made
makeall.sh -q

echo "Crawling..."

# first run the crawler
pushd ./crawler
./crawler $1 page $2
popd

# get the result from the last command
RESVAL=$?

# make sure crawler implemented successfully
if [ ${RESVAL} != 0 ]; then
		# clean up
		cleanall.sh -q
		rm -rf crawler/page 2>/dev/null >/dev/null
		
		echo "usage: run.sh seedURL maxDepth [-p] [-q] [numThreads]"
		echo "    seedURL : URL from which to initiate crawl"
		echo "    maxDepth : maximum depth for crawler"
		echo "    -p : use parallel indexer"
		echo "    -q : load queries quietly from file"
		echo "    numThreads: number of parallel threads to open - should only be present with -p"
		exit
fi

echo "Indexing..."

# run the indexer (parallel if -p flag provided)
if [ $# == 4 ] && ([ "$3" == "-p" ] || [ "$3" == "-qp" ] || [ "$3" == "-pq" ]); then
		pushd ./parallel_indexer
		./indexer ../crawler/page index $4
		popd
else
		pushd ./indexer
		./indexer ../crawler/page index
		popd
fi;

# get the result from the last command
RESVAL=$?

# make sure crawler implemented successfully
if [ ${RESVAL} != 0 ]; then
 		# clean up
		cleanall.sh -q
		rm -rf crawler/page 2>/dev/null >/dev/null
		rm indexer/index 2>/dev/null >/dev/null
		rm parallel_indexer/index 2>/dev/null >/dev/null
		
		echo "usage: run.sh seedURL maxDepth [-p] [-q] [numThreads]"
		echo "    seedURL : URL from which to initiate crawl"
		echo "    maxDepth : maximum depth for crawler"
		echo "    -p : use parallel indexer"
		echo "    -q : load queries quietly from file"
		echo "    numThreads: number of parallel threads to open - should only be present with -p"
		exit
fi

echo "Begin queries..."

# run the querier (quiet if -q flag provided)
if ([ $# -gt 2 ]) && ([ "$3" == "-q" ] || [ "$3" == "-qp" ] || [ "$3" == "-pq" ]); then
		pushd ./querier

		# Check is parallel indexer was run
		if [ $# == 4 ] && ([ "$3" == "-p" ] || [ "$3" == "-qp" ] || [ "$3" == "-pq" ]); then
				./query ../crawler/page ../parallel_indexer/index -q
		else
				./query ../crawler/page ../indexer/index -q
		fi
		
		popd
else
		pushd ./querier

		# Check is parallel indexer was run
		if [ $# == 4 ] && ([ "$3" == "-p" ] || [ "$3" == "-qp" ] || [ "$3" == "-pq" ]); then
				./query ../crawler/page ../parallel_indexer/index
		else
				./query ../crawler/page ../indexer/index
		fi

		popd
fi;

# get the result from the last command
RESVAL=$?

# make sure crawler implemented successfully
if [ ${RESVAL} != 0 ]; then
 		# clean up
		cleanall.sh -q
		rm -rf crawler/page 2>/dev/null >/dev/null
		rm indexer/index 2>/dev/null >/dev/null
		rm parallel_indexer/index 2>/dev/null >/dev/null
		
		echo "usage: run.sh seedURL maxDepth [-p] [-q] [numThreads]"
		echo "    seedURL : URL from which to initiate crawl"
		echo "    maxDepth : maximum depth for crawler"
		echo "    -p : use parallel indexer"
		echo "    -q : load queries quietly from file"
		echo "    numThreads: number of parallel threads to open - should only be present with -p"
		exit
else
		# clean up
		echo "Cleaning..."
		cleanall.sh -q
		rm -rf crawler/page 2>/dev/null >/dev/null
		rm indexer/index 2>/dev/null >/dev/null
		rm parallel_indexer/index 2>/dev/null >/dev/null
fi;

