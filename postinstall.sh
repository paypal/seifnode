#!/bin/sh
echo "Adding rpath after build is complete"
temp=`pwd`
unamestr=`node -e "console.log(process.platform);"`
if [[ "$unamestr" == "darwin" ]]; then
    install_name_tool -add_rpath $temp/deps/seifrng/lib $temp/build/Release/seifnode.node
fi

if [[ "$unamestr" == "linux" ]]; then

    patchelfCmd="patchelf"

    if ! type patchelf > /dev/null; then
        echo "patchelf not found; attempting to install locally."
        cd deps/3rdParty
        wget http://nixos.org/releases/patchelf/patchelf-0.9/patchelf-0.9.tar.bz2
        tar xf patchelf-0.9.tar.bz2
        rm patchelf-0.9.tar.bz2
        cd patchelf-0.9/
        ./configure
        make
        patchelfCmd=./src/patchelf
    fi


    eval $patchelfCmd --set-rpath $temp/deps/seifrng/lib $temp/build/Release/seifnode.node
    cd $temp
fi
