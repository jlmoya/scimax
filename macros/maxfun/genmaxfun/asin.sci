function b=asin(a)

if argn(2)~=1 then error(42), end
b=maxevalf('asin',a)

endfunction
