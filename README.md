# Build
## Reference decoder
```
cd reference-decoder
unzip jm19.0.zip
cd JM
chmod +x unixprep.sh
./unixprep.sh
cd ..
patch -p0 -i mb-print.patch
cd JM
make ldecod
```

## Macroblock parser
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```