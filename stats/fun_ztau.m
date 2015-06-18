% FUN_ZTAU     dummy function
%   Such function is used by HTSTAT to determine the solution of
%   the system of two equations, as described in HTSTAT.

%   Author: 
%   Andrea Zanella, Ph.D.
%   email: andrea.zanella@cnit.it
%
%   Last Revision: Jan 27, 2003
function  [z]=fun_ztau(tau,n,W,m);
    p   = 1-(1-tau).^(n-1);                                     
    z   = tau - 2*(1-2*p)./( (1-2*p).*(W+1) +p.*W.*(1- (2*p).^m ) );    
