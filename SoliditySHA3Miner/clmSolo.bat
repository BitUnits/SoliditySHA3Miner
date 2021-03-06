@echo off
pushd %~dp0

for %%X in (dotnet.exe) do (set FOUND=%%~$PATH:X)
if defined FOUND (goto dotNetFound) else (goto dotNetNotFound)

:dotNetNotFound
echo .NET Core is not found or not installed,
echo download and install from https://www.microsoft.com/net/download/windows/run
goto end

:dotNetFound
DEL /F /Q SoliditySHA3Miner.conf
dotnet SoliditySHA3Miner.dll web3api=https://mainnet.infura.io/ANueYSYQTstCr2mFJjPE abiFile=CLM.abi contract=0xA38FcEdd23dE2191Dc27f9a0240ac170BE0A14fE gasToMine=5 gasLimit=1704624 gasApiURL=https://ethgasstation.info/json/ethgasAPI.json gasApiPath=$.safeLow gasApiMultiplier=0.1 gasApiOffset=0.5 privateKey=YOUR_ETH_PRIVATE_KEY
pause