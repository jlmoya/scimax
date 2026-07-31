function b=Return(a)

if argn(2)~=1 then error(42), end
b=maxevalf('return',a)

endfunction
