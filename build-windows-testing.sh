DEST_FOLDER=artifact

mkdir ${DEST_FOLDER} || true

cp build/firelight.exe ${DEST_FOLDER}/firelight.exe

#/c/Qt/6.6.1/mingw_64/bin/windeployqt.exe --add-plugin-types=imageformats,iconengines,platforms,platforminputcontexts,\
#qmltooling,quick,quickcontrols2,quicktemplates2,quickwidgets,sql,tls,styles,sqldrivers,svg,translations,webp \
# ${DEST_FOLDER}/firelight.exe

cp /c/msys64/mingw64/bin/libgcc_s_seh-1.dll ${DEST_FOLDER}/libgcc_s_seh-1.dll
cp /c/msys64/mingw64/bin/libstdc++-6.dll ${DEST_FOLDER}/libstdc++-6.dll
cp /c/msys64/mingw64/bin/libwinpthread-1.dll ${DEST_FOLDER}/libwinpthread-1.dll

cp /c/msys64/mingw64/bin/SDL2.dll ${DEST_FOLDER}/SDL2.dll
cp /c/msys64/mingw64/bin/libssl-3-x64.dll ${DEST_FOLDER}/libssl-3-x64.dll
cp /c/msys64/mingw64/bin/libcrypto-3-x64.dll ${DEST_FOLDER}/libcrypto-3-x64.dll

cp build/qtquickcontrols2.conf ${DEST_FOLDER}/qtquickcontrols2.conf

cp /c/msys64/mingw64/bin/Qt6Core.dll ${DEST_FOLDER}/Qt6Core.dll
cp /c/msys64/mingw64/bin/Qt6Gui.dll ${DEST_FOLDER}/Qt6Gui.dll
cp /c/msys64/mingw64/bin/Qt6Network.dll ${DEST_FOLDER}/Qt6Network.dll
cp /c/msys64/mingw64/bin/Qt6OpenGL.dll ${DEST_FOLDER}/Qt6OpenGL.dll
cp /c/msys64/mingw64/bin/Qt6Pdf.dll ${DEST_FOLDER}/Qt6Pdf.dll
cp /c/msys64/mingw64/bin/Qt6Qml.dll ${DEST_FOLDER}/Qt6Qml.dll
cp /c/msys64/mingw64/bin/Qt6Quick.dll ${DEST_FOLDER}/Qt6Quick.dll
cp /c/msys64/mingw64/bin/Qt6Quick3DUtils.dll ${DEST_FOLDER}/Qt6Quick3DUtils.dll
cp /c/msys64/mingw64/bin/Qt6Sql.dll ${DEST_FOLDER}/Qt6Sql.dll
cp /c/msys64/mingw64/bin/Qt6Svg.dll ${DEST_FOLDER}/Qt6Svg.dll
cp /c/msys64/mingw64/bin/Qt6VirtualKeyboard.dll ${DEST_FOLDER}/Qt6VirtualKeyboard.dll
cp /c/msys64/mingw64/bin/Qt6Qml.dll ${DEST_FOLDER}/Qt6Qml.dll
cp /c/msys64/mingw64/bin/Qt6QmlModels.dll ${DEST_FOLDER}/Qt6QmlModels.dll
cp /c/msys64/mingw64/bin/Qt6QmlWorkerScript.dll ${DEST_FOLDER}/Qt6QmlWorkerScript.dll
cp /c/msys64/mingw64/bin/Qt6QuickControls2.dll ${DEST_FOLDER}/Qt6QuickControls2.dll
cp /c/msys64/mingw64/bin/Qt6QuickControls2Impl.dll ${DEST_FOLDER}/Qt6QuickControls2Impl.dll
cp /c/msys64/mingw64/bin/Qt6QuickDialogs2.dll ${DEST_FOLDER}/Qt6QuickDialogs2.dll
cp /c/msys64/mingw64/bin/Qt6QuickDialogs2QuickImpl.dll ${DEST_FOLDER}/Qt6QuickDialogs2QuickImpl.dll
cp /c/msys64/mingw64/bin/Qt6QuickDialogs2Utils.dll ${DEST_FOLDER}/Qt6QuickDialogs2Utils.dll
cp /c/msys64/mingw64/bin/Qt6QuickEffects.dll ${DEST_FOLDER}/Qt6QuickEffects.dll
cp /c/msys64/mingw64/bin/Qt6QuickLayouts.dll ${DEST_FOLDER}/Qt6QuickLayouts.dll
cp /c/msys64/mingw64/bin/Qt6QuickShapes.dll ${DEST_FOLDER}/Qt6QuickShapes.dll
cp /c/msys64/mingw64/bin/Qt6QuickTemplates2.dll ${DEST_FOLDER}/Qt6QuickTemplates2.dll

mkdir ${DEST_FOLDER}/generic || true
cp /c/msys64/mingw64/share/qt6/plugins/generic/qtuiotouchplugin.dll ${DEST_FOLDER}/generic/qtuiotouchplugin.dll

mkdir ${DEST_FOLDER}/iconengines || true
cp /c/msys64/mingw64/share/qt6/plugins/iconengines/qsvgicon.dll ${DEST_FOLDER}/iconengines/qsvgicon.dll

