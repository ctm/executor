all: splash.8bp.ppm

splash.8bp.ppm:	splash.8bp.tiff
	tifftopnm < splash.8bp.tiff |  ppmquant 256 | pnmnoraw > splash.8bp.ppm

