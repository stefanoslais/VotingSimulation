#!/bin/bash


#check if the number of arguments given is the correct one
if [ "$#" -ne 2 ]; then
	echo "Needs 2 arguments for the script to run"
	exit 1
fi

#check correctness of first argument
if [ ! -f "$1" ] || [ "$1" != "political_parties.txt" ]; then
	echo "First argument needs to be an existing file called 'political_parties.txt'"
	exit 1
fi

#check correctness of second argument
if [ $2 -le 0 ]; then
	echo "Second argument must be a positive integer"
	exit 1
fi

polParties=$1
let numLines=$2
touch inputFile
for (( numLine=0; numLine < numLines; numLine++ )); do

#first we choose a random length between 3 and 12
	randomLengthName=$(( RANDOM % 10 + 3 ))
	randomLengthSurname=$(( RANDOM % 10 + 3 ))
	
#we will use /dev/urandom to produce a continuous flow of random bytes
#we will process those bytes as characters, thus the need for tr command
#we are deleting the characters that are non alphabetical and also whitespaces
#to limit the number of characters to the length we want,and also for the /urandom/ to not be running eternally,
#we feed the flow to the head command providing the length
	
	Name=$(tr -dc '[:alpha:]' < /dev/urandom | tr -d '[:space:]' | head -c $randomLengthName)
#same with Surname
	Surname=$(tr -dc '[:alpha:]' < /dev/urandom | tr -d '[:space:]' | head -c $randomLengthName)

#time to choose a political party
#we know that in the politicalParties.txt file, the number of parties is equal to the number of lines of the file
#wc -l outputs both the number of lines and the name of the file too
#so we need keep only the first column of the output
	NumParties=$( wc -l $polParties  | awk '{print $1}')

#pick a random line in the range of the first line till the $NumParties line
	PartyLine=$(( RANDOM % NumParties + 1 ))

#fetch that line without outputing it to the console
	PartyName=$( sed -n "${PartyLine}p" "$polParties") 

#insert it to the inputFile
	echo $Name $Surname "$PartyName" >> inputFile

done
