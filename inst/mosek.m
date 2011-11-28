## -*- texinfo -*-
## @deftypefn{Loadable Function} {@var{r} =} mosek (@var{problem}, @var{opts} {= struct()})
## 
## >> Solve an optimization problem.
## 
## Solve an optimization problem using the MOSEK optimization library. 
## Please see the 'userguide.pdf' for a detailed introduction to this pacakge. 
## This file is located in the "doc" directory at the root of this package:
## @example
## @group
## pkg_root = pkg("list")@{strcmp(@{[pkg("list")@{:@}].name@},"octmosek")@}.dir;
## fullfile(pkg_root, "doc", "userguide.pdf")
## @end group
## @end example
##
## @sp 1
## ========== Arguments ==========
## @sp 1
## @multitable {.......................} {..................} {...........}
## @item problem                         @tab STRUCTURE         @tab                    
## @item ..sense                         @tab STRING            @tab                    
## @item ..c                             @tab REAL VECTOR       @tab                    
## @item ..c0                            @tab SCALAR            @tab (OPTIONAL)         
## @item ..A                             @tab SPARSE MATRIX     @tab                    
## @item ..blc                           @tab REAL VECTOR       @tab                    
## @item ..buc                           @tab REAL VECTOR       @tab                    
## @item ..blx                           @tab REAL VECTOR       @tab                    
## @item ..bux                           @tab REAL VECTOR       @tab                    
## @item ..cones                         @tab CELL              @tab (OPTIONAL)         
## @item ....@{i@}.type                  @tab STRING            @tab                    
## @item ....@{i@}.sub                   @tab INTEGER VECTOR    @tab                    
## @item ..intsub                        @tab INTEGER VECTOR    @tab (OPTIONAL)         
## @item ..iparam/dparam/sparam          @tab STRUCTURE         @tab (OPTIONAL)         
## @item ....<MSK_PARAM>                 @tab STRING / SCALAR   @tab (OPTIONAL)         
## @item ..sol                           @tab STRUCTURE         @tab (OPTIONAL)         
## @item ....itr/bas/int                 @tab STRUCTURE         @tab (OPTIONAL)
## @end multitable
##
## @multitable {.......................} {..................} {...........}
## @item opts                            @tab STRUCTURE          @tab (OPTIONAL)         
## @item ..verbose                       @tab SCALAR             @tab (OPTIONAL)         
## @item ..usesol                        @tab BOOLEAN            @tab (OPTIONAL)         
## @item ..useparam                      @tab BOOLEAN            @tab (OPTIONAL)         
## @item ..writebefore                   @tab STRING (filepath)  @tab (OPTIONAL)         
## @item ..writeafter                    @tab STRING (filepath)  @tab (OPTIONAL)         
## @end multitable
##
## The optimization problem should be described in a structure of definitions. 
## The number of variables in the problem is determined from the number of 
## columns in the constraint matrix @var{A}.
##
## Like a Linear Program it has a linear objective with one coefficient in 
## @var{c} for each variable, some optional constant @var{c0}, and the improving
## direction @var{sense}. The constraints can either be linear, specified as 
## rows in @var{A} with lower bounds @var{blc} and upper bounds @var{buc} (you 
## can use @var{Inf} if needed), or conic as specified in the list @var{cones} 
## (add constraints copyx=x if some variable x appears in multiple cones). 
## Each variable is bounded by @var{blx} and @var{bux} and will be integer if 
## it appears in the @var{intsub} list.
## 
## Parameters can also be specified for the MOSEK call. @var{iparam} is integer-
## typed parameters, @var{dparam} ia double-typed parameters and @var{sparam} 
## is string-typed parameters. These parameters can be ignored by setting the 
## option @var{useparam} to FALSE (the default is TRUE).
##
## Initial solutions are specified in @var{sol} and should have the same format 
## as the solution returned by the function call. This solution can be ignored 
## by setting the option @var{usesol} to FALSE (the default is TRUE).
##
## The amount of information printed by the interface can be limited by 
## @var{verbose} (default=10). The generated model can be exported to any 
## standard modelling fileformat (e.g. lp, opf, lp or mbt), with (resp. without) 
## the identified solution using @var{writeafter} (resp. @var{writebefore}). 
##
## The optimization process can be terminated at any moment using CTRL + C.
##
## @multitable {.......................} {....................................} 
## @item problem                         @tab Problem description
## @item ..sense                         @tab Objective sense, e.g. "max" or "min"
## @item ..c                             @tab Objective coefficients
## @item ..c0                            @tab Objective constant
## @item ..A                             @tab Constraint matrix
## @item ..blc                           @tab Constraint lower bounds
## @item ..buc                           @tab Constraint upper bounds
## @item ..blx                           @tab Variable lower bounds
## @item ..bux                           @tab Variable upper bounds
## @item ..cones                         @tab Conic constraints
## @item ....@{i@}.type                  @tab Cone type 
## @item ....@{i@}.sub                   @tab Cone variable indexes 
## @item ..intsub                        @tab Integer variable indexes 
## @item ..iparam/dparam/sparam          @tab Parameter list 
## @item ....<MSK_PARAM>                 @tab Value of any <MSK_PARAM> 
## @item ..sol                           @tab Initial solution list 
## @item ....itr/bas/int                 @tab Initial solution description 
## @end multitable
##
## @multitable {.......................} {...............................................} 
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
## @multitable {.......................} {.................} {...................}
## @item r				@tab STRUCTURE		@tab 			
## @item ..response			@tab STRUCTURE		@tab 			
## @item ....code			@tab SCALAR		@tab 			
## @item ....msg			@tab STRING		@tab 			
## @item ..sol				@tab STRUCTURE		@tab 			
## @item ....itr/bas/int		@tab STRUCTURE		@tab (SOLVER DEPENDENT) 
## @item ......solsta			@tab STRING		@tab 			
## @item ......prosta			@tab STRING		@tab 			
## @item ......skx			@tab STRING VECTOR	@tab 			
## @item ......skc			@tab STRING VECTOR	@tab 			
## @item ......xx			@tab REAL VECTOR	@tab 			
## @item ......xc			@tab REAL VECTOR	@tab 			
## @item ......slc			@tab REAL VECTOR	@tab (NOT IN int) 	
## @item ......suc			@tab REAL VECTOR	@tab (NOT IN int) 	
## @item ......slx			@tab REAL VECTOR	@tab (NOT IN int) 	
## @item ......sux 			@tab REAL VECTOR	@tab (NOT IN int) 	
## @item ......snx 			@tab REAL VECTOR	@tab (NOT IN int/bas) 
## @end multitable
## 
## The result is a named list containing the response of the MOSEK optimization 
## library. A response code of zero is the signal of success.
##
## Depending on the specified solver, one or more solutions mays be returned. 
## The interior-point solution @var{itr}, the basic (corner point) solution 
## @var{bas}, and the integer solution @var{int}.
##
## The problem status @var{prosta} in all solutions shows the feasibility of 
## your problem description. All solutions are described by a solution status 
## @var{solsta} (e.g. optimal) along with the variable and constraint activities. 
## All activities will further have a bound key that specify their value 
## in relation to the declared bounds.
##
## Dual variables are returned for all defined bounds wherever possible. 
## Integer solutions @var{int} does not have any dual variables as such 
## definitions would not make sense. Basic (corner point) solutions @var{bas} 
## would never be returned if the problem had conic constraints, and 
## does not define @var{snx}.
##
## @multitable {.......................} {............................................} 
## @item r				@tab Result 
## @item ..response			@tab Response from the MOSEK optimization library 
## @item ....code			@tab ID-code of response 
## @item ....msg			@tab Human-readable message 
## @item ..sol				@tab All solutions identified 
## @item ....itr/bas/int		@tab Solution description 
## @item ......solsta			@tab Solution status 
## @item ......prosta			@tab Problem status  
## @item ......skx			@tab Variable bound keys  
## @item ......skc			@tab Constraint bound keys  
## @item ......xx			@tab Variable activities  
## @item ......xc			@tab Constraint activities  
## @item ......slc			@tab Dual variable for constraint lower bounds  
## @item ......suc			@tab Dual variable for constraint upper bounds  
## @item ......slx			@tab Dual variable for variable lower bounds  
## @item ......sux 			@tab Dual variable for variable lower bounds  
## @item ......snx 			@tab Dual variable of conic constraints 
## @end multitable
##
## @sp 1
## ========== Examples ==========
## @sp 1
## @example
## @group
## clear -v lo1;
## lo1.sense = "max";
## lo1.c = [3 1 5 1];
## lo1.A = sparse([3 1 2 0;
##                 2 1 3 1;
##                 0 2 0 3]);
## lo1.blc = [30 15 -inf];
## lo1.buc = [30 inf 25];
## lo1.blx = [0 0 0 0];
## lo1.bux = [inf 10 inf inf];
## r = mosek(lo1);
## @end group
## @end example
##
## @seealso{mosek_version,mosek_clean}
##
## @end deftypefn 

function r = mosek(problem, opts=struct())

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

      r = __mosek__(problem, opts);

    catch
      error(strcat(lasterr,"\n"));    % Newline prevents printing call-sequence
    end_try_catch
  unwind_protect_cleanup
    page_screen_output(old_val);
  end_unwind_protect

endfunction
