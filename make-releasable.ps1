cmake -B build_releasable -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -GNinja
cmake --build build_releasable
mkdir sp64-tt 
cp build_releasable/sp64-tt.exe sp64-tt/.
cp win32.tex sp64-tt/. 
cp README.md sp64-tt/.
Compress-Archive -Path ./sp64-tt -DestinationPath sp64-tt.zip
rmdir build_releasable -Recurse -Force
rmdir sp64-tt -Recurse -Force
