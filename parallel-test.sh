#!/bin/bash

if test "$1" == "" ; then
	echo "Usage: parallel-test.sh file1 [file2 ...]"
	echo ""
	exit
fi


i=0
for filename in "$@"
do
	if test "$filenames" != "" ; then
		filenames="$filenames $filename"
	else
		filenames="$filename"
	fi
	let "i += 1"
	echo "========================"
	if test $i == 1 ; then
		echo -e "\033[1m$i File:\033[0m"
	else
		echo -e "\033[1m$i Files:\033[0m"
	fi

	echo "(sort -S 1M $filenames)"
	echo "========================"
	
	current_time=`(time -p src/sort -S 200K -m $filenames >out.dev 2>/dev/null) 2>&1 | grep real | sed 's/real *\([0-9]*\.[0-9]*\)/\1/'`
	echo "Development Version: $current_time seconds"

	
	release_time=`(time -p sort -S 200K -m $filenames >out.release 2>/dev/null) 2>&1 | grep real | sed 's/real *\([0-9]*\.[0-9]*\)/\1/'`
	echo "Release Version: $release_time seconds"
	echo ""
	
	
	improvement_over_release=`echo "scale=2; (1 - $current_time / $release_time) * 100.0" | bc`
	echo "Improvement over Release Version: $improvement_over_release%"
	
	if [ "`diff -q out.dev out.release`" != "" ]; then
		tput setaf 1
		echo "Development output does not match Release output!"
		tput op
	fi
	
	
	echo ""
done

rm out.dev
rm out.old
rm out.release
