require 'json'

langfiles = {
        "en" => "english",
        "ru" => "russian",
        "hu" => "hungarian",
        "fr" => "french",
        "de" => "german",
        "pl" => "polish",
        "es" => "spanish",
        "uk" => "ukrainian",
        "tr" => "turkish"
}

tr0 = JSON.parse(File.read("orig/translations.json"))["translated strings"]

tr = []

langfiles.each do |short, long|
  poe = JSON.parse(File.read("from_poeditor/" + short + ".json"))
  poe.each do |h|
    tr << {
      "context" => long,
      "or" => h["term"],
      "tr" => (h["definition"] || "")
    }
  end  
end

tr += tr0

json = {
  "translated strings" => tr
}

File.write("translations.json_new", JSON.pretty_generate(json))
