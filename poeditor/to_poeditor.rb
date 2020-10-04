require 'json'

tr = JSON.parse(File.read("translations_.json"))

languages = tr.map {|t| t["context"]}.sort.uniq

languages.each do |language|
  json = tr.select{ |t| t["context"] == language }.map { |t| a = t.dup; a.delete("context"); a }.to_json
  File.write("#{language}.json", json)
end

english_json = tr.select{ |t| t["context"] == languages.first }.map { |t| a = t.dup; a.delete("context"); a["definition"]=a["term"];a }.to_json
File.write("english.json", english_json)
