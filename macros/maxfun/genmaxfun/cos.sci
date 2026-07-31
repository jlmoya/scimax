function b=cos(a)

if argn(2)~=1 then error(42), end
b=maxevalf('__cos',a)

endfunction
