# Scans through all header files
# If the header file uses any of the trap/entrypoint macros, makes sure it includes api-module.h
# Creates template instantiation C++ files for all such header files

open(CMAKE, ">trap_instances/trap_instances.cmake");
print CMAKE "set(trap_instance_sources\n";

while($header = <include/*.h include/rsys/*.h>) {
    $hadtrap = 0;
    $i = 0;
    $lastincludeline = 1;
    $hasmodulename = 0;
    open(HEADER, $header);
    while($line = <HEADER>) {
        $i++;
        $lastincludeline = $i if($line =~ /^#include/);
        $hasmodulename = 1 if($line =~ /^#define MODULE_NAME/);
        $hadtrap = 1 if($line =~ /^(PASCAL|REGISTER|RAW_68K)_TRAP/ || $line =~ /^(PASCAL|REGISTER)_SUBTRAP/ || $line =~ /^(NOTRAP_FUNCTION|PASCAL_FUNCTION)/);
    }
    close(HEADER);

    if($hadtrap) {

        if($header =~ /^include\/rsys\/(.*)\.h$/) {
            $modname = "rsys_$1";
            $headername = "rsys/$1.h";
        } elsif($header =~ /^include\/(.*)\.h$/) {
            $modname = $1;
            $headername = "$1.h";
        }

        open(HEADER, $header);
        open(OUT, ">temp.h");
        $i = 0;
        while($l = <HEADER>) {
            $i++;

            print OUT $l;
            if($i == $lastincludeline && !$hasmodulename) {
                print($header, "\n");

                print OUT "\n";
                print OUT "#define MODULE_NAME ", $modname, "\n";
                print OUT "#include <rsys/api-module.h>\n"
            }
        }
        close(OUT);
        close(HEADER);

        open(OUT, ">trap_instances/$modname.cpp");
        print OUT "#define INSTANTIATE_TRAPS_$modname\n";
        print OUT "#include <$headername>";        
        close(OUT);
        print CMAKE "\t\ttrap_instances/$modname.cpp\n";
        
        system "mv temp.h $header";
    }
}
print CMAKE "\t)\n";
close(CMAKE);
