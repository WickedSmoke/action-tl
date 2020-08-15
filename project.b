rule %icons.cpp %icons.qrc {
    rcc-qt5 icons.qrc -o icons.cpp
}

exe %action-tl [
    debug
    objdir %obj
    cxxflags "-Wno-deprecated-copy"     ; C++11 bullshit
    qt [widgets]
    sources [
        %Timeline.cpp
        %icons.cpp
    ]
]
