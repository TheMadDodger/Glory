if [ -d "C:\Program Files\Mono" ]; then
    echo "Mono is already installed."
else
    echo "Mono is NOT installed. Installing automatically."
    curl "https://download.mono-project.com/archive/6.12.0/windows-installer/mono-6.12.0.182-x64-0.msi" -o mono.msi
    powershell -ExecutionPolicy Bypass -File ./install-mono.ps1
fi