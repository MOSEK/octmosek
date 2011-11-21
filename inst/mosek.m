## -*- texinfo -*-
## @deftypefn{Loadable Function} {@var{r} =} mosek (@var{problem}, @var{opts} {= struct()})
## 
## >> Solve an optimization problem
## 
## Solve an optimization problem using the MOSEK Optimization Library. 
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
## @multitable {.........................} {.................} {..........}
## @item problem                         @tab STRUCTURE          @tab                    
## @item __.sense                        @tab STRING             @tab                    
## @item __.c                            @tab NUMERIC VECTOR     @tab                    
## @item __.c0                           @tab NUMERIC            @tab (OPTIONAL)         
## @item __.A                            @tab SPARSE MATRIX      @tab                    
## @item __.blc                          @tab NUMERIC VECTOR     @tab                    
## @item __.buc                          @tab NUMERIC VECTOR     @tab                    
## @item __.blx                          @tab NUMERIC VECTOR     @tab                    
## @item __.bux                          @tab NUMERIC VECTOR     @tab                    
## @item __.cones                        @tab CELL               @tab (OPTIONAL)         
## @item ____@{i@}.type                  @tab STRING             @tab                    
## @item ____@{i@}.sub                   @tab NUMERIC VECTOR     @tab                    
## @item __.intsub                       @tab NUMERIC VECTOR     @tab (OPTIONAL)         
## @item __.iparam/.dparam/.sparam       @tab STRUCTURE          @tab (OPTIONAL)         
## @item ____.<MSK_PARAM>                @tab STRING / NUMERIC   @tab (OPTIONAL)         
## @item __.sol                          @tab STRUCTURE          @tab (OPTIONAL)         
## @item ____.itr/.bas/.int              @tab STRUCTURE          @tab (OPTIONAL)
## @end multitable
##
## @multitable {.........................} {.................} {..........}
## @item opts                            @tab STRUCTURE          @tab (OPTIONAL)         
## @item __.verbose                      @tab NUMERIC            @tab (OPTIONAL)         
## @item __.usesol                       @tab BOOLEAN            @tab (OPTIONAL)         
## @item __.useparam                     @tab BOOLEAN            @tab (OPTIONAL)         
## @item __.writebefore                  @tab STRING (filepath)  @tab (OPTIONAL)         
## @item __.writeafter                   @tab STRING (filepath)  @tab (OPTIONAL)         
## @end multitable
##
## The optimization problem should be specified in a named list of definitions. 
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
## @multitable {.........................} {....................................} 
## @item problem                         @tab Problem description
## @item __.sense                        @tab Objective sense, e.g. "max" or "min"
## @item __.c                            @tab Objective coefficients
## @item __.c0                           @tab Objective constant
## @item __.A                            @tab Constraint matrix
## @item __.blc                          @tab Constraint lower bounds
## @item __.buc                          @tab Constraint upper bounds
## @item __.blx                          @tab Variable lower bounds
## @item __.bux                          @tab Variable upper bounds
## @item __.cones                        @tab Conic constraints
## @item ____@{i@}.type                  @tab Cone type 
## @item ____@{i@}.sub                   @tab Cone variable indexes 
## @item __.intsub                       @tab Integer variable indexes 
## @item __.iparam/.dparam/.sparam       @tab Parameter list 
## @item ____.<MSK_PARAM>                @tab Value of any <MSK_PARAM> 
## @item __.sol                          @tab Initial solution list 
## @item ____.itr/.bas/.int              @tab Initial solution description 
## @end multitable
##
## @multitable {.........................} {...............................................} 
## @item opts                            @tab Options 
## @item __.verbose                      @tab Output logging verbosity 
## @item __.usesol                       @tab Whether to use the initial solution 
## @item __.useparam                     @tab Whether to use the specified parameter settings 
## @item __.writebefore                  @tab Filepath used to export model 
## @item __.writeafter                   @tab Filepath used to export model and solution 
## @end multitable
##
## @sp 1
## ========== Value ==========
## @sp 1
## @multitable {.........................} {.................} {..................}
## @item r				@tab STRUCTURE		@tab 			
## @item __.response			@tab STRUCTURE		@tab 			
## @item ____.code			@tab NUMERIC		@tab 			
## @item ____.msg			@tab STRING		@tab 			
## @item __.sol				@tab STRUCTURE		@tab 			
## @item ____.itr/.bas/.int		@tab STRUCTURE		@tab (SOLVER DEPENDENT) 
## @item ______.solsta			@tab STRING		@tab 			
## @item ______.prosta			@tab STRING		@tab 			
## @item ______.skx			@tab STRING VECTOR	@tab 			
## @item ______.skc			@tab STRING VECTOR	@tab 			
## @item ______.xx			@tab NUMERIC VECTOR	@tab 			
## @item ______.xc			@tab NUMERIC VECTOR	@tab 			
## @item ______.slc			@tab NUMERIC VECTOR	@tab (NOT IN .int) 	
## @item ______.suc			@tab NUMERIC VECTOR	@tab (NOT IN .int) 	
## @item ______.slx			@tab NUMERIC VECTOR	@tab (NOT IN .int) 	
## @item ______.sux 			@tab NUMERIC VECTOR	@tab (NOT IN .int) 	
## @item ______.snx 			@tab NUMERIC VECTOR	@tab (NOT IN .int/.bas) 
## @end multitable
## 
## The result is a named list containing the response of the MOSEK Optimization 
## Library. A response code of zero is the signal of success.
##
## Depending on the specified solver, one or more solutions mays be returned. 
## The interior-point solution @var{itr}, the basic (corner point) solution 
## @var{bas}, and the integer solution @var{int}.
##
## The problem status @var{prosta} in all solutions shows the feasibility of 
## your problem definition. All solutions are described by a solution status 
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
## @multitable {.........................} {............................................} 
## @item r				@tab Result 
## @item __.response			@tab Response from the MOSEK Optimization Library 
## @item ____.code			@tab ID-code of response 
## @item ____.msg			@tab Human-readable message 
## @item __.sol				@tab All solutions identified 
## @item ____.itr/.bas/.int		@tab Solution description 
## @item ______.solsta			@tab Solution status 
## @item ______.prosta			@tab Problem status  
## @item ______.skx			@tab Variable bound keys  
## @item ______.skc			@tab Constraint bound keys  
## @item ______.xx			@tab Variable activities  
## @item ______.xc			@tab Constraint activities  
## @item ______.slc			@tab Dual variable for constraint lower bounds  
## @item ______.suc			@tab Dual variable for constraint upper bounds  
## @item ______.slx			@tab Dual variable for variable lower bounds  
## @item ______.sux 			@tab Dual variable for variable lower bounds  
## @item ______.snx 			@tab Dual variable of conic constraints 
## @end multitable
##
## @sp 1
## ========== Examples ==========
## @sp 1
## @example
## @group
## clear -v lo1;
## lo1.sense = "max";
## lo1.c = [3 1 5 1]';
## lo1.A = sparse([3 1 2 0;
##                 2 1 3 1;
##                 0 2 0 3]);
## lo1.blc = [30 15 -inf]';
## lo1.buc = [30 inf 25]';
## lo1.blx = [0 0 0 0]';
## lo1.bux = [inf 10 inf inf]';
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


