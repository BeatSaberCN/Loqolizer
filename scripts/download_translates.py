import time

MANIFEST_URL = f"https://frto027.github.io/ssl10n.csv/manifest.json"

import urllib.request
import http.client
import json
import sys
import datetime
import pathlib
import gzip
from io import StringIO
assert len(sys.argv) == 2, "usage: scripts/download_translates.py <generate cpp file path>"

manifest = json.loads(urllib.request.urlopen(MANIFEST_URL + f"?time={time.time()}").read().decode("utf8"))

protobuf = urllib.request.urlopen(manifest['protobufSnapshotBinaryUrl']).read()

protobuf_txt =  ",".join([str(x) for x in protobuf]) 


print(protobuf)
output = """#include "EmbbedData.hpp"

namespace LangCtrl{

const char * getManifestUrl(){
    return "%s";
}

std::string_view embbedDataSnapshot(){
    const static char strs [] = { %s };
    return std::string_view(strs, sizeof(strs));
}
}

""" % (MANIFEST_URL, protobuf_txt)

with open(sys.argv[1], "w") as f:
    f.write(output)

