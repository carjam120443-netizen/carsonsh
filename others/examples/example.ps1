<#
CarsonSH example: run this file from CarsonSH with:
  powershell.exe -NoProfile -File others/examples/example.ps1

CarsonSH currently launches external programs; it does not yet interpret
.ps1/.sh files as native CarsonSH scripts. This file is a PowerShell script.
#>

Write-Output ""
Write-Output "=== Welcome to CarsonSH ==="
Write-Output "This is a small PowerShell command-line tour."
Write-Output ""

Write-Output "1. Show the current directory:"
Get-Location

Write-Output ""
Write-Output "2. Print a message:"
Write-Output "Hello from a script!"

Write-Output ""
Write-Output "3. Show the current date:"
Get-Date

Write-Output ""
Write-Output "4. Show basic system information:"
$computer = Get-CimInstance Win32_OperatingSystem -ErrorAction SilentlyContinue
if ($computer) {
    Write-Output "OS: $($computer.Caption)"
    Write-Output "Version: $($computer.Version)"
} else {
    Write-Output "OS details are unavailable from this PowerShell session."
}

Write-Output ""
Write-Output "5. List files in the current directory:"
Get-ChildItem | Select-Object -First 10 Name, Mode

Write-Output ""
Write-Output "6. CarsonSH built-ins to try interactively:"
@(
    "help"
    "history"
    "cpufetch"
    "gpufetch"
    "displayfetch"
) | ForEach-Object { Write-Output "  $_" }

Write-Output ""
Write-Output "Done! Try those built-ins directly at the CarsonSH prompt."
