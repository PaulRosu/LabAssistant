<#
.SYNOPSIS
    MQ Proxy settings for PowerShell
.DESCRIPTION
    MQ Proxy settings for PowerShell
#>

Param(
    [switch]$askForCreds ## ask for username and password (encrypted)
)

$ErrorActionPreference = "Stop"

# On MQ machines proxy is configured via PAC (Proxy Auto Configuration) in the registry
$RegistryInternetSettings = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Internet Settings"

if ($null -eq (Get-ItemProperty -Path $RegistryInternetSettings).ProxyEnable) {
    # If AutoConfigURL value doesn't exist, set it to the specified URL
    Set-ItemProperty -Path $RegistryInternetSettings -Name AutoConfigURL -Value "http://os-proxypac-dl.marquardt.de:8081/proxy-OS.pac"
    Write-Output "AutoConfigURL value has been set."
}

if (0 -eq (Get-ItemProperty -Path $RegistryInternetSettings).ProxyEnable) {
    # We want to use the system's default proxy in PowerShell, too. So enable it.
    Set-ItemProperty -Path $RegistryInternetSettings -Name ProxyEnable -Value 1
    Write-Output "ProxyEnable value has been set."
}

# Still some environment variables are needed for tools like git or python
$Env:NO_PROXY = "localhost,.marquardt.de,.marquardt.com"

# Jenkins always needs special treatment:
# obviously the default proxy does not work for our service user MQ_S_ci_jenkservice
if ($Env:JENKINS_URL) {
    $ProxyHost = 'osde01proxy02.marquardt.de:8080'
    [net.webrequest]::DefaultWebProxy = New-Object System.Net.WebProxy("http://$ProxyHost", $true, ($Env:NO_PROXY).split(','))
}
else {
    $ProxyHost = 'os-proxypac-dl.marquardt.de:8080'
}

# Some default credentials are always available when logged in
[net.webrequest]::DefaultWebProxy.Credentials = [System.Net.CredentialCache]::DefaultNetworkCredentials

# Remaining proxy env vars for git & Co.
if ($askForCreds) {
    [string]$username = Read-Host "Please enter your Windows user name for proxy authentication"
    $userpwd_sec = Read-Host "Please enter your Windows password for proxy authentication" -AsSecureString
    $bstr = [System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($userpwd_sec)
    [string]$encodedPwd = [System.Uri]::EscapeDataString([System.Runtime.InteropServices.Marshal]::PtrToStringAuto($bstr))
    $Env:HTTP_PROXY = "http://" + $username + ':' + $encodedPwd + '@' + $ProxyHost
}
else {
    $Env:HTTP_PROXY = "http://$ProxyHost"
}
$Env:HTTPS_PROXY = $Env:HTTP_PROXY