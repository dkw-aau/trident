# Trident

This is a modified version of the Trident triple store. This modification has the node B+ tree substituted with the <a href="https://github.com/microsoft/ALEX">ALEX</a> learned index.

## Installation with Docker

Build the Docker image

```bash
docker build -t trident-clone .
```

## Installation on Ubuntu (or similar Linux Distros)

The installation should be fairly easy on Ubuntu (or on similar Linux distro
that provide packages of commonly-used libraries). Make sure you have
the following packages installed (with apt-get or similar):

- git
- libboost-all-dev
- liblz4-dev
- libtbb-dev
- libsparsehash-dev
- python3-dev
- libcurl-dev
- cmake

Then, clone Trident (if you have not already done so):

```bash
git clone git@github.com:karmaresearch/trident.git
```

This will create a `trident` directory. Next, create a build directory in the main source directory
(e.g build), go to this directory and launch `cmake` (see below for some special parameters like DEBUG
mode, etc.). So:

```bash
cd trident
mkdir build
cd build
cmake -DSPARQL=1 -DSERVER=1 ..
```

This will create some configuration files. Then, a simple

```bash
make
```

should compile everything. The compilation process creates a: 1) static library called
"libtrident.a", 2) a Python module called "trident.so" (remember to add this
path to the environmental variable PYTHONPATH if you want to use Trident with
Python), 3) an executable called "trident_exec". 


## Installation on MacOS or Windows

On Windows, the process is exactly the same as on Ubuntu.
On MacOS, the process is very similar to Ubuntu. You can install the libraries that
are mentioned with Homebrew.

## More advanced info on compilation

The compilation relies on CMake to create the executable and libraries. CMake
requires that you create a new directory and run the program "cmake" inside it
pointing to another location that contains the file CMakeLists.txt. This
program will create all sorts of files which will instruct "make" to build
everything.  Suppose we create a directory "build" inside the main source tree
with:

```bash
mkdir build
```

Then we move there

```bash
cd build
```

and create all the setup files to compile the program

```bash
cmake -SPARQL=1 -DSERVER=1 ..
```

If you want to create a debug version of the program, add the following
parameter to cmake:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

Exchange `Debug` with `Release` to build the release version of Trident.

Trident requires that the Boost libraries are compiled with multi-threading
support and should be available in accessable locations. Trident also requires
Intel's Thread Building Block libraries and libcurl. These three libraries will
not be installed by the Trident compilation routines, so you must be sure they
are already installed.

Trident uses other libraries (e.g. Google's sparsehash library, LZ4, etc.),
but if these other libraries are not available then CMake will (or should)
automatically download and compile them.

Trident heavily relies on KOGNAC (an advanced compression library) to encode
the knowledge graph and for various other routines. KOGNAC is available at <a 
href="https://github.com/jrbn/kognac">here</a>. The default behaviour of
Trident is to download and compile KOGNAC during the building process. If, for
any reason, you want to use an pre-existing version of KOGNAC, you can point
Trident to the locations of KOGNAC with the two parameters to pass to cmake:

```
-DKOGNAC_LIB=<absolute path to the KOGNAC library>/libkognac.a
-DKOGNAC_INC=<absolute path to the KOGNAC include files>
```

Remember a few things: If your system uses a version of Boost which is older
than 1.62, then you might get many warnings during the compilation. These
warnings are not problematic since they relate to a bug in the Boost libraries.

## Troubleshooting

### It does not compile!

I'm sorry to hear this. I did my best to make the compilation process as robust
as possible, but things can always go wrong.

One reason could be that you are using a old compiler. Trident relies on C++11
so it requires a modern compiler. Also, it assumes that the system can handle
64bit numbers well. This means that a "long" should correspond to 64bit while a
"int" points to a 32bit number.

Another reason for failure could be that in the source code of Trident there is
also a (minimally modified) version of the SNAP library. This library must be
compiled first, otherwise Trident won't be able to compile. The CMake process
should automatically take care of this, but if things go wrong here you have a
problem. To check SNAP has been correctly compiled, make sure that the file
"snap/snap-core/libsnap.a" exists.

## Tests

Tests are located in the `tests` folder in which a single `Makefile` wraps execution of all tests.
To run a test, simply type

```bash
make <TEST NAME>
```

For example, to run tests for the learned index, type

```bash
make testlearnedindex
```

To run the learned index test in Docker, type

```bash
docker run trident-clone cd ../tests && make testlearnedindex
```

## Usage

These are the commands that can be passed to the _trident_ binary.

For any of the commands, the option `-l` can be used to set logging level. For example, `-l debug` will set the debugging level to _debug_.

#### Load

To load Trident, use the _load_ command

```bash
./trident load -f <FOLDER WITH DATA> -i <DATA FOLDER>
```

The option `-f` must be followed by a folder containing RDF files.
The option `-i` must be followed by a non-existing folder in which trident data will be stored.

#### Query

To query Trident, use the _query_ command

```bash
./trident query -i <DATA FOLDER> -q <QUERY FILE>
```

The `-i` option is the same as when using the `load` command.
The option `-q` option must be followed by a query file.

#### Server

To run the Trident server, use the _server_ command

```bash
./trident server -i <DATA FOLDER>
```

The `-i` option is the same as when using the `load` command.

To send a query to the Trident server, an HTTP POST request with the header parameter `'Content-Type': 'application/x-www-form-urlencoded'` is used. The data section in the request must contain a _print_ parameter to indicate whether to output the results (in JSON format) and the parameter _query_ to pass the actual query.
You can use the following simple Python script to send a simple query to retrieve all types:

```python
import requests
import json

url = 'http://localhost:8080/sparql'
query = 'SELECT DISTINCT ?t where { ?s a ?t }'
headers = {'Content-Type': 'application/x-www-form-urlencoded'}
data = 'print=true&query=' + query
r = requests.post(url, headers = headers, data = data)
json_r = json.loads(r.text)

print(str(json_r['results']['bindings']))
```

### Usage in Docker

Run any of the above commands with

```bash
docker run trident-clone ./trident <COMMAND>
```

## License

Trident is released under Apache 2 license. In the source directory
there is one directory called "rdf3x". This directory
contains a modified version of the RDF3X engine written by <a
href="https://db.in.tum.de/~neumann/">Thomas Neumann</a>. The source code of
RDF3X was released on  google code but unfortunately that repository is no
longer available. The original version of RDF3X is released with Creative
Commons. In order to comply with the original license, also this modified
version should be considered released with the same license. There is another directory, called "snap", which contains a modified version of the SNAP library developed at
Stanford (<a href="http://snap.stanford.edu/snap/index.html">Here</a> is the
link to the homepage of the project). I modified the code of SNAP in order to
make it more efficient and to be able to scale up to larger graphs. My changes are contained in the "snap", "src/snap" and "include/snap" directories. Since the original version of SNAP is
released with the BSD license, then also my modifications are released with
the same license. This means that everything under these three directories should be considered released under BSD while everything in the directory "rdf3x" should be considered under Creative Commons.
