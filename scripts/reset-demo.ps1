# Puts the demo in its starting state: both drivers warmed up,
# demo orders removed, legacy line active in orderdesk.cfg.
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$cfgPath = Join-Path $root 'orderdesk.cfg'

function Use-Driver([ValidateSet('legacy', 'new')][string]$Which) {
    $lines = foreach ($line in Get-Content $cfgPath) {
        if ($line -match '^;?(SQL_CONNECT=(.*))$') {
            $isLegacy = $Matches[2] -like 'VTX12_SQLNATIVE:*'
            if ($isLegacy -eq ($Which -eq 'legacy')) { $Matches[1] } else { ";$($Matches[1])" }
        } else {
            $line
        }
    }
    Set-Content -Path $cfgPath -Value $lines
}

Push-Location $root
try {
    foreach ($which in 'new', 'legacy') {
        Use-Driver $which
        dbs bin\bench
        if ($LASTEXITCODE) { throw "Warm-up run failed on the $which driver" }
    }

    $cfg = Get-Content $cfgPath -Raw
    if (-not ($cfg -match 'SQL_CONNECT=(?!VTX12_SQLNATIVE)\w+:([^/]+)/([^/]+)/([^/\s]+)/(\d+)')) {
        throw 'orderdesk.cfg has no new-driver line'
    }
    $env:SQLCMDPASSWORD = $Matches[2]
    sqlcmd -S "$($Matches[3]),$($Matches[4])" -U $Matches[1] -d OrderDesk -C -b -i sql\reset.sql
    if ($LASTEXITCODE) { throw 'reset.sql failed' }

    Write-Host 'Ready: legacy line active, demo orders removed.'
}
finally {
    Remove-Item Env:SQLCMDPASSWORD -ErrorAction SilentlyContinue
    Pop-Location
}
