#!/bin/sh
echo "Adding rpath after build is complete"
temp=`pwd`
unamestr=`node -e "console.log(process.platform);"`
if [[ "$unamestr" == "darwin" ]]; then
    install_name_tool -add_rpath $temp/deps/seifrng/lib $temp/build/Release/seifnode.node
fi

if [[ "$unamestr" == "linux" ]]; then
    patchelf --set-rpath $temp/deps/seifrng/lib $temp/build/Release/seifnode.node
fi