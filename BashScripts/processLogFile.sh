#!/bin/bash

if [ "$#" -ne 1 ];then
    echo "Needs one argument"
    exit 1
fi

pollLog=$1

if [ ! -e "$pollLog" ];then 
    echo "There is no inputFile in this directory"
    exit 1
fi

if [ ! -r "$pollLog" ];then   
    echo "User has no read rights"
    exit 1
fi

declare -A Results

while read -r name surname vote
do

#inputFile given from piazza had a newline character on a seperate line at then end of the file
if [[ -z $name ]];then
    #echo "inputFile reached its end"
    break
fi

fullname="${name}_${surname}"

if [[ -v Results["$fullname"] ]]; then
    continue;
fi

Results["$fullname"]="$vote"
#echo "$name $surname voted for ${Results["$fullname"]}"


done < "$pollLog"

declare -a votes
votes=("${Results[@]}")
#we will sort the votes by name
#in order for sort to work we need to replace space with newline character to be the delimeter, so we use tr
echo "${votes[@]}" | tr ' ' '\n' | sort | \
#uniq -c counts the number of times each party has appeared and outputs the number and the party name
        uniq -c | \
#we want the first output to be the name of the party and then comes the number, so we use awk for that
        awk '{print $2,$1}' > "pollerResultsFile"
