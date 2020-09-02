#!/bin/zsh

API_TOKEN="f5b28e07427a279fa6bbf8b86d96c438"

for lang ("ru" "hu" "fr" "de" "pl" "es" "uk" "tr")
do
  curl -X POST https://api.poeditor.com/v2/projects/export \
       -d api_token="f5b28e07427a279fa6bbf8b86d96c438" \
       -d id="363119" \
       -d language=$lang \
       -d type="json" \
  | jq . | grep url | cut -d\" -f4 | xargs -I\# wget \# -O "from_poeditor/"$lang".json"
done
