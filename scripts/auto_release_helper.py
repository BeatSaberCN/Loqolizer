from pathlib import Path
import sys
import re
import json
import urllib.request
import time

qpm_json_path = Path(__file__).parent.parent / 'qpm.json'
mod_template_path = Path(__file__).parent.parent / 'mod.template.json'

with qpm_json_path.open("r",encoding="utf-8") as f:
    qpm_json = json.load(f)
version = qpm_json["info"]["version"]

latest_tag = json.loads(urllib.request.urlopen("https://api.github.com/repos/BeatSaberCN/Loqolizer/releases/latest").read().decode("utf8"))["tag_name"]
ver_re = re.compile(r"^(auto-)?v(\d+)\.(\d+)\.(\d+)$")
ver_match = ver_re.match(latest_tag)
assert ver_match, f"Invalid version number {latest_tag} in remote release"
next_version = f"{ver_match[2]}.{ver_match[3]}.{int(ver_match[4])+1}"
print(f"Remote latest tag is {latest_tag}. Will bump release version {version} -> {next_version} if new data avaliable.")

has_new_data = False
local_json = json.loads(urllib.request.urlopen(f"https://github.com/BeatSaberCN/Loqolizer/releases/latest/download/EmbbedDataReport.json?time={time.time()}").read().decode("utf8"))["mods"]
local_md5s:dict[str,set[str]] = {}
remote_json = json.loads(urllib.request.urlopen(f"https://frto027.github.io/ssl10n.csv/manifest.json?time={time.time()}").read().decode("utf8"))
remote_md5s:dict[str,set[str]] = {}
for mod_id in local_json:
    if "datas" in local_json[mod_id] and len(local_json[mod_id]["datas"]) > 0:
        local_md5s[mod_id] = set([x["md5"] for x in local_json[mod_id]["datas"]])
for mod_id in remote_json:
    if "datas" in remote_json[mod_id] and len(remote_json[mod_id]["datas"]) > 0:
        remote_md5s[mod_id] = set([x["md5"] for x in remote_json[mod_id]["datas"]])
print(f"we have {len(remote_json)} remote mods and {len(local_md5s)} local mods.")
auto_release_change_log = ""
for mod_id in remote_md5s:
    if not mod_id in local_md5s:
        has_new_data = True
        auto_release_change_log += f"{mod_id}: Added\n"
    if mod_id in local_md5s and remote_md5s[mod_id] != local_md5s[mod_id]:
        has_new_data = True
        auto_release_change_log += f"{mod_id}: Updated\n"
for mod_id in local_md5s:
    if not mod_id in remote_md5s:
        has_new_data = True
        auto_release_change_log += f"{mod_id}: Removed\n"

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

    print("Emit AutoReleaseChangelog.txt...")
    with open("AutoReleaseChangelog.txt",'w', encoding='utf-8') as f:
        f.write(auto_release_change_log)
else:
    print("no translate update detected")

import os

if "GITHUB_OUTPUT" in os.environ:
    print("github environ detected, will write results")
    with open(os.environ["GITHUB_OUTPUT"],'w') as f:
        f.write(f"""cur_ver={version}
cur_latest_tag={latest_tag}
next_ver={next_version}
has_new_data={'true' if has_new_data else 'false'}
""")