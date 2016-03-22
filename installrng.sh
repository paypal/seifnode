#!/bin/sh
echo `pwd`
rngConfigFile="`pwd`/config"
mkdir deps
cd deps
rm -rf seifrng
git clone https://github.com/paypal/seifrng.git
cd seifrng
mkdir build
cd build

rngOptions=""
while IFS='' read -r line || [[ -n "$line" ]]; do
	
	if [[ $line != \#* ]]; then
		rngOptions="$rngOptions -D$line"
	fi
done < "$rngConfigFile"

rngCmakeCmd="cmake $rngOptions ../"

unamestr=`node -e "console.log(process.platform);"`

rngBuildCmd="make -j8"
if [[ "$unamestr" == "win32" ]]; then
	rngBuildCmd='msbuild.exe isaacrng.sln //t:Build /p:Configuration=Release;Platform=x64'
fi

echo "Runnning $rngCmakeCmd"
eval $rngCmakeCmd

if [[ $? != 0 ]]; then
	exit 1
fi



echo "Running $rngBuildCmd"
echo `$rngBuildCmd`

if [[ "$unamestr" != "win32" ]]; then
	make install
	cd ../../../
	node-gyp rebuild
fi


