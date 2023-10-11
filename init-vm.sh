sudo apt-get update
sudo apt-get upgrade -y

# Install C++
sudo apt-get install build-essential -y
gcc --version

# Install lcov for code coverage
sudo apt-get -y install lcov

# Install Boost
sudo apt-get install libboost-all-dev -y

# Install Valgrind
sudo apt-get -y install valgrind

# Install GDB
sudo apt-get -y install gdb

# Install CMake3
sudo apt-get install cmake -y
cmake --version

# Install Doxygen
sudo apt-get -y install doxygen
doxygen --version

# Install Catch2
sudo apt-get -y install catch2

# Install curl
sudo apt-get -y install curl

# Install Java SDK 18
# sudo apt-get install -y openjdk-18-jdk
# sudo apt-get install -y openjdk-18-jre
# sudo apt-get install -y libc6-x32 libc6-i386
# wget https://download.oracle.com/java/18/latest/jdk-18_linux-x64_bin.deb
# sudo apt-get install ./jdk-18_linux-x64_bin.deb
# cat <<EOF | sudo tee /etc/profile.d/java.sh
# export JAVA_HOME=/usr/lib/jvm/jdk-18
# export PATH=\$PATH:\$JAVA_HOME/bin
# EOF
# source /etc/profile.d/jdk18.sh
# java -version

# Git
# Install Git
sudo apt-get install git -y
# Set default editor to vim
# cd /order-matching-engine
# git config --global core.editor "vim"
# git config credential.helper store