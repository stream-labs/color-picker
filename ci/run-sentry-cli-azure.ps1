$url = "https://github.com/getsentry/sentry-cli/releases/download/1.57.0/sentry-cli-Windows-x86_64.exe"
$output = "$env:ROOTDIRECTORY\sentry-cli.exe"
$wc = New-Object System.Net.WebClient
$wc.DownloadFile($url, $output)

$url = "https://github.com/google/breakpad/blob/master/src/tools/windows/binaries/dump_syms.exe"
$output = "$env:ROOTDIRECTORY\dump_syms.exe"
$wc = New-Object System.Net.WebClient
$wc.DownloadFile($url, $output)

dir $env:ROOTDIRECTORY

dir $env:PDBPATH

.\dump_syms.exe $env:PDBPATH\color_picker.pdb  > $env:PDBPATH\color_picker.sym

.\sentry-cli.exe upload-dif --log-level DEBUG --org streamlabs-obs --project obs-client $env:PDBPATH\color_picker.sym
.\sentry-cli.exe upload-dif --log-level DEBUG --org streamlabs-obs --project obs-client -t $env:PDBPATH\color_picker.pdb

dir $env:PDBPATH

