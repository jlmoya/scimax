mode(-1);
maxima_path=getenv('MAXIMA_EXE_PATH');
// macOS/2027 port (Task 12): unix_g() is now a deprecated wrapper around
// host() (warnobsolete since 2027.0.0); call host() directly. host()
// returns [status, stdout, stderr] -- unix_g() used to hand back stdout
// alone, so keep only that piece here.
[%sm_lisp_status, output] = host(maxima_path + ' -q --batch-string "":lisp (load \""make.lisp\"")""');

global %lisp_error;

%lisp_error=%F;

if (output($) == 'ERROR') then
  %lisp_error=%T;
end

for i=output',
  if (i <> '') then printf("%s\n",i), end;
end;

clear output