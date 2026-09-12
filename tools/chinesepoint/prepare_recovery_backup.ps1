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
