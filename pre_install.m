function pre_install(desc = 0)

# TODO: Which is better?
# systemtype = upper(computer());
# isWindows = !isempty(findstr("MINGW", systemtype));
systemtype = upper(getenv("OS"));
isWindows = !isempty(findstr("WINDOWS", systemtype));

if (isWindows) 
  disp("Configuring for the WINDOWS platform");
else
  disp("Configuring for a UNIX-alike platform");
endif

LocalsysPath = strcat("./src/", systemtype, "/Localsys.m");

#
# ATTEMPT 1: READ CONFIGURATION FROM ENVIRONMENT
#
global PKG_MOSEKHOME;
global PKG_MOSEKLIB;

if (isValid(PKG_MOSEKHOME) || isValid(PKG_MOSEKLIB))
  disp("Found variable definitions in octave environment");
endif


#
# ATTEMPT 2: READ CONFIGURATION FROM LOCALSYS FILE
#
if (!isValid(PKG_MOSEKHOME) && !isValid(PKG_MOSEKLIB))
  
  if (exist(LocalsysPath, "file") == 2)
    source(LocalsysPath);
  endif
  
  if (isValid(PKG_MOSEKHOME) || isValid(PKG_MOSEKLIB))
    disp(cstrcat("Found variable definitions in ",LocalsysPath," file"));
  endif
  
endif


#
# ATTEMPT 3: GUESS CONFIGURATION FROM "mosek" command
#
if (!isValid(PKG_MOSEKHOME) && !isValid(PKG_MOSEKLIB))

  if (isWindows) 
    [sys_status, sys_output] = system("where mosek");
  else
    [sys_status, sys_output] = system("which mosek");
  endif
  
  if (sys_status == 0)
    PKG_MOSEKEXE = strsplit(sys_output,"\n"){1};
    PKG_MOSEKHOME = fileparts(fileparts(PKG_MOSEKEXE));
    
    if (isWindows) 
      [sys_status, sys_output] = system(cstrcat("where /R \"",PKG_MOSEKHOME,"\" mosek*.lib\""));
    else
      [sys_status, sys_output] = system(cstrcat("find ",PKG_MOSEKHOME," -name 'libmosek*.*'"));
    endif
    
    if (sys_status == 0)
      PKG_MOSEKLIBPATHS = strsplit(sys_output,"\n");
    
      if (isWindows)
        [ans,ans,ans,ans,moseklib_postfix] = regexp(PKG_MOSEKLIBPATHS, "mosek([0-9_]*).lib*$");
      else
        [ans,ans,ans,ans,moseklib_postfix] = regexp(PKG_MOSEKLIBPATHS, "libmosek([0-9_]*).(so|dylib)*$");
      endif
      
      for ID = moseklib_postfix
        if !isempty(ID{1})
          PKG_MOSEKLIB = strcat("mosek", ID{1}{1}{1});
        endif
      endfor
    endif
  endif
  
  if (isValid(PKG_MOSEKHOME) || isValid(PKG_MOSEKLIB))
    if (isWindows) 
      disp(cstrcat("Guessed variable definitions from command 'mosek' in Windows CMD"));
    else
      disp(cstrcat("Guessed variable definitions from shell command 'mosek'"));
    endif
  endif
  
endif


#
# CHECK THAT WE HAVE ALL WE NEED
#
if (!isValid(PKG_MOSEKHOME) && !isValid(PKG_MOSEKLIB))
  disp("*** No variable 'PKG_MOSEKHOME' in octave environment ***");
  disp(cstrcat("*** No variable 'PKG_MOSEKHOME' in ",LocalsysPath," file ***"));
  
  if (isWindows) 
    disp("*** Could not guess variable 'PKG_MOSEKHOME' from shell command 'mosek' ***");
  else
    disp("*** Could not guess variable 'PKG_MOSEKHOME' from command 'mosek' in Windows CMD ***");
  endif
  
  error();
endif

if !isValid(PKG_MOSEKHOME)
  disp("*** Variable 'PKG_MOSEKHOME' was not found here ***");
  error();
endif

if !isValid(PKG_MOSEKLIB)
  disp("*** Variable 'PKG_MOSEKLIB' was not found here ***");
  error();
endif


#
# FORMAT AND VERIFY 'PKG_MOSEKHOME' AS A DIRECTORY
#
if (isWindows)
  PKG_MOSEKHOME = strrep(PKG_MOSEKHOME, '\', '/');
endif

if (exist(PKG_MOSEKHOME, "dir") != 7)
  disp(cstrcat("*** Variable 'PKG_MOSEKHOME' is not a directory: ",PKG_MOSEKHOME," ***"));
  error();
endif 


#
# PERFORM CONFIGURATION
#
disp(cstrcat("Using MOSEK home directory: ",PKG_MOSEKHOME));
disp(cstrcat("Using MOSEK library: ",PKG_MOSEKLIB));

# Escape backslashes on all output
PKG_MOSEKHOME_ESC = strrep(PKG_MOSEKHOME, '\', '\\');
PKG_MOSEKLIB_ESC = strrep(PKG_MOSEKLIB, '\', '\\');

# Print output
file = fopen("src/configure.in","wt");
fprintf(file, strcat("PKG_MOSEKHOME:=",PKG_MOSEKHOME_ESC,"\n"));
fprintf(file, strcat("PKG_MOSEKLIB:=",PKG_MOSEKLIB_ESC,"\n"));
fprintf(file,"\n");
fclose(file);

disp("Configuration done.");
endfunction


#
# CHECKS IF A VARIABLE DEFINITION IS VALID
#
function ret = isValid(var)
  ret = false;

  if (isempty(var))
    return;
  endif

  if !strcmp(class(var), "char")
    return;
  endif

  if strcmp(var(1), "[") || strcmp(var(end), "]") 
    return;
  endif
  
  ret = true;
endfunction
