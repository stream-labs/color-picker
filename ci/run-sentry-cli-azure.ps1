$url = "https://github.com/getsentry/sentry-cli/releases/download/1.57.0/sentry-cli-Windows-x86_64.exe"
$output = "$env:ROOTDIRECTORY\sentry-cli.exe"
$wc = New-Object System.Net.WebClient
$wc.DownloadFile($url, $output)

$url = "https://github.com/google/breakpad/blob/master/src/tools/windows/binaries/dump_syms.exe"
$output = "$env:ROOTDIRECTORY\dump_syms.exe"
$wc = New-Object System.Net.WebClient
$wc.DownloadFile($url, $output)


Get-ChildItem -Path $env:PDBPATH\color-picker -Filter *.pdb -File | ForEach-Object {.\dump_syms.exe $_.FullName > $env:ROOTDIRECTORY\syms\$_} 
Get-ChildItem -Path $env:PDBPATH\color-picker -Filter *.pdb -Recurse -File | ForEach-Object {.\dump_syms.exe $_.FullName > $env:ROOTDIRECTORY\syms\$_}
Get-ChildItem -Path $env:ROOTDIRECTORY\syms\*.pdb | Rename-Item -NewName { $_.Name -Replace ".pdb",".sym"}
Get-ChildItem -Path $env:ROOTDIRECTORY\syms -Filter *.sym -File | ForEach-Object {Get-Content $_.FullName | Out-File -Encoding Ascii "$env:ROOTDIRECTORY\syms\ascii\$($_.BaseName).sym" } 