%% usage: r = mosek(problem, opts)                                    
%% ------------------------------------------------------------
%% problem                  STRUCTURE                          
%% ..sense                  STRING                             
%% ..c                      COLUMN VECTOR                      
%% ..c0                     SCALAR          (OPTIONAL)         
%% ..A                      SPARSE MATRIX                      
%% ..blc                    COLUMN VECTOR                      
%% ..buc                    COLUMN VECTOR                      
%% ..blx                    COLUMN VECTOR                      
%% ..bux                    COLUMN VECTOR                      
%% ..cones                  CELL ARRAY      (OPTIONAL)         
%% ....{i}.type             STRING                             
%% ....{i}.sub              COLUMN VECTOR                      
%% ..intsub                 COLUMN VECTOR   (OPTIONAL)         
%% ..iparam/dparam/sparam   STRUCTURE       (OPTIONAL)         
%% ....<MSK_PARAM>          STRING/SCALAR   (OPTIONAL)         
%% ..sol                    STRUCTURE       (OPTIONAL)         
%% ....itr/bas/int          STRUCTURE       (OPTIONAL)         
%% opts                     STRUCTURE       (OPTIONAL)         
%% ..verbose                SCALAR          (OPTIONAL)         
%% ..usesol                 BOOLEAN         (OPTIONAL)         
%% ..useparam               BOOLEAN         (OPTIONAL)         
%% ..writebefore            FILEPATH        (OPTIONAL)         
%% ..writeafter             FILEPATH        (OPTIONAL)         
%%                                                             
%% r                        STRUCTURE                          
%% ..sol                    STRUCTURE                          
%% ....itr/bas/int          STRUCTURE       (SOLVER DEPENDENT) 
%% ......solsta             STRING                             
%% ......prosta             STRING                             
%% ......skx                CELL ARRAY                         
%% ......skc                CELL ARRAY                         
%% ......xx                 COLUMN VECTOR                      
%% ......xc                 COLUMN VECTOR                      
%% ......slc                COLUMN VECTOR   (NOT IN int)       
%% ......suc                COLUMN VECTOR   (NOT IN int)       
%% ......slx                COLUMN VECTOR   (NOT IN int)       
%% ......sux                COLUMN VECTOR   (NOT IN int)       
%% ......snx                COLUMN VECTOR   (NOT IN int OR bas)


