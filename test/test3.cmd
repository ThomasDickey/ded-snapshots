	# PROCESS BEGUN
	# argv[0] = 'ded'
	# argv[1] = '-t/tmp'
	# argv[2] = '-ctest3.cmd'
	# argv[3] = '-ltest3.out'
1cf	# before ""
\tjunk_file\r
	# after  "junk_file"
1p444p	# chmod 444 junk_file
1!rm -f #
	# execute rm -f junk_file
	# ELAPSED TIME
1l
1R
1st
1+	# "Makefile"
1_
1H
1cd	# before ""
\tjunk_dir\r
	# after  "junk_dir"
1cd	# before ""
\tjunk_dir/junk\r
	# after  "junk_dir/junk"
1D	# path: CM_TOOLS/test
1\r	# path: CM_TOOLS/test/junk_dir
1\r	# path: CM_TOOLS/test/junk_dir/junk
1k	# path: CM_TOOLS/test/junk_dir
1k	# path: CM_TOOLS/test
1D	# chdir CM_TOOLS/test
1!rmdir #
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
\tjunk_link\r
	# after  "junk_link"
1<	# before "."
\f\t\b../#\r
	# after  "../#"
	# relink "../junk_link" (link=junk_link)
1=	# before "junk_link"
\006\t\b\b\b\b\b\b\b\b\b\btest\r
	# after  "test"
	# rename "test" (name=junk_link)
1<	# before "../junk_link"
\006\b\b\b\b\b\b\b\006\t\b\b\b\b\b\b\b\b\btest\r
	# after  "../test"
	# relink "../test" (link=test)
1@
1\s
1%ls -l #
	# execute ls -l test
\r	# Hit <RETURN> to continue
	# ELAPSED TIME
1:ls -ld #/*
	# execute ls -ld test/*
\r	# Hit <RETURN> to continue
	# ELAPSED TIME
1!rm #
	# execute rm test
	# ELAPSED TIME
1l
1R
1q	# PROCESS ENDED
