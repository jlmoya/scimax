mode(-1);
function __ckpt2(s), fd=mopen('/tmp/scimax_build_checkpoint.log','a'); mputl(s,fd); mclose(fd); endfunction
__ckpt2('bsrc-a start');
here = get_absolute_file_path('builder_src.sce');
cd(here);
printf('Entering in C directory ...\n');
__ckpt2('bsrc-b entering C dir');
cd 'c';
exec('builder_c.sce');
__ckpt2('bsrc-c C build done');
cd '../lisp';
printf('Entering in Lisp directory ...\n');
__ckpt2('bsrc-d entering lisp dir');
exec('builder_lisp.sce');
__ckpt2('bsrc-e lisp build done');
cd '..';
__ckpt2('bsrc-f builder_src done');