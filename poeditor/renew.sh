#!/bin/bash

./get_all_languages.sh || exit 1
ruby from_poeditor.rb || exit 1
cp translations.json_new ../build/translations.json || exit 1
scp ../build/translations.json 'infection@teeworlds-ams:/home/infection/teeworlds-infection-ddnet' || exit 1
rm translations.json_new || exit 1
echo "Updated successfully"

