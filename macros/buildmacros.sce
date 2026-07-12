here = get_absolute_file_path('buildmacros.sce');
cd(here);
tbx_build_macros(TOOLBOX_NAME, here);
cd 'percent'
exec ('buildpercent.sce', -1);
cd '..'
// macOS/2027 port (Task 12): buildmaxfun.sce generates macros/maxfun/genmaxfun/
// (one convenience macro per known Maxima function, e.g. expand/factor/solve)
// by calling createmaxfun() ~1500 times; createmaxfun.sci calls code2str(),
// removed from Scilab core (obsoleted in favor of ascii(), then deleted --
// see commit bb8eae0f5a5). Even fixed, the generated tree's load path
// (etc/SciMax.start) falls through to macros/maxfun/personnalizedOverload.sce,
// which calls the Overload Toolbox's own overload() function -- also absent
// here (see builder.sce). Both are pure sugar over maxevalf()/maxevalfl(),
// which this port's gateway registers unconditionally regardless; skip
// generating them rather than chasing two more missing dependencies.
// etc/SciMax.start already loads macros/maxfun/genmaxfun/ conditionally, so
// leaving it ungenerated is a clean, documented gap (see
// docs/design/toolbox-verification.md), not a broken reference.
// cd 'maxfun'
// exec ('buildmaxfun.sce', -1);
// cd '..'
clear tbx_build_macros;
