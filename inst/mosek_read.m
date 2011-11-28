## -*- texinfo -*-
## @deftypefn{Loadable Function} {@var{r} =} mosek_read (@var{modelfile}, @var{opts} {= struct()})
## 
## >> Read problem from a model file.
## 
## Interprets a model from any standard modelling fileformat (e.g. lp, opf, 
## mps, mbt, etc.), controlled by a set of options. The result contains an 
## optimization problem which is compliant with the input specifications of 
## function @code{mosek}.
## 
## @sp 1
## ========== Arguments ==========
## @sp 1
## @multitable {..............} {..................} {...........}
## @item modelfile 			 @tab STRING (filepath)  @tab			
## @end multitable
##
## @multitable {..............} {..................} {...........}
## @item opts                            @tab STRUCTURE          @tab (OPTIONAL)         
## @item ..verbose                       @tab SCALAR             @tab (OPTIONAL)         
## @item ..usesol                        @tab BOOLEAN            @tab (OPTIONAL)         
## @item ..useparam                      @tab BOOLEAN            @tab (OPTIONAL)          
## @end multitable
##
## The @var{modelfile} should be an absolute path to a model file. 
##
## The amount of information printed by the interface can be limited by 
## @var{verbose} (default=10). Whether to read the initial solution, if one 
## such exists in the model file, is indicated by @var{usesol} which by default 
## is FALSE. Whether to read the full list of parameter settings, some of which 
## may have been changed by the model file, is indicated by @var{useparam} 
## which by default is FALSE.
##
## @multitable {..............} {...............................................} 
## @item modelfile 			 @tab Filepath to the model
## @end multitable
## 
## @multitable {..............} {...............................................} 
## @item opts                            @tab Options 
## @item ..verbose                       @tab Output logging verbosity 
## @item ..usesol                        @tab Whether to use the initial solution 
## @item ..useparam                      @tab Whether to use the specified parameter settings 
## @item ..writebefore                   @tab Filepath used to export model 
## @item ..writeafter                    @tab Filepath used to export model and solution 
## @end multitable
##
## @sp 1
## ========== Value ==========
## @sp 1
## @multitable {...............} {..................} {...........}
## @item r				@tab STRUCTURE		@tab 			
## @item ..response			@tab STRUCTURE		@tab 			
## @item ....code			@tab SCALAR		@tab 			
## @item ....msg			@tab STRING		@tab 			
## @item ..prob				@tab STRUCTURE		@tab			
## @end multitable
##
## The result is a named list containing the response of the MOSEK optimization 
## library when reading the model file. A response code of zero is the signal 
## of success.
##
## On success, the result contains the problem specification with all problem 
## data. This problem specification is compliant with the input specifications 
## of function @code{mosek}.
##
## @multitable {...............} {............................................} 
## @item r				@tab Result 
## @item ..response			@tab Response from the MOSEK optimization library 
## @item ....code			@tab ID-code of response 
## @item ....msg			@tab Human-readable message 
## @item ..prob				@tab Problem desciption
## @end multitable
##
## @sp 1
## ========== Examples ==========
## @sp 1
## @example
## @group
## pkg_root = pkg("list")@{strcmp(@{[pkg("list")@{:@}].name@},"octmosek")@}.dir;
## modelfile = fullfile(pkg_root, "extdata", "lo1.opf");
## rr = mosek_read(modelfile);
## if (rr.response.code ~= 0)
##   error("Failed to read model file");
## endif
## rlo1 = mosek(rr.prob);
## @end group
## @end example
##
## @example
## @group
## pkg_root = pkg("list")@{strcmp(@{[pkg("list")@{:@}].name@},"octmosek")@}.dir;
## modelfile = fullfile(pkg_root, "extdata", "milo1.opf");
## rr = mosek_read(modelfile);
## if (rr.response.code ~= 0)
##   error("Failed to read model file");
## endif
## rmilo1 = mosek(rr.prob);
## @end group
## @end example
##
## @example
## @group
## pkg_root = pkg("list")@{strcmp(@{[pkg("list")@{:@}].name@},"octmosek")@}.dir;
## modelfile = fullfile(pkg_root, "extdata", "cqo1.opf");
## rr = mosek_read(modelfile);
## if (rr.response.code ~= 0)
##   error("Failed to read model file");
## endif
## rcqo1 = mosek(rr.prob);
## @end group
## @end example
##
## @seealso{mosek,mosek_write}
##
## @end deftypefn                 

function r = mosek_read(modelfile, opts=struct())

  if (nargin < 1)
    printf("Invalid number of input arguments\n");
    print_usage();
  endif

  if (nargout > 1)
    printf("Invalid number of output arguments\n");
    print_usage();
  endif

  old_val = page_screen_output;
  unwind_protect
    page_screen_output(0);
    try
  
      r = __mosek_read__(modelfile, opts);

    catch
      error(strcat(lasterr,"\n"));    % Newline prevents printing call-sequence
    end_try_catch
  unwind_protect_cleanup
    page_screen_output(old_val);
  end_unwind_protect 
  
endfunction
