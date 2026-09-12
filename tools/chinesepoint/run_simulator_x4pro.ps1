[CmdletBinding()]
param(
  [ValidateSet('ssd1677', 'uc8179', 'uc8279')]
  [string]$Panel = 'ssd1677',
  [switch]$NoUpdate
)

$ErrorActionPreference = 'Stop'

$distro = 'ChinesePoint-Emulator'
$simulatorRepo = '/opt/chinesepoint-src'
$platformio = '/opt/chinesepoint-pio/bin/pio'
$platformioCore = '/opt/chinesepoint-platformio'
$environment = "chinesepoint_simulator_x4pro_$Panel"
$update = if ($NoUpdate) { '0' } else { '1' }

# The Linux working tree and PlatformIO cache live inside a WSL VHDX placed on
# D:. Building directly through /mnt/d is much slower because SCons performs
# thousands of metadata operations over the Windows file bridge.
$command = @'
set -euo pipefail

repo="__REPO__"
pio="__PIO__"
core="__CORE__"
environment="__ENVIRONMENT__"

test -x "$pio" || {
  echo "PlatformIO was not configured for ChinesePoint-Emulator. Run the ChinesePoint emulator setup first." >&2
  exit 1
}

if [ ! -d "$repo/.git" ]; then
  git clone --depth 1 --branch feature/x4pro-continuacao --recurse-submodules \
    https://github.com/pngjaico/ChinesePoint.git "$repo"
elif [ "__UPDATE__" = "1" ]; then
  git -C "$repo" fetch --depth 1 origin feature/x4pro-continuacao
  git -C "$repo" checkout --detach origin/feature/x4pro-continuacao
  git -C "$repo" submodule update --init --recursive
fi

cd "$repo"
PLATFORMIO_CORE_DIR="$core" "$pio" run -e "$environment"
exec ".pio/build/$environment/program"
'@

$command = $command.Replace('__REPO__', $simulatorRepo).
  Replace('__PIO__', $platformio).
  Replace('__CORE__', $platformioCore).
  Replace('__ENVIRONMENT__', $environment).
  Replace('__UPDATE__', $update)
$command = $command -replace "`r", ''

# WSL's Windows command-line bridge strips nested shell quotes when a multiline
# command is passed directly. Base64 keeps the already-validated Bash payload
# intact (including its variables and quoted paths).
$payload = [Convert]::ToBase64String([Text.Encoding]::UTF8.GetBytes($command))
$invoke = "echo '$payload' | base64 -d | bash"
& wsl.exe -d $distro -u root -- bash -lc $invoke
if ($LASTEXITCODE -ne 0) {
  throw "The X4 Pro $Panel simulator exited with code $LASTEXITCODE."
}
