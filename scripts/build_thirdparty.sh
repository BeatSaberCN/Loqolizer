cd third_party/semver_rustlibrary
cargo build --release --target aarch64-linux-android
cp target/aarch64-linux-android/release/libsemver_rustc.a ../../build