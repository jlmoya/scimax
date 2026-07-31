function b=acos(a)

if argn(2)~=1 then error(42), end
b=maxevalf('acos',a)

endfunction
