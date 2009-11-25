mode(-1);
printf('Entering in C directory ...\n');
cd 'c';
exec('builder_c.sce');
cd '../lisp';
printf('Entering in Lisp directory ...\n');
exec('builder_lisp.sce');
cd '..';