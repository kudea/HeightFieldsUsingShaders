Before you start the code, please check your system environment.

For the Windows platform, we provide the Visual Studio 2017 and 2019 solution/project files in ./hw1-starterCode (same files for both 2017 and 2019).

For Mac OS X, before you do any coding, you must install command-line utilities (make, gcc, etc.). Install XCode from the Mac app store, then go to XCode, and use "Preferences/Download" to install the command line tools. Important: If you are using Mac OS X Mojave, you need to update the OS to the latest version of Mojave. Otherwise, OpenGL does not work. Or, you can use Catalina, Big Sur or Monterey.

On Linux, you need the libjpeg library, which can be obtained by "sudo apt-get install libjpeg62-dev libglew-dev". For Windows and Mac OS X, the starter code contains a precompiled jpeg library. On Intel-based Apple chips, the jpeg library should work as is. On Apple M1 chips (https://en.wikipedia.org/wiki/Apple_M1), you need to take the following steps to get jpeg library to compile and link:

// Do this for Apple M1 chips ONLY. If you don't do it, you will
// get linker errors related to the libjpeg library.
// The below steps are not necessary for Windows, Linux or Intel-based Apple computers.
```
cd external/jpeg-9a-mac

chmod +x configure
./configure --prefix=$(pwd)

make clean

make
chmod +x install-sh
make install
```
