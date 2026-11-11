#pragma once

#include <cstddef>
extern "C"{
    int semver_set_mod_version(const char *version, size_t len);
    int semver_test_version_match(const char *version_range, size_t len);
}