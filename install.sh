#!/bin/bash

echo "╔══╣ Install: CCF Person Identification (STARTING) ╠══╗"


# Get current directory
DIR=`pwd`

# Download repository
# cd ../
# git clone https://github.com/TeamSOBITS/monocular_people_tracking


# Download library
git clone -b v19.24.4 https://github.com/davisking/dlib.git
sudo rm -rf /opt/dlib
sudo mv dlib/ /opt/dlib/

echo "export DLIB_ROOT=/opt/dlib" >> ~/.bashrc
source ~/.bashrc

# Return to the original directory
cd $DIR


echo "╚══╣ Install: CCF Person Identification (FINISHED) ╠══╝"
