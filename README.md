Quick build instructions:

```bash
sudo apt-get install build-essential libsdl2-dev
git clone git@github.com:daid/Geronimo.git
git clone git@github.com:daid/SeriousProton2.git
cd Geronimo
mkdir _build
cd _build
cmake ..
make -j 5
cd ..
./_build/Geronimo
```

