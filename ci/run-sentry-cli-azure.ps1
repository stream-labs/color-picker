Get-ChildItem -Path $env:PDBPATH\color-picker -Filter *.pdb -File | ForEach-Object {.\dump_syms.exe $_.FullName > $env:ROOTDIRECTORY\syms\$_} 
Get-ChildItem -Path $env:PDBPATH\color-picker -Filter *.pdb -Recurse -File | ForEach-Object {.\dump_syms.exe $_.FullName > $env:ROOTDIRECTORY\syms\$_}
Get-ChildItem -Path $env:ROOTDIRECTORY\syms\*.pdb | Rename-Item -NewName { $_.Name -Replace ".pdb",".sym"}
Get-ChildItem -Path $env:ROOTDIRECTORY\syms -Filter *.sym -File | ForEach-Object {Get-Content $_.FullName | Out-File -Encoding Ascii "$env:ROOTDIRECTORY\syms\ascii\$($_.BaseName).sym" } 
