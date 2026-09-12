<#
.SYNOPSIS
Prepares the exact SD-card backup contract used by ChinesePoint X4 Pro recovery.

.DESCRIPTION
Copies one operator-selected X4 Pro application image to
<SD root>\backup\crosspoint-x4pro.bin and writes its SHA-256 to the adjacent
crosspoint-x4pro.bin.sha256 file. The firmware still validates the ESP image,
chip family and X4 Pro board tag before it writes an OTA slot.
#>
[CmdletBinding(SupportsShouldProcess)]
param(
  [Parameter(Mandatory)]
  [ValidateScript({ Test-Path -LiteralPath $_ -PathType Leaf })]
  [string]$SourceFirmware,

  [Parameter(Mandatory)]
  [ValidateScript({ Test-Path -LiteralPath $_ -PathType Container })]
  [string]$SdRoot,

  [switch]$Force
)

$source = (Resolve-Path -LiteralPath $SourceFirmware).Path
$root = (Resolve-Path -LiteralPath $SdRoot).Path
$backup = Join-Path -Path $root -ChildPath 'backup'
$target = Join-Path -Path $backup -ChildPath 'crosspoint-x4pro.bin'
$digest = "$target.sha256"

# This is deliberately only a desktop preflight: the firmware repeats a full
# streamed checksum, SHA trailer, chip and board validation before it changes
# an OTA target. Reject the most dangerous operator mistakes here as well,
# before a full USB image or another board's application reaches the SD card.
$minimumApplicationBytes = 64KB
$maximumApplicationBytes = 0x640000
$espImageMagic = [byte]0xE9
$esp32S3ChipId = [byte]9
$x4ProBoardTag = [System.Text.Encoding]::ASCII.GetBytes('CROSSPOINT-BOARD-V1:x4pro;')

function Test-X4ProApplicationPreflight {
  param([Parameter(Mandatory)][string]$Path)

  $payload = [System.IO.File]::ReadAllBytes($Path)
  if ($payload.Length -lt $minimumApplicationBytes) {
    throw "Recovery source is too small to be an X4 Pro application: $($payload.Length) bytes."
  }
  if ($payload.Length -gt $maximumApplicationBytes) {
    throw "Recovery source is too large for the X4 Pro OTA application partition: $($payload.Length) bytes."
  }
  if ($payload[0] -ne $espImageMagic) {
    throw 'Recovery source is not an ESP application image (missing 0xE9 magic).'
  }
  if ($payload.Length -le 12 -or $payload[12] -ne $esp32S3ChipId) {
    throw 'Recovery source is not tagged for ESP32-S3.'
  }

  $hasBoardTag = $false
  for ($offset = 0; $offset -le $payload.Length - $x4ProBoardTag.Length; $offset++) {
    $matches = $true
    for ($index = 0; $index -lt $x4ProBoardTag.Length; $index++) {
      if ($payload[$offset + $index] -ne $x4ProBoardTag[$index]) {
        $matches = $false
        break
      }
    }
    if ($matches) {
      $hasBoardTag = $true
      break
    }
  }
  if (-not $hasBoardTag) {
    throw 'Recovery source has no X4 Pro board tag; do not stage an untagged, other-board, or full USB image.'
  }

  return $payload.Length
}

$preflightBytes = Test-X4ProApplicationPreflight -Path $source
Write-Host "Recovery backup preflight accepted X4 Pro application: $preflightBytes bytes"

if ((Test-Path -LiteralPath $target) -and -not $Force) {
  throw "Refusing to overwrite existing recovery backup: $target. Inspect it first, then rerun with -Force."
}

if ($PSCmdlet.ShouldProcess($backup, 'Create verified X4 Pro recovery backup')) {
  New-Item -ItemType Directory -Path $backup -Force | Out-Null
  Copy-Item -LiteralPath $source -Destination $target -Force:$Force
  $hash = (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant()
  Set-Content -LiteralPath $digest -Value $hash -NoNewline -Encoding ascii
  Get-Item -LiteralPath $target, $digest | Select-Object FullName, Length, LastWriteTime
  Write-Host "Recovery backup digest: $hash"
}
