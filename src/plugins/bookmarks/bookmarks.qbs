import qbs 1.0

import QtcPlugin

QtcPlugin {
    name: "Bookmarks"

    Depends { name: "Qt.widgets" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "TextEditor" }

    files: [
        "bookmark.cpp",
        "bookmark.h",
        "bookmarkmanager.cpp",
        "bookmarkmanager.h",
        "bookmarks.qrc",
        "bookmarks_global.h",
        "bookmarksplugin.cpp",
        "bookmarksplugin.h",
    ]
}

