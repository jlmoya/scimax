mode(-1);
lines(0);
function __ckpt(s), fd=mopen('/tmp/scimax_build_checkpoint.log','a'); mputl(s,fd); mclose(fd); endfunction
__ckpt('00 start');
try
 getversion('scilab');
catch
 error(gettext('Scilab 5.0 or more is required.'));
end;
__ckpt('01 getversion ok');
// ====================================================================

toolbox_dir = get_absolute_file_path('builder.sce');
__ckpt('02 toolbox_dir='+toolbox_dir);

// macOS/2027 port (Task 12): the Overload Toolbox (a separate operator-
// overload helper toolbox by the same author) is not installed and this
// port does not need it -- SciMax registers its own sym-type operator
// overloads directly via newfun() in etc/SciMax.start. Degrade gracefully
// (print a note, keep going) instead of the original hard abort; a genuine
// install still gets picked up below exactly as before. etc/SciMax.start's
// matching load is likewise made optional.
// By default, %otb_path is set to '../Overload_Toolbox/'
if ~exists('%otb_path') then
   %otb_path='../Overload_Toolbox'
end

%otb_loader=%otb_path+'/'+'loader.sce';
if fileinfo(toolbox_dir+%otb_loader)~=[] then
  path=toolbox_dir+%otb_loader;
elseif fileinfo(%otb_loader)~=[] then
    path=%otb_loader;
else
  printf("Overload Toolbox not found (checked %%otb_path=''%s''); continuing "+..
	"without it -- SciMax''s own operator overloads do not need it.\n", %otb_path);
  path=toolbox_dir+%otb_loader;
end

// macOS/2027 port (Task 12): these used to be bare relative paths ("etc/...")
// -- fine only when builder.sce is run with the toolbox dir already as cwd
// (the old by-hand usage). tbx_build.sci execs this script via errcatch
// from whatever cwd the harness happens to be in, so a relative path here
// wrote into the SCILAB TREE's own etc/, not the toolbox's -- write
// absolute paths instead.
fd=mopen(toolbox_dir+'etc/Overload_TB_path.sce','w');
mputl('Overload_TB_path='''+path+'''',fd);
mclose(fd);
__ckpt('03 otb path written: '+path);


// macOS/2027 port (Task 12): resolve maxima via PATH (matches how
// src/c/maxinit.c actually launches it: execlp("maxima", ...)) instead of
// requiring the caller to pre-set %maxima_exe_path by hand.
if ~exists('%maxima_exe_path') then
  [st, which_out] = host('which maxima');
  if st == 0 & which_out(1) <> '' then
    %maxima_exe_path = which_out(1);
  end
end
__ckpt('04 maxima_exe_path resolve done, exists='+string(exists('%maxima_exe_path')));

if exists('%maxima_exe_path') then
  fd=mopen(toolbox_dir+'etc/MAXIMA_EXE_PATH.sce','w');
  mputl('%maxima_exe_path='''+%maxima_exe_path+'''',fd);
  mclose(fd);
  setenv('MAXIMA_EXE_PATH',%maxima_exe_path)
else
  printf("I need to know full path of maxima execution.\nPleases "+..
	"set the variable MAXIMA_EXE_PATH=''path of maxima execution ''\n(maybe ''C:/Maima/bin/maxima.bat'') and"+...
	 " re-exec builder.sce.\n");
  __ckpt('04b ABORTING: no maxima_exe_path');
  abort;
end
__ckpt('05 maxima_exe_path='+%maxima_exe_path);

// ====================================================================

// ====================================================================
if ~with_module('development_tools') then
  error(msprintf(gettext('%s module not installed.'),'development_tools'));
end
__ckpt('06 development_tools ok');
// ====================================================================
TOOLBOX_NAME = 'SciMax';
TOOLBOX_TITLE = 'SciMax Toolbox';
// ====================================================================

__ckpt('07 calling tbx_builder_src');
tbx_builder_src(toolbox_dir);
__ckpt('08 tbx_builder_src returned');

global %lisp_error;

if (%lisp_error) then
  printf('Process aborted\n');
  __ckpt('08b ABORTING: lisp_error');
  abort
end
__ckpt('09 lisp ok, calling tbx_builder_macros');

tbx_builder_macros(toolbox_dir);
__ckpt('10 tbx_builder_macros returned, calling tbx_build_loader');
tbx_build_loader(TOOLBOX_NAME, toolbox_dir);
__ckpt('11 tbx_build_loader returned');
cd(toolbox_dir);
clear toolbox_dir TOOLBOX_NAME TOOLBOX_TITLE %otb_path fd %lisp_error;
__ckpt('12 builder.sce done');

// ====================================================================










