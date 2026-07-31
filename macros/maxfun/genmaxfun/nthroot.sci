function c=nthroot(a,b)

if argn(2)~=2 then error(42), end
c=maxevalf('nthroot',a,b)

endfunction
