level = 0
raw = true

while line = gets
    if raw or line =~ /struct PACKED/ then
        if not raw and line =~ /;/ then
            #$stderr.puts(line);
            $stdout.puts(line);
        else
            head = nil
            foot = nil
            names = nil
            if raw then
                lines = [line]
                while line = gets
                    lines << line
                end
            else
                head = line
                level = head.count('{')
                if level == 0 then
                    line2 = gets
                    head += line2
                    level = line2.count('{')
                end

                lines = []
                while level != 0 && line = gets
                    level += line.count('{') - line.count('}')
                    if level != 0 then
                        lines << line
                    end
                end

                foot = line

                names = []
                if head =~ /struct\s+PACKED\s+([_0-9A-Za-z]+)/ then
                    names << $1
                end
                
                if head =~ /typedef/ then
                    if foot =~ /\}\s*([^;]*);/ then
                        names.concat($1.split(/,\s*/))
                    elsif foot =~ /^\s*\}\s*$/ then
                        foot += gets
                        if foot =~ /\}\s*([^;]*);/ then
                            names.concat($1.split(/,\s*/))
                        else
                            $stderr.puts("FOOT PROBLEMS: " + foot)
                        end                        
                    else
                    $stderr.puts("FOOT PROBLEMS: " + foot)
                    end
                end
                
                
                name = nil
                if names[0] =~ /^([_0-9A-Za-z])+$/ then
                    name = names.shift
                    
                    if names[0] == name then
                        names.shift
                    end
                end
            end

            ok = true

            converted = []
            offendingLine = nil

            startedComment = ""
            lines.each do |thisLine|
                l = startedComment + thisLine
                l.sub!(/PACKED_MEMBER\(\s*([^,]+)\s*,\s*([^)]+)\s*\)/, '\1 \2')
                comments = ""

                l.gsub!( /((?:\/\*(?:[^*]|(?:\*+[^*\/]))*\*+\/)|(?:\/\/.*))/ ) do |c|
                    comments += c
                    ""
                end
                if comments != ""
                    comments = "    " + comments
                end
                if l =~ /\/\*/ then
                    startedComment = startedComment + thisLine
                    next
                else
                    startedComment = ""
                end
                    

                if l=~/^\s*$/ then
                    $stdout.puts(comments)
                    next
                end

                type = "";
                varname = nil;
                hadType = false;
                
                if l =~ /^#/ then
                    converted << l
                else
                    s = l.dup
                    
                    while s != "" do
                        if s.sub!(/^\s+/,'') then
                        elsif s.sub!(/^;\s+$/,'') then
                        elsif s.sub!(/^(unsigned|signed|short|long|int|char)\b/,'') then
                            type += " " + $~.to_s
                            hadType = true
                        elsif s.sub!(/^(const|union|struct)\b/,'') then
                            type += " " + $~.to_s
                        elsif s.sub!(/^(PACKED)\b/,'') then
                        elsif s.sub!(/^[A-Za-z0-9_]+\b/,'') then
                            if hadType then
                                if varname then
                                    ok = false
                                    offendingLine = (offendingLine or thisLine)
                                    #$stderr.puts("another word after varname")
                                end
                                varname = $~.to_s
                            else
                                type += " " + $~.to_s
                                hadType = true
                            end
                        elsif s.sub!(/^\*/,'') then
                            if varname then
                                ok = false
                                offendingLine = (offendingLine or thisLine)
                                #$stderr.puts("* after varname")
                            end
                            type += "*"
                        elsif s.sub!(/^\[[^\]]*\]/,'') then
                            type += $~.to_s
                        else
                            offendingLine = (offendingLine or thisLine)
                            ok = false
                        # $stderr.puts("unexplained: #{s}")
                            break
                        end
                    end

                    if hadType and varname then
                        converted << "    GUEST<#{type}> #{varname};#{comments}"
                        
                    else
                        ok = false
                        offendingLine = (offendingLine or thisLine)
                    end
                end
            end

            if ok then

                if not raw then
                    if names.count > 0 then
                        $stdout.puts("typedef struct #{name} : GuestStruct {")
                    else
                        $stdout.puts("struct #{name} : GuestStruct {")
                    end
                end

                converted.each do |l|
                    $stdout.puts(l)
                end
                # $stderr.puts(foot)
                if not raw then
                    if names.count > 0 then
                        $stdout.puts("} " + names.join(", ") + ";")
                    else
                        $stdout.puts("};");
                    end
                end
            else
                str = ""
                str += "// ### Struct needs manual conversion to GUEST<...>\n"
                str += "// #{offendingLine}"
                
                str += head
                lines.each do |l|
                    str += l
                end
                str += foot
                $stderr.puts(str)
                $stderr.puts
                $stdout.puts(str)
            end
        end
    else
        $stdout.puts(line)
    end
    break if raw
end