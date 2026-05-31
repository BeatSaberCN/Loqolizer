from pathlib import Path
import sys
import re
import json
import urllib.request

qpm_json_path = Path(__file__).parent.parent / 'qpm.json'
mod_template_path = Path(__file__).parent.parent / 'mod.template.json'

with qpm_json_path.open("r",encoding="utf-8") as f:
    qpm_json = json.load(f)
version = qpm_json["info"]["version"]
ver_re = re.compile(r"^(\d+)\.(\d+)\.(\d+)$")
ver_match = ver_re.match(version)
assert ver_match, f"Invalid version number {version} in qpm.json"
next_version = f"{ver_match[1]}.{ver_match[2]}.{int(ver_match[3])+1}"
print(f"Bump release version {version} -> {next_version}")

has_new_data = True
remote_json = json.loads(urllib.request.urlopen("https://frto027.github.io/ssl10n.csv/manifest.json?time={time.time()}").read().decode("utf8"))
# TODO: has_new_data

if has_new_data:
    print("New data detected, will update qpm.json to prepare the next auto release")
    qpm_json["info"]["version"] = next_version
    with qpm_json_path.open("w", encoding="utf-8") as f:
        json.dump(qpm_json, f)

    print("will update mod.template.json")
    with mod_template_path.open("r", encoding='utf-8')as f:
        mod_template = json.load(f)
    mod_template["version"] = next_version
    with mod_template_path.open("w", encoding='utf-8') as f:
        json.dump(mod_template, f)
    
    print("qpm.shared.json will not update, please run 'qpm restore'")

import os

if "GITHUB_OUTPUT" in os.environ:
    print("github environ detected, will write results")
    with open(os.environ["GITHUB_OUTPUT"],'w') as f:
        f.write(f"""cur_ver={version}
next_ver={next_version}
has_new_data={'true' if has_new_data else 'false'}
""")