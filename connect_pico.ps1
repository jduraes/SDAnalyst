# PowerShell script to connect to Raspberry Pi Pico SD Card Reader
# Run this from Windows PowerShell

$port = "COM6"
$baudRate = 115200

Write-Host "Connecting to Raspberry Pi Pico SD Card Reader..."
Write-Host "Port: $port"
Write-Host "Baud Rate: $baudRate"
Write-Host "Press Ctrl+C to exit"
Write-Host "----------------------------------------"

try {
    # Configure the port
    mode $port BAUD=$baudRate PARITY=n DATA=8 STOP=1
    
    # Read from the port
    Get-Content $port -Wait
}
catch {
    Write-Host "Error connecting to $port"
    Write-Host "Make sure:"
    Write-Host "1. The Pico is connected and running the SD card reader firmware"
    Write-Host "2. COM6 is the correct port (check Device Manager)"
    Write-Host "3. No other application is using the port"
    Write-Host ""
    Write-Host "You can also use PuTTY with these settings:"
    Write-Host "- Connection type: Serial"
    Write-Host "- Serial line: $port"
    Write-Host "- Speed: $baudRate"
}
