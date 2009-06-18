LOWGLOBALS_LD_OPTION = lowglobals.o
                       
HOST_LIBS = -lIndexing_s -lkernload -lMedia_s -lNeXT_s

HOST_POST_LD_CMD = $(build_obj_dir)/set_page_zero_size
HOST_POST_LD_OPTIONS = executor 200000

$(build_obj_dir)/set_page_zero_size: set_page_zero_size.c
	$(BUILD_GCC) -o $(build_obj_dir)/set_page_zero_size $<
