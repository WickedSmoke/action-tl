options [
    -debug:   false         "Compile for debugging"
]

rule %icons.cpp %icons.qrc {
    $(RCC) icons.qrc -o icons.cpp
}

exe %action-tl [
    either -debug [debug] [release]
    objdir %obj
    cxxflags "-Wno-deprecated-copy"     ; C++11 bullshit
    linux [
        cxxflags "-std=c++11"           ; Needed on CentOS 7.
    ]
    qt [widgets]
    sources [
        %Timeline.cpp
        %PixmapChooser.cpp
        %icons.cpp
    ]
]
