// This file contains Maxima names and the corresponding Scilab names
// You should see the file personnalizedOverload.sce

pNames=[['__det','det'];
        ['__sinm','sinm'];
        ['__cosm','cosm'];
	['__tanhm','tanhm'];
	['__sinhm','sinhm'];
        ['__coshm','coshm'];
	['__tanhm','tanhm'];
	['__expm','expm'];
	['__logm','logm'];
	['__sin','sin'];
        ['__cos','cos'];
	['__tanh','tanh'];
	['__sinh','sinh'];
        ['__cosh','cosh'];
	['__tanh','tanh'];
	['__exp','exp'];
	['__log','log'];
	['num','Num'];
        ['rhs','Rhs'];
        ['lhs','Lhs'];
	['status','Status'];
	['newline','Newline'];
	['load','Load'];
        // Maxima functions whose names are Scilab KEYWORDS. Without these the
        // generator emits e.g. "function a=break(varargin)", which cannot parse,
        // and genlib aborts the whole genmaxfun build partway through — which is
        // the second reason genmaxfun had never been built. Same capitalisation
        // convention as the entries above, which avoid clashes with Scilab
        // FUNCTIONS rather than keywords.
	['if','If'];
	['for','For'];
	['while','While'];
	['do','Do'];
	['break','Break'];
	['return','Return'];
	['catch','Catch'];
	['quit','Quit']]
	