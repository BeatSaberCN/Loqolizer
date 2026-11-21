import pathlib

with open("third_party/protobuf/CMakeLists.txt", "r") as f:
    txt = f.read()

if not "qpm_defines.cmake" in txt:
    txt = "include(../../qpm_defines.cmake)\n" + txt
    with open("third_party/protobuf/CMakeLists.txt", "w") as f:
        f.write(txt)
    proj_ndk = pathlib.Path("ndkpath.txt")
    if proj_ndk.exists():
        proj_ndk.copy_into("third_party/protobuf/")