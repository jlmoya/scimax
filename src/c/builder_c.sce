mode(-1);
ici=get_absolute_file_path('builder_c.sce');
include='-I'+ici+'../include/ ';
include=include+'-I'+SCI+'/../../include/scilab/shell/';


cfuns=['sci_answer.c','sci_latex.c','sci_Matrix.c','sci_maxevalfl.c','sci_maxinit.c','sci_maxprint.c','sci_sym.c','sci_Syms.c','sci_defmf.c','sci_mathml.c','sci_maxevalf.c','sci_maxevalop.c','sci_maxkill.c','sci_noanswer.c','sci_symnp.c','donnees.c','latex.c','mathml.c','maxevalf.c','maxevalop.c','maxkill.c','symnp.c','defmf.c','gestionVar.c','Matrix.c','maxevalfl.c','maxinit.c','maxprint.c','sym.c'];

if getos()=="Windows" then
    cfuns($+1)='spawnpipe.c';
end

functions=['maxinit' 'sci_maxinit';
	   'maxkill' 'sci_maxkill';
	   'maxevalf' 'sci_maxevalf';
	   'maxevalfl' 'sci_maxevalfl';
	   '%sym_p' 'sci_maxprint';
	   'sym' 'sci_sym';
	   'Matrix' 'sci_Matrix';
	   'symnp' 'sci_symnp';
	   'defmf' 'sci_defmf';
	   'Syms' 'sci_Syms';
	   'latex' 'sci_latex';
	   'mathml' 'sci_mathml';
	   'answer' 'sci_answer';
	   'noanswer' 'sci_noanswer';
	   'maxevalopA' 'sci_maxevalop';
	   'maxevalopB' 'sci_maxevalop';
	   'maxevalopC' 'sci_maxevalop';
	   'maxevalopD' 'sci_maxevalop';
	   'maxevalopE' 'sci_maxevalop';
	   'maxevalopF' 'sci_maxevalop';
	   'maxevalopG' 'sci_maxevalop';
	   'maxevalopH' 'sci_maxevalop';
	   'maxevalopI' 'sci_maxevalop';
	   'maxevalopJ' 'sci_maxevalop';
	   'maxevalopK' 'sci_maxevalop';
	   'maxevalopL' 'sci_maxevalop';
	   'maxevalopM' 'sci_maxevalop';
	   'maxevalopN' 'sci_maxevalop';
	   'maxevalopO' 'sci_maxevalop';
	   'maxevalopP' 'sci_maxevalop';
	   'maxevalopQ' 'sci_maxevalop';
	   'maxevalopR' 'sci_maxevalop';
	   'maxevalopS' 'sci_maxevalop'];

if getos() == 'Windows' then
  ldflags =SCI + '/bin/core.lib '+ SCI + '/bin/sciconsole.lib ' + SCI + '/bin/string.lib ' + SCI + '/bin/io.lib';
else
  ldflags = '';
end

// macOS/2027 port (Task 12): every gateway file here keeps its original
// K&R-style declarations (no prototypes) -- clang 17+ flags that as
// -Wdeprecated-non-prototype on ~20 files, and the resulting flood of
// warning text appears to overrun a fixed-size buffer somewhere in
// ilib_compile's output relay (matches the project's known scivprint()
// unbounded-buffer bug class), corrupting the session. Silence the noisy
// warning class rather than rewriting every K&R declaration to ANSI style.
ilib_build('maxima',functions,cfuns,[],'Makelib',ldflags,include+' -D__USE_DEPRECATED_STACK_FUNCTIONS__ -Wno-deprecated-non-prototype -Wno-implicit-int','',%f,'');

clear mpath ici functions cfuns include;

