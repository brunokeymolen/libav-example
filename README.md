LIBAV_EXAMPLE
=============

Demonstrates how to demux video containers and decode the frames.
It is tested with .mkv files with one video stream only. 


License
=======

see LICENSE file.

Dependencies
============

If no package available (on my Ubuntu 16.04 the default libav package was too old), these steps can help you to install from sources.

Compile and Install libav 
-------------------------

https://libav.org/<br>
wget https://libav.org/releases/libav-12.3.tar.xz<br>
tar -xvf libav-12.3.tar.xz<br>
cd libav-12.3/<br>
<br>
./configure --prefix=/opt/keymolen<br>
Make<br>
sudo make install<br>


Compile and Install opencv 
--------------------------

https://riptutorial.com/opencv/example/15781/build-and-install-opencv-from-source<br>
<br>
git clone https://github.com/opencv/opencv.git<br>
<br>
cd opencv/<br>
sudo apt-get update<br>
sudo apt-get install build-essential<br>
sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev<br>
<br>
mkdir build<br>
cd build/<br>
sudo mkdir /opt/keymolen<br>
<br>
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/opt/keymolen -D INSTALL_PYTHON_EXAMPLES=ON -D INSTALL_C_EXAMPLES=ON -D OPENCV_GENERATE_PKGCONFIG=ON  ..<br>
make <br>
sudo make install<br>

Build
=====

make


