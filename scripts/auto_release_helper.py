from pathlib import Path
import sys
import re
import json
import urllib.request

qpm_json_path = Path(__file__).parent.parent / 'qpm.json'#
with qpm_json_path.open("r",encoding="utf-8") as f:
    qpm_json = json.load(f)
version = qpm_json["info"]["version"]
ver_re = re.compile(r"^(\d+)\.(\d+)\.(\d+)$")
ver_match = ver_re.match(version)
assert ver_match, f"Invalid version number {version} in qpm.json"
next_version = f"{ver_match[1]}.{ver_match[2]}.{int(ver_match[3])+1}"
print(f"Bump release version {version} -> {next_version}")

has_new_data = False
remote_json = json.loads(urllib.request.urlopen("https://frto027.github.io/ssl10n.csv/manifest.json?time={time.time()}").read().decode("utf8"))
# TODO: has_new_data

import os

if "GITHUB_OUTPUT" in os.environ:
    with open(os.environ["GITHUB_OUTPUT"],'w') as f:
        f.write(f"""cur_ver={'true' if version else 'false'}
next_ver={'true' if next_version else 'false'}
has_new_data={'true' if has_new_data else 'false'}
""")