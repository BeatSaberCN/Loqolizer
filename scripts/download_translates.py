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

output = """#include "EmbbedData.hpp"

"""

with open(sys.argv[1], "w") as f:
    f.write(output)

