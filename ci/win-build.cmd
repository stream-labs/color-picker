cmake -H. ^
      -B"build" ^
      -G"%WinGenerator%" ^
      -A%WinType% ^
      -DCMAKE_INSTALL_PREFIX="build\distribute\color-picker" ^
      -DNODEJS_NAME=%RuntimeName% ^
      -DNODEJS_URL=%RuntimeURL% ^
      -DNODEJS_VERSION=%RuntimeVersion%

cmake --build build --target install --config RelWithDebInfo