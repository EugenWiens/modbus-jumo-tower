"""
PlatformIO pre-build script: inject git-tag-based version numbers.

Runs 'git describe --tags --always', parses the result into major/minor/patch
components, and exposes them as integer macros:
  FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_PATCH

Falls back to 0.0.0 when git is unavailable or no tags exist.
"""
import subprocess

Import("env")  # noqa: F821 – injected by PlatformIO SCons environment


def _git_version() -> str:
    try:
        raw = subprocess.check_output(
            ["git", "describe", "--tags", "--always"],
            stderr=subprocess.DEVNULL,
        ).decode().strip()
        if not raw:
            return "0.0.0"
        # Strip optional leading 'v' (e.g. "v1.2.3" → "1.2.3")
        raw = raw.lstrip("v")
        # If the result contains no dot it is a bare commit hash, not a tag.
        if "." not in raw:
            return "0.0.0"
        return raw
    except Exception:
        return "0.0.0"


version = _git_version()
print(f"*** Firmware version: {version}", flush=True)

# Parse into numeric components so each can be stored in its own Modbus register.
# Accepts formats like "1.2.3", "01.02.03", "v1.2.3-4-gabcdef" (git describe).
def _split(v: str):
    parts = v.split(".")
    def _int(s: str) -> int:
        try:
            return int("".join(c for c in s if c.isdigit()) or "0")
        except ValueError:
            return 0
    major = _int(parts[0]) if len(parts) > 0 else 0
    minor = _int(parts[1]) if len(parts) > 1 else 0
    patch = _int(parts[2]) if len(parts) > 2 else 0
    return major, minor, patch

major, minor, patch = _split(version)
print(f"    → major={major} minor={minor} patch={patch}", flush=True)

env.Append(CPPDEFINES=[  # noqa: F821
    ("FIRMWARE_VERSION_MAJOR", str(major)),
    ("FIRMWARE_VERSION_MINOR", str(minor)),
    ("FIRMWARE_VERSION_PATCH", str(patch)),
])
