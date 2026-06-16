import time

MANIFEST_URL = f"https://frto027.github.io/ssl10n.csv/manifest.json?time={time.time()}"

import urllib.request
import http.client
import json
import sys
import datetime
import pathlib
import gzip
from io import StringIO
assert len(sys.argv) == 2, "usage: scripts/download_translates.py <generate cpp file path>"

manifest = json.loads(urllib.request.urlopen(MANIFEST_URL).read().decode("utf8"))["mods"]

embbed_data_report_json = {
    "mods":{}
}

output_first = """#include "EmbbedData.hpp"

"""

next_source_index = 0

output = """
std::map<std::string, LangCtrl::DataModInfo> LangCtrl::embbedData = {
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
    
    output += "   {\"" + escape(modid) + "\",{"
    output += f"""
    .modId = "{escape(modid)}",
    .name = "{escape(name)}",
    .datas = {'{'}"""
    

    for data in body["datas"]:
        version = data["version"]
        csv_bytes = urllib.request.urlopen(data["csv_url"]).read()
        csv_md5 = data["md5"]
        bytes_str = ','.join([str(x) for x in csv_bytes])

        next_source_index += 1

        output_first += f"static const char res_{next_source_index}[{len(csv_bytes)}] = " + "{" + bytes_str + "};\n"
        output += f"""
        {'{'}
            .csv_buffer = res_{next_source_index},
            .csv_buffer_size = {len(csv_bytes)},
            .md5 = "{escape(csv_md5)}",
            .version = "{escape(version)}"
        {'}'},"""
    output += """
    }
"""
    output += """
    }},
"""
    embbed_data_report_json["mods"][modid] = body
    mod_build_count += 1



output += "};\n\n"

# polyglot csv file from sira localizer

distribution_key = "b8d0ace786d64ba14775878o9lk"
def read_sira(path:str)->bytes:
    url:http.client.HTTPResponse = urllib.request.urlopen(f"https://distributions.crowdin.net/{distribution_key}/{path}")
    if 'Content-Encoding' in url.headers:
        assert url.headers['Content-Encoding'] == 'gzip', 'we only support gzip content encoding from crowdin project'
        return gzip.decompress(url.read())
    return url.read()
siraLocalizerTexts = read_sira("content/beat-saber.csv")
output += f"static const char res_siraLocalizer[{len(siraLocalizerTexts)}] = " + "{" + ','.join([str(x) for x in siraLocalizerTexts]) + "};\n"
import hashlib
siraLocalizerTextsMd5 = hashlib.md5()
siraLocalizerTextsMd5.update(siraLocalizerTexts)
siraLocalizerTextsMd5Text = siraLocalizerTextsMd5.hexdigest()
output += """
std::string_view LangCtrl::siraLocalizerResource(){
    return std::string_view(res_siraLocalizer, sizeof(res_siraLocalizer));
}
"""

import csv
sira_csv = read_sira(f"content/plugins/sira-localizer.csv")
csv_rows = [x for x in csv.reader(StringIO(sira_csv.decode('utf-8-sig')))]
started = False
for row in csv_rows:
    if not started:
        if len(row) > 0 and row[0] == "Polyglot":
            started = True
        continue
    if len(row) > 0 and len(row[0]) > 0:
        row[0] = "SIRALOCALIZER_" + row[0]
    if len(row) > 0 and row[-1].endswith("\r"):
        row[-1] = row[-1][:-1]
csv_file = pathlib.Path(__file__).parent.parent / 'build' / 'downloaded_sira_localize.csv'
with csv_file.open('w', encoding='utf-8-sig') as f:
    w = csv.writer(f,lineterminator='\n')
    w.writerows(csv_rows)
with csv_file.open('rb') as f:
    bts = f.read()
    output += f"static const char res_siraLocalizer_mod_res[{len(bts)}] = " + "{" + ','.join([str(x) for x in bts]) + "};\n"
output += """
std::string_view LangCtrl::siraLocalizerModResource(){
    return std::string_view(res_siraLocalizer_mod_res, sizeof(res_siraLocalizer_mod_res));
}
"""



timestamp = datetime.datetime.now()

output += "const char * LangCtrl::embbedDataTimestamp = " + f'"{timestamp}";\n'

embbed_data_report_json["timestamp"] = str(timestamp)
embbed_data_report_json["mod_count"] = mod_build_count
embbed_data_report_json["siraLocalizerMd5"] = siraLocalizerTextsMd5Text
with open(sys.argv[1], "w") as f:
    f.write(output_first)
    f.write(output)

with (pathlib.Path(__file__).parent.parent / "EmbbedDataReport.json").open("w", encoding='utf-8') as f:
    f.write(json.dumps(embbed_data_report_json, ensure_ascii=False))
