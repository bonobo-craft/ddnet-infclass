import json

langfiles = {
        "ru" : "russian.txt",
        "hu" : "hungarian.txt",
        "fr" : "french.txt",
        "de" : "german.txt",
        "pl" : "polish.txt",
        "es" : "spanish.txt",
        "uk" : "ukrainian.txt",
        "tr" : "turkish.txt"
}

for short in langfiles:
  print(short)
  f1 = open("from_poeditor/" + short + ".json")
  f2 = open("2upload/" + langfiles[short], 'a')
  
  terms = json.loads(f1.read())
  
  f2.write("\n")
  f2.write("everything.bellow.is.added\n")
  f2.write("== EVERYTHINGBELLOWISADDED\n")
  
  for term in terms:
      f2.write("\n")
      f2.write(term["term"] + "\n")
      if term["definition"]:
          f2.write("== " + term["definition"].rstrip().replace("\n", "\\n") + "\n")
      else:
          f2.write("== " + term["term"].rstrip().replace("\n", "\\n") + "\n")
    
  f1.close()
  f2.close()

print("\nDone")
