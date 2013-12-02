echo "Installing packages required for compiling..."
apt-get build-dep lirc
sudo apt-get install dialog automake autoconf libtool
echo "Installing packages complete - Starting build..."
echo "Running pre-compile scripts..."
./autogen.sh
./configure -with-driver=userspace
echo "Compiling Lirc driver: lirc_xbox"
cd drivers/lirc_xbox/
make
echo "Installing newly compiled driver...."
sudo make install
echo "Installation complete"
echo "** Remember to add correct hardware.conf and lircd.conf to /etc/lirc/ **"

