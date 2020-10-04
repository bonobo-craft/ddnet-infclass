#!/bin/zsh

for lang ("russian.txt" "hungarian.txt" "french.txt" "german.txt" "polish.txt" "spanish.txt" "ukrainian.txt" "turkish.txt")
do
        echo "processing "$lang
        if ! (grep "EVERYTHINGBELLOWISADDED" ../data/languages/$lang 1>/dev/null )
        then
                cat 2upload/$lang >> ../data/languages/$lang
        else
                (echo "Already concatenated"; exit 1)
        fi
done
