## -*- texinfo -*-
## @deftypefn{Loadable Function} {@var{r} =} mosek_write (@var{problem}, @var{modelfile}, @var{opts} {= struct()})
## 
## >> Write problem to a model file.
##
## Outputs a model of an optimization problem in any standard modelling 
## fileformat (e.g. lp, opf, mps, mbt, etc.), controlled by a set of options. 
## The modelling fileformat is selected based on the extension of the modelfile.
## 
## @sp 1
## ========== Arguments ==========
## @sp 1
## @multitable {..............} {..................} {...........}
## @item problem 			 @tab STRUCTURE		@tab			
## @end multitable
##
## @multitable {..............} {..................} {...........}
## @item modelfile 			 @tab STRING (filepath)	@tab			
## @end multitable
##
## @multitable {..............} {..................} {...........}
## @item opts                            @tab STRUCTURE          @tab (OPTIONAL)         
## @item ..verbose                       @tab SCALAR             @tab (OPTIONAL)         
## @item ..usesol                        @tab BOOLEAN            @tab (OPTIONAL)         
## @item ..useparam                      @tab BOOLEAN            @tab (OPTIONAL)         
## @item ..writebefore                   @tab STRING (filepath)  @tab (OPTIONAL)         
## @item ..writeafter                    @tab STRING (filepath)  @tab (OPTIONAL)         
## @end multitable
##
## The @var{problem} should be compliant with the input specification of 
## function @code{mosek}. Please see this function for more details.
##
## The @var{modelfile} should be an absolute path to the model file. If the 
## file extension is @code{.opf}, the model will be written in the Optimization 
## Problem Format. Other formats include lp, mps and mbt.
##
## The amount of information printed by the interface can be limited by 
## @var{verbose} (default=10). Whether to write the initial solution, if one 
## such exists in the problem description, is indicated by @var{usesol} which, 
## by default, is FALSE. Whether to write the full list of parameter settings, 
## some of which may have been specified by the problem description, 
## is indicated by @var{useparam} which by default is FALSE.
##
## @multitable {..............} {..................} {...........}
## @item problem 			 @tab Problem desciption
## @end multitable
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
##
## @multitable {..............} {..................} {...........}
## @item r				@tab STRUCTURE		@tab 			
## @item ..response			@tab STRUCTURE		@tab 			
## @item ....code			@tab SCALAR		@tab 			
## @item ....msg			@tab STRING		@tab 			
## @end multitable
##
## The result is a named list containing the response of the MOSEK optimization 
## library when writing to the model file. A response code of zero is the signal
## of success.
##
## @multitable {..............} {............................................} 
## @item r				@tab Result 
## @item ..response			@tab Response from the MOSEK optimization library 
## @item ....code			@tab ID-code of response 
## @item ....msg			@tab Human-readable message 
## @end multitable
##
## @sp 1
## ========== Examples ==========
## @sp 1
##
## @example
## @group
## clear -v lo1;
## lo1.sense = "max";
## lo1.c = [3 1 5 1];
## lo1.A = sparse([3 1 2 0;
##                 2 1 3 1;
##                 0 2 0 3]);
## lo1.blc = [30 15 -Inf];
## lo1.buc = [30 Inf 25];
## lo1.blx = [0 0 0 0];
## lo1.bux = [Inf 10 Inf Inf];
## rr = mosek_write(lo1, "lo1.opf")
## if (rr.response.code ~= 0)
##   error("Failed to write model file to current working directory");
## endif
## @end group
## @end example
##
## @seealso{mosek,mosek_read}
##
## @end deftypefn                              

function r = mosek_write(problem, modelfile, opts=struct())

  if (nargin < 2 || nargin > 3 || nargout > 1)
    print_usage();
  endif

  old_val = page_screen_output;
  unwind_protect
    page_screen_output(0);
    try

      r = __mosek_write__(problem, modelfile, opts);

    catch
      error(strcat(lasterr,"\n"));    % Newline prevents printing call-sequence
    end_try_catch
  unwind_protect_cleanup
    page_screen_output(old_val);
  end_unwind_protect
  
endfunction

