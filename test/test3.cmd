	# PROCESS BEGUN
	# argv[0] = 'ded'
	# argv[1] = '-t/tmp'
	# argv[2] = '-ctest3.cmd'
	# argv[3] = '-ltest3.out'
1cf	# before ""
\tjunk_file\n
	# after  "junk_file"
1p444p	# chmod 444 junk_file
1!rm\s-f\s#\n
	# execute rm -f junk_file
	# ELAPSED TIME
1l
1R
1st
1+	# "Makefile"
1_
1H
1cd	# before ""
\tjunk_dir\n
	# after  "junk_dir"
1cd	# before ""
\tjunk_dir/junk\n
	# after  "junk_dir/junk"
1D	# path: CM_TOOLS/test
1\n	# path: CM_TOOLS/test/junk_dir
1\n	# path: CM_TOOLS/test/junk_dir/junk
1k	# path: CM_TOOLS/test/junk_dir
1k	# path: CM_TOOLS/test
1D	# chdir CM_TOOLS/test
1!rmdir\s#\n
	# execute rmdir junk_dir/junk
	# ELAPSED TIME
1j
1.	# execute rmdir junk_dir
	# ELAPSED TIME
1R
1+	# "Makefile"
1_
1H
1cl	# before ""
q
	# after  ""
1cd	# before ""
\t\tq
	# after  ""
1cl	# before ""
\tjunk_link\n
	# after  "junk_link"
1<	# before "."
\f\t\b../#\n
	# after  "../#"
	# relink "../junk_link" (link=junk_link)
1=	# before "junk_link"
\F\t\b\b\b\b\b\b\b\b\b\btest\n
	# after  "test"
	# rename "test" (name=junk_link)
1<	# before "../junk_link"
\F\b\b\b\b\b\b\b\F\t\b\b\b\b\b\b\b\b\btest\n
	# after  "../test"
	# relink "../test" (link=test)
1@
1\s
1%ls\s-l\s#\n
	# execute ls -l test
\n	# Hit <RETURN> to continue
	# ELAPSED TIME
1:\F\t\L\L\td\F/*\n
	# execute ls -ld test/*
\n	# Hit <RETURN> to continue
	# ELAPSED TIME
1!rm\s#\n
	# execute rm test
	# ELAPSED TIME
1l
1R
1q	# PROCESS ENDED
