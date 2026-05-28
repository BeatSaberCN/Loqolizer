MANIFEST_URL = "https://frto027.github.io/ssl10n.csv/manifest.json"

import urllib.request
import json
import sys
import datetime
import pathlib
assert len(sys.argv) == 2, "usage: scripts/download_translates.py <generate cpp file path>"

manifest = json.loads(urllib.request.urlopen(MANIFEST_URL).read().decode("utf8"))

embbed_data_report = "Embbed Data Report:\n"

output_first = """#include "EmbbedData.hpp"

"""

next_source_index = 0

output = """
std::map<std::string, LangCtrl::EmbbedData> LangCtrl::embbedData = {
"""

mod_build_count = 0

def escape(s:str):
    return s.replace("\\","\\\\").replace('"','\\"').replace("\n","\\\n")

for modid in manifest:
    modid:str
    body = manifest[modid]
    if not "name" in body:
        name = modid
    elif type(body["name"]) == dict and 'English' in body["name"]:
        name = body["name"]["English"]
    elif type(body["name"]) == str:
        name = body["name"]
    
    if len(body["datas"]) == 0:
        continue
    data = body["datas"][0]
    version = data["version"]
    csv_bytes = urllib.request.urlopen(data["csv_url"]).read()
    csv_md5 = data["md5"]
    bytes_str = ','.join([str(x) for x in csv_bytes])

    next_source_index += 1

    output_first += f"static const char res_{next_source_index}[{len(csv_bytes)}] = " + "{" + bytes_str + "};\n"

    output += "   {\"" + escape(modid) + "\",{"
    output += f"""
    .modId = "{escape(modid)}",
    .name = "{escape(name)}",
    .csv_buffer = res_{next_source_index},
    .csv_buffer_size = {len(csv_bytes)},
    .md5 = "{escape(csv_md5)}",
    .version = "{escape(version)}"
"""
    output += "   }},\n"

    embbed_data_report += "Mod: " + modid + ", version:" + version + ", md5: " + csv_md5 + "\n"

    mod_build_count += 1



output += "};\n"

timestamp = datetime.datetime.now()

output += "const char * LangCtrl::embbedDataTimestamp = " + f'"{timestamp}";\n'
embbed_data_report += f"{mod_build_count} mods built\n"
embbed_data_report += "Build Timestamp: " + str(timestamp) + "\n"

with open(sys.argv[1], "w") as f:
    f.write(output_first)
    f.write(output)

with (pathlib.Path(__file__).parent.parent / "EmbbedDataReport.txt").open("w") as f:
    f.write(embbed_data_report)
