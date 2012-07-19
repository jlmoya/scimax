mode(-1);
maxima_path=getenv('MAXIMA_EXE_PATH');
output=unix_g(maxima_path + ' -q --batch-string "":lisp (load \""make.lisp\"")""');

global %lisp_error;

%lisp_error=%F;

if (output($) == 'ERROR') then
  %lisp_error=%T;
end

for i=output',
  if (i <> '') then printf("%s\n",i), end;
end;

clear output