mkdir ${DEST_FOLDER}/imageformats || true
cp /c/msys64/mingw64/share/qt6/plugins/imageformats/qgif.dll ${DEST_FOLDER}/imageformats/qgif.dll
cp /c/msys64/mingw64/share/qt6/plugins/imageformats/qicns.dll ${DEST_FOLDER}/imageformats/qicns.dll
cp /c/msys64/mingw64/share/qt6/plugins/imageformats/qico.dll ${DEST_FOLDER}/imageformats/qico.dll
cp /c/msys64/mingw64/share/qt6/plugins/imageformats/qjpeg.dll ${DEST_FOLDER}/imageformats/qjpeg.dll
## PDF??
cp /c/msys64/mingw64/share/qt6/plugins/imageformats/qsvg.dll ${DEST_FOLDER}/imageformats/qsvg.dll
cp /c/msys64/mingw64/share/qt6/plugins/imageformats/qtga.dll ${DEST_FOLDER}/imageformats/qtga.dll
cp /c/msys64/mingw64/share/qt6/plugins/imageformats/qtiff.dll ${DEST_FOLDER}/imageformats/qtiff.dll
cp /c/msys64/mingw64/share/qt6/plugins/imageformats/qwbmp.dll ${DEST_FOLDER}/imageformats/qwbmp.dll
cp /c/msys64/mingw64/share/qt6/plugins/imageformats/qwebp.dll ${DEST_FOLDER}/imageformats/qwebp.dll

mkdir ${DEST_FOLDER}/networkinformation || true
cp /c/msys64/mingw64/share/qt6/plugins/networkinformation/qnetworklistmanager.dll ${DEST_FOLDER}/networkinformation/qnetworklistmanager.dll

mkdir ${DEST_FOLDER}/platforminputcontexts || true
cp /c/msys64/mingw64/share/qt6/plugins/platforminputcontexts/qtvirtualkeyboardplugin.dll ${DEST_FOLDER}/platforminputcontexts/qtvirtualkeyboardplugin.dll

mkdir ${DEST_FOLDER}/platforms || true
cp /c/msys64/mingw64/share/qt6/plugins/platforms/qwindows.dll ${DEST_FOLDER}/platforms/qwindows.dll

mkdir ${DEST_FOLDER}/qml || true

mkdir ${DEST_FOLDER}/qmltooling || true
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_debugger.dll ${DEST_FOLDER}/qmltooling/qmldbg_debugger.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_inspector.dll ${DEST_FOLDER}/qmltooling/qmldbg_inspector.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_local.dll ${DEST_FOLDER}/qmltooling/qmldbg_local.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_messages.dll ${DEST_FOLDER}/qmltooling/qmldbg_messages.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_native.dll ${DEST_FOLDER}/qmltooling/qmldbg_native.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_nativedebugger.dll ${DEST_FOLDER}/qmltooling/qmldbg_nativedebugger.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_preview.dll ${DEST_FOLDER}/qmltooling/qmldbg_preview.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_profiler.dll ${DEST_FOLDER}/qmltooling/qmldbg_profiler.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_quick3dprofiler.dll ${DEST_FOLDER}/qmltooling/qmldbg_quick3dprofiler.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_quickprofiler.dll ${DEST_FOLDER}/qmltooling/qmldbg_quickprofiler.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_server.dll ${DEST_FOLDER}/qmltooling/qmldbg_server.dll
cp /c/msys64/mingw64/share/qt6/plugins/qmltooling/qmldbg_tcp.dll ${DEST_FOLDER}/qmltooling/qmldbg_tcp.dll

mkdir ${DEST_FOLDER}/sqldrivers || true
cp /c/msys64/mingw64/share/qt6/plugins/sqldrivers/qsqlite.dll ${DEST_FOLDER}/sqldrivers/qsqlite.dll
cp /c/msys64/mingw64/share/qt6/plugins/sqldrivers/qsqlmimer.dll ${DEST_FOLDER}/sqldrivers/qsqlmimer.dll
cp /c/msys64/mingw64/share/qt6/plugins/sqldrivers/qsqlodbc.dll ${DEST_FOLDER}/sqldrivers/qsqlodbc.dll
cp /c/msys64/mingw64/share/qt6/plugins/sqldrivers/qsqlpsql.dll ${DEST_FOLDER}/sqldrivers/qsqlpsql.dll

mkdir ${DEST_FOLDER}/tls || true
cp /c/msys64/mingw64/share/qt6/plugins/tls/qcertonlybackend.dll ${DEST_FOLDER}/tls/qcertonlybackend.dll
cp /c/msys64/mingw64/share/qt6/plugins/tls/qopensslbackend.dll ${DEST_FOLDER}/tls/qopensslbackend.dll
cp /c/msys64/mingw64/share/qt6/plugins/tls/qschannelbackend.dll ${DEST_FOLDER}/tls/qschannelbackend.dll

#mkdir ${DEST_FOLDER}/translations

#/c/Qt/6.6.1/mingw_64/bin/windeployqt.exe ${DEST_FOLDER}/firelight.exe