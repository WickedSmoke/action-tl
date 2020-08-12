exe %action-tl [
    debug
    objdir %obj
    cxxflags "-Wno-deprecated-copy"     ; C++11 bullshit
    qt [widgets]
    sources [
        %Timeline.cpp
    ]
]
