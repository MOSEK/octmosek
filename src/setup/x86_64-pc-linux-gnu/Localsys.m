## 
## Greetings user of the Octave-to-MOSEK interface!
## 
## If you are sitting on a LINUX x86_64 platform, this is the file that
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
##    PKG_MOSEKHOME=C:\Progra~2\Mosek\6\tools\platform\win32x86
##
## If your computer contains the two directories:
##    C:\Progra~2\Mosek\6\tools\platform\win32x86\bin
##    C:\Progra~2\Mosek\6\tools\platform\win32x86\h
## ----------------------------------------------------------------------
##

PKG_MOSEKHOME="[MOSEK_HOME_PATH]";


##################
## Step 2 of 2  ##
##################
## Please substitute [MOSEK_LIB_FILE] below, with the name of the library you 
## wish to use within the "bin" folder of your PKG_MOSEKHOME path. Note that 
## this "bin" folder must contain a file called [MOSEK_LIB_FILE].lib.
## ----------------------------------------------------------------------
## Continuing the example from above, you can write:
##    PKG_MOSEKLIB=mosek6_0
##
## If your computer contains the file:
##    C:\Progra~2\Mosek\6\tools\platform\win32x86\bin\mosek6_0.lib
## ----------------------------------------------------------------------
##

PKG_MOSEKLIB="[MOSEK_LIB_FILE]";
