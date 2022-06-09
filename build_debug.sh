if [ ! -d build ]
then
    mkdir build
fi

cd build
cmake -DSPARQL=1 -DSERVER=1 ..
make
cd ..
