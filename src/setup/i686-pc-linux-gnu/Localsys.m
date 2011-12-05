##
## Greetings user of the Octave-to-MOSEK interface!
##
## If you are sitting on a LINUX i686 platform, this is the file that
## you will have to setup before this package can be installed.
## (see e.g. the Octave-to-MOSEK userguide)
##

##################
## Step 1 of 2  ##
##################
## Please substitute [MOSEK_HOME_PATH] below, with the path to the platform-
## specific folder within the MOSEK installation you want to use. Note that
## this path should contain a "bin" and a "h" folder.
## ----------------------------------------------------------------------
## For example you can write:
##    PKG_MOSEKHOME="~/mosek/6/tools/platform/linux32x86";
##
## If your computer contains the two directories:
##    ~/mosek/6/tools/platform/linux32x86/bin
##    ~/mosek/6/tools/platform/linux32x86/h
## ----------------------------------------------------------------------
##

PKG_MOSEKHOME="[MOSEK_HOME_PATH]";


##################
## Step 2 of 2  ##
##################
## Please substitute [MOSEK_LIB_FILE] below, with the name of the library you 
## wish to use within the "bin" folder of your PKG_MOSEKHOME path. Note that 
## this "bin" folder must contain a file called lib[MOSEK_LIB_FILE].so or .dylib.
## ----------------------------------------------------------------------
## Continuing the example from above, you can write:
##    PKG_MOSEKLIB="mosek";
##
## If your computer contains the file:
##    ~/mosek/6/tools/platform/linux32x86/bin/libmosek.so
##
## or
##    ~/mosek/6/tools/platform/linux32x86/bin/libmosek.dylib
##
## depending on your Unix-alike system.
## ----------------------------------------------------------------------
##

PKG_MOSEKLIB="[MOSEK_LIB_FILE]";

