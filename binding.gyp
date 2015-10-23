{
    "targets": [
        {
            "target_name": "seifnode",
            "sources": [ 
                "src/addon.cc", 
                "src/eccisaac.cc", 
                "src/aesxor.cc", 
                "src/rng.cc", 
                "src/seifsha3.cc" 
            ],
            "cflags_cc!": [ 
                "-fno-rtti", 
                "-fno-exceptions" 
            ],
            "conditions": [
                [ 'OS=="mac"', {
                    "xcode_settings": {
                        'OTHER_CPLUSPLUSFLAGS' : [
                            '-std=c++11',
                            '-stdlib=libc++', 
                            '-v'
                        ],
                        'OTHER_LDFLAGS': ['-stdlib=libc++'],
                        'MACOSX_DEPLOYMENT_TARGET': '10.10',
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
                    },
                    "include_dirs": [
                        "/usr/local/include/cryptopp",
                        "<!(node -e \"require('nan')\")",
                        "<!(pwd)/deps/seifrng/isaacRandomPool/include",
                        "<!(pwd)/deps/seifrng/isaacrng/include",
                        "<!(pwd)/deps/seifrng/fileCryptopp/include"
                    ],
                    "libraries": [
                        "/usr/local/lib/libcryptopp.a",
                        "<!(pwd)/deps/seifrng/lib/*",
                        "-Wl,-rpath <!(pwd)/deps/seifrng/lib/"
                    ],
                }],
                [ 'OS=="linux"', {
                    "include_dirs": [
                        "/usr/local/include/cryptopp",
                        "<!(node -e \"require('nan')\")",
                        "<!(pwd)/deps/seifrng/isaacRandomPool/include",
                        "<!(pwd)/deps/seifrng/isaacrng/include",
                        "<!(pwd)/deps/seifrng/fileCryptopp/include"
                    ],
                    "libraries": [
                        "/usr/local/lib/libcryptopp.a",
                        "<!(pwd)/deps/seifrng/lib/*",
                        "-Wl,-rpath <!(pwd)/deps/seifrng/lib/"
                    ],
                }],
                [ 'OS=="win"', {
                    "include_dirs": [
                        "C:/cryptopp",
                        "<!(node -e \"require('nan')\")",
                        "<!(pwd)/deps/seifrng/isaacRandomPool/include",
                        "<!(pwd)/deps/seifrng/isaacrng/include",
                        "<!(pwd)/deps/seifrng/fileCryptopp/include"
                    ],
                    "libraries": [
                        "C:/cryptopp/x64/Output/Release/cryptlib.lib",
                        "<!(pwd)/deps/seifrng/lib/*"
                    ],
                }],
            ]

        }
    ]
}