% FUN_ZTAU     dummy function
%   Such function is used by HTSTAT to determine the solution of
%   the system of two equations, as described in HTSTAT.

%   Author: 
%   Andrea Zanella, Ph.D.
%   email: andrea.zanella@cnit.it
%
%   Last Revision: Jan 27, 2003
%
%   Modified to comply with retry limits by: Luis Sanabria-Russo
%   email: luis.sanabria@upf.edu
%
%   This new versions adds the meaning of mr = maximum retries.
%   For our testings, mr > m.
function  [z]=fun_ztau_w_retry(tau,n,W,m,mr);
    p   = 1-(1-tau).^(n-1);
    z   = tau - 2*(1-2*p).*(1-(p).^(mr+1))./( W.*(1-(2*p).^(m+1)).*(1-p) + (1-2*p).*(1-(p).^(mr+1))+ W.* (2).^(m).*(p).^(m+1).*(1-2*p).*(1-(p).^(mr-m)) );
