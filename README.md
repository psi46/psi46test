psi46test README
====================================================================

*NOTE: Do not forget to read the Changelog.txt file as well for latest
      changes to the code.*

Prerequisites:
--------------

The FTDI D2XX drivers for USB are needed. If not yet installed, download
them from

    http://www.ftdichip.com/Drivers/D2XX.htm

and follow the instructions in the ReadMe file there. You only need to install
the shared library. No need for the examples.


Installation:
-------------

Linux/Darwin (Mac OS X):

  1. Compile the software by typing

	   `make` 


  2. If your environment variable PATH contains /usr/local/bin, skip to the
     next step. You can find out if you type

	 `echo $PATH`

     If the path is not in the variable then edit your ~/.bashrc file and
     append

	 `export PATH=$PATH:/usr/local/bin`

     Newly opened shells will then have the PATH environment variable set
     correctly.


  3. If your environment variable LD_LIBRARY_PATH contains /usr/local/lib,
     skip to the next step. You can find out by typing

	`echo $LD_LIBRARY_PATH`

     If the path is missing, edit your ~/.bashrc file and append

	`export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib`

     Newly opened shells will then have the LD_LIBRARY_PATH environment
     variable set correctly.


  4. (Skip on a Mac)
     If you want regular users to use the software you must grant them
     permission to the USB devices. You can do this by adding the file
     /etc/udev/rules.d/10-testboard.rules (needs sudo) or edit it, if the
     file already exists. Add the line

	`SUBSYSTEM == "usb", ATTR{manufacturer}=="PSI", GROUP="usb", MODE="0664"`

     and it should work now.


  5. If you get an error that your USB device is busy, then your Linux
     box has a generic serial driver for the FTDI chip already installed. 
     To prevent the driver ftdi_sio from taking ownership of the testboard
     check first if this is really the case. Disconnect and reconnect 
     the testboard. Then issue

	`dmesg | tail -20`

     If you see that the driver ftdi_sio gets loaded and a serial usb 
     interface has been established for your testboard, then you lost.
     Fix it as follows:

     Add a line with "blacklist ftdi_sio" to /etc/modprobe.d/blacklist.conf
     (needs sudo) and issue the command sudo update-initramfs -u. After a 
     reboot test it again. If the serial USB connection is no longer 
     established you should be able to talk to the board.

     This comes at a disadvantage: Every other USB device using that chip
     may stop to work. So be careful. If you have a better solution report
     this or even better update this README.


  6. Run

	`bin/psi46test <logfilename>`

     <logfilename> is just a name of your choice for the logfile that 
     psi64test is going to write to. It is recreated every time anew, i.e.
     it overwrites any old one with the same name.

Common issues
-------------

  * Some Ubuntu installations miss header. When you call make the compiler
    will complain. You can install them with

	`sudo apt-get install libreadline-dev libx11-dev`


  * Some Ubuntu installations show take over of the device by ehci_hcd. 
    Blacklisting doesn't help. Currently we are stuck. If anybody found
    a solution, please let us know and update this file.

  * On Ubuntu 12.04 at least one user report says that you will need to add

	`-ltermcap`

    to the variable LDFLAGS in the Makefile (in the Linux case, not Darwin).

  * If you are on Mac OS 10.8:
    You must install X11 (http://support.apple.com/kb/HT5293) and create
    a symbolic link
    
	`ln -s /opt/X11/include/X11 /usr/local/include/X11`

  * If you are on Mac OS 10.7 (or lower):
    make sure you have the most recent version of Xcode and gcc installed.
    See
    
	http://woss.name/2012/01/24/how-to-install-a-working-set-of-compilers-on-mac-os-x-10-7-lion/
	
    for instructions.

  * If you are on Max OS 10.6:
    If you managed to make it work, let us know. We did not.

  * Some older compilers complain about -Wno-logical-op-parentheses in the
    invocation of the C++ compiler. Just remove this in the Makefile and
    it should work.

