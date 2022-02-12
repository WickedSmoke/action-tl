exe %action-tl [
    cxxflags "-Wno-deprecated-copy"     ; C++11 bullshit
    linux [
        cxxflags "-std=c++11"           ; Needed on CentOS 7.
    ]
    qt [widgets]
    sources [
        %Timeline.cpp
        %PixmapChooser.cpp
        %icons.qrc
    ]
]
