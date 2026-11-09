# Cloud CI scope and validation boundary

The current `ci.yml` workflow is configured for GitHub-hosted Ubuntu runners. It installs clang-format and PlatformIO tooling, runs `pio check`, builds the `default`, `sticky`, `x4pro`, and `papermono` environments, and runs host unit tests with CMake, Ninja, and CTest. The firmware steps build artifacts; they do not invoke PlatformIO upload or flash targets. The workflow checks submodules recursively and downloads PlatformIO platform/toolchain dependencies, so a remote run needs network access.

`ci.yml` runs for pull requests and pushes to `master`. `release.yml` is tag-triggered. The font release and release-candidate workflows require manual dispatch. No firmware build or test was executed for this documentation change. A passing cloud build or host test does not establish physical X4 Pro panel compatibility, recovery behavior, or flash safety; those remain hardware validation tasks.
