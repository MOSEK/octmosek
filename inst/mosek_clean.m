## -*- texinfo -*-
## @deftypefn{Loadable Function} {} mosek_clean ()
## 
## >> Releases an acquired MOSEK license.
##
## Forces the early release of any previously acquired MOSEK license. If you do 
## not share a limited number of licenses among multiple users, you do not need 
## to use this function. Notice that the acquisition of a new MOSEK license will
## automatically take place at the next call to the function @code{mosek} given 
## a valid problem description, using a small amount of extra time.
##
## @seealso{mosek}
##
## @end deftypefn

function mosek_clean()

  if (nargout > 0)
    printf("Invalid number of output arguments\n");
    print_usage();
  endif

  old_val = page_screen_output;
  unwind_protect
    page_screen_output(0);
    try

      __mosek_clean__();

    catch
      error(strcat(lasterr,"\n"));    % Newline prevents printing call-sequence
    end_try_catch
  unwind_protect_cleanup
    page_screen_output(old_val);
  end_unwind_protect
  
endfunction
