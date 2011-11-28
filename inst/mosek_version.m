## -*- texinfo -*-
## @deftypefn{Loadable Function} {@var{r} =} mosek_version ()
## 
## >> Version of the MOSEK optimization library.
##
## Retrieves a string @var{r}, containing the version number of the utilized 
## MOSEK optimization library.
##
## @seealso{mosek}
##
## @end deftypefn

function r = mosek_version()

  if (nargout > 1)
    printf("Invalid number of output arguments\n");
    print_usage();
  endif

  old_val = page_screen_output;
  unwind_protect
    page_screen_output(0);
    try

      r = __mosek_version__();

    catch
      error(strcat(lasterr,"\n"));    % Newline prevents printing call-sequence
    end_try_catch
  unwind_protect_cleanup
    page_screen_output(old_val);
  end_unwind_protect  

endfunction
