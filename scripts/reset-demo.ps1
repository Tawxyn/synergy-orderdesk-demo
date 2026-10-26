# Clears demo orders and recorded runs using the current direct SQL connection.
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
if ($env:SQL_CONNECT -notmatch '^sqlserver:([^/]+)/([^/]+)/([^/\s]+)/(\d+)$') {
    throw 'Set SQL_CONNECT to sqlserver:user/password/server/port before resetting the demo.'
}
$login = $Matches[1]
$password = $Matches[2]
$server = $Matches[3]
$port = $Matches[4]
$previousPassword = $env:SQLCMDPASSWORD

Push-Location $root
try {
    $env:SQLCMDPASSWORD = $password
    sqlcmd -S "$server,$port" -U $login -d OrderDesk -C -b -i sql\reset.sql
    if ($LASTEXITCODE) { throw 'reset.sql failed' }

    Write-Host 'Demo orders and recorded runs cleared.'
}
finally {
    $env:SQLCMDPASSWORD = $previousPassword
    Pop-Location
}
