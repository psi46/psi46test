psi46test README
====================================================================

Installation:
-------------

Linux:

  1. Compile the software by typing

	make

  2. Install the software by typing (as root)

	make install

     which will install the psi46test executable in /usr/local/bin and the
     USB library to /usr/local/lib.

  3. If your environment variable PATH contains /usr/local/bin, skip to the
     next step. You can find out if you type

	echo $PATH

     If the path is not in the variable then edit your ~/.bashrc file and
     append

	export PATH=$PATH:/usr/local/bin

     Newly opened shells will then have the PATH environment variable set
     correctly.

  4. If your environment variable LD_LIBRARY_PATH contains /usr/local/lib,
     skip to the next step. You can find out by typing

	echo $LD_LIBRARY_PATH

     If the path is missing, edit your ~/.bashrc file and append

	export LD_LIBRARY_PATH:$LD_LIBRARY_PATH:/usr/local/lib

     Newly opened shells will then have the LD_LIBRARY_PATH environment
     variable set correctly.

  5. If you want regular users to use the software you must grant them
     permission to the USB devices. You can do this by editing the files
     in /etc/udev/rules.d. There you may find a line that reads

	ACTION=="add", SUBSYSTEM=="usb_device", PROGRAM="/bin/sh -c 'K=%k; K=$${K#usbdev}; printf bus/usb/%%03i/%%03i $${K%%%%.*} $${K#*.}'", NAME="%c", MODE="0644"

     Change MODE="0644" to MODE="0666". You must be root to do this. After
     a restart of the computer the new setting is effective.

     It does not matter whether usbserial or ftdi_si are loaded or not.
     At least not in kernel version 2.6.18.

  6. Have a psi46test.ini in your working directory, an example of which you
     can find in this package. To find the USB serial number of the testboard
     that you are using, start the psi46test program and type
     
	scan

     Alternatively you can find the serial number by executing the command

	/sbin/lsusb -v

     in the shell. Specify the serial number in the psi46test.ini file to
     automatically open the device at the program start.

  8. Run

	psi46test <logfilename>
