$file = "mono.msi" 
$log = "install.log" 
$procMain = Start-Process "msiexec" "/i `"$file`" /qn /l*! `"$log`"" -NoNewWindow -PassThru
$procLog = Start-Process "powershell" "Get-Content -Path `"$log`" -Wait" -NoNewWindow -PassThru 
$procMain.WaitForExit() 
$procLog.Kill()