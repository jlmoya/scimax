tbx_build_macros(TOOLBOX_NAME, get_absolute_file_path('buildmacros.sce'));
cd 'percent'
exec ('buildpercent.sce', -1);
cd '../maxfun'
exec ('buildmaxfun.sce', -1);
cd '..'
clear tbx_build_macros;
