#!/bin/bash

./get_all_languages.sh || exit 1
ruby from_poeditor.rb || exit 1
cp translations.json_new ../build/translations.json && echo OK
cp translations.json 'infection@teeworlds-ams:/home/infection/teeworlds-infection-ddnet'  

