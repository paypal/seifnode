#!/bin/sh

unamestr=`node -e "console.log(process.platform);"`
npm install nan

if [[ "$unamestr" != "win32" ]]; then

	echo `pwd`
	rngConfigFile="`pwd`/config"
	mkdir deps
	cd deps
	git clone https://github.com/paypal/seifrng.git
	cd seifrng

	if [ -d "./build" ]; then
		rm -rf build
	fi

	rngCmakeCmd="cmake ../"
	if ! type cmake > /dev/null; then
		# attempt to install cmake
		echo "Attempting to install cmake locally....."
		echo "This is slow. It is recommended that cmake be installed globally."

		if [ ! -d "./3rdParty" ]; then
			mkdir 3rdParty
		fi

		cd 3rdParty
		git clone https://gitlab.kitware.com/cmake/cmake.git

		cd cmake
		./bootstrap && make -j4

		rngCmakeCmd="../3rdParty/cmake/bin/cmake ../"
		cd ../../
	fi


	mkdir build
	cd build

	rngBuildCmd="make"
	rngBuildInstallCmd="make install"

	echo "Runnning $rngCmakeCmd"
	eval $rngCmakeCmd

	echo "Runnning $rngBuildInstallCmd"
	eval $rngBuildInstallCmd

	if [[ $? != 0 ]]; then
		exit 1
	fi

	echo "Running $rngBuildCmd"
	echo `$rngBuildCmd`



	if [ -d "../3rdParty/cryptopp" ]; then
		echo "CryptoPP Installed Locally."
	else

		if [ ! -d "../3rdParty" ]; then
			mkdir ../3rdParty
		fi

		echo "Moving CryptoPP."
		cp -r /usr/local/include/cryptopp ../3rdParty
		cp /usr/local/lib/libcryptopp.a ../3rdParty/cryptopp/libcryptopp.a
	fi

	cd ../../../
	node-gyp rebuild
else
	echo "[Error] OS not supported."
	exit 1
fi
