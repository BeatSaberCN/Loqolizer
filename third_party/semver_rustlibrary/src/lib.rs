//! C-compatible wrappers around `semver` for a single-threaded caller.
//!
//! Both input arguments are UTF-8 byte buffers, passed as a pointer and length;
//! they do not need a terminating NUL. A non-null pointer must refer to `len`
//! readable bytes for the duration of the call. A null pointer is invalid.

use libc::{c_char, c_int, size_t};
use semver::{Version, VersionReq};
use std::{slice, str};

// Access is restricted by the C API's single-threaded, non-reentrant contract.
static mut MOD_VERSION: Option<Version> = None;

/// Parse and store the current version. Returns 1 on success and 0 on error.
/// An unsuccessful call leaves the previously stored version unchanged.
///
/// # Safety
/// `version` must be non-null and point to `len` readable bytes. All calls to
/// either exported function must occur on one thread and must not overlap.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn semver_set_mod_version(version: *const c_char, len: size_t) -> c_int {
    if version.is_null() || len > isize::MAX as usize {
        return 0;
    }
    // SAFETY: The caller guarantees the buffer is valid for `len` bytes.
    let bytes = unsafe { slice::from_raw_parts(version.cast::<u8>(), len) };
    let Ok(text) = str::from_utf8(bytes) else {
        return 0;
    };
    let Ok(parsed) = Version::parse(text) else {
        return 0;
    };
    // SAFETY: The caller guarantees exclusive, non-reentrant access.
    unsafe { std::ptr::replace(&raw mut MOD_VERSION, Some(parsed)) };
    1
}

/// Test the stored version against a semver requirement.
/// Returns 1 for a match, 0 for a mismatch, and -1 for invalid input or if no
/// version has been stored yet.
///
/// # Safety
/// `version_range` must be non-null and point to `len` readable bytes. All
/// calls to either exported function must occur on one thread and not overlap.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn semver_test_version_match(
    version_range: *const c_char,
    len: size_t,
) -> c_int {
    if version_range.is_null() || len > isize::MAX as usize {
        return -1;
    }
    // SAFETY: The caller guarantees the buffer is valid for `len` bytes.
    let bytes = unsafe { slice::from_raw_parts(version_range.cast::<u8>(), len) };
    let Ok(text) = str::from_utf8(bytes) else {
        return -1;
    };
    let Ok(requirement) = VersionReq::parse(text) else {
        return -1;
    };
    // SAFETY: The caller guarantees no overlapping access to MOD_VERSION.
    match unsafe { &*(&raw const MOD_VERSION) }.as_ref() {
        Some(version) => c_int::from(requirement.matches(version)),
        None => -1,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn set(input: &[u8]) -> c_int {
        unsafe { semver_set_mod_version(input.as_ptr().cast(), input.len()) }
    }

    fn matches(input: &[u8]) -> c_int {
        unsafe { semver_test_version_match(input.as_ptr().cast(), input.len()) }
    }

    #[test]
    fn c_api_handles_versions_and_errors() {
        assert_eq!(matches(b"^1.0"), -1);
        assert_eq!(set(b"1.2.3"), 1);
        assert_eq!(matches(b"^1.0"), 1);
        assert_eq!(matches(b">=2.0"), 0);
        assert_eq!(matches(b"not a requirement"), -1);
        assert_eq!(set(b"invalid"), 0);
        assert_eq!(set(b"\xff"), 0);
        assert_eq!(matches(b"^1.0"), 1);
        assert_eq!(unsafe { semver_set_mod_version(std::ptr::null(), 0) }, 0);
        assert_eq!(
            unsafe { semver_test_version_match(std::ptr::null(), 0) },
            -1
        );
    }
}
