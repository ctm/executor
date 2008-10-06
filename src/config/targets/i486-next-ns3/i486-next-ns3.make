LOWGLOBALS_LD_OPTION = lowglobals.o
                       
TARGET_LIBS = -lIndexing_s -lkernload -lMedia_s -lNeXT_s

TARGET_POST_LD_CMD = $(host_obj_dir)/set_page_zero_size
TARGET_POST_LD_OPTIONS = executor 200000

$(host_obj_dir)/set_page_zero_size: set_page_zero_size.c
	$(HOST_GCC) -o $(host_obj_dir)/set_page_zero_size $<
