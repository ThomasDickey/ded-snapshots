	# PROCESS BEGUN
	# argv[0] = 'ded'
	# argv[1] = '-t/tmp'
	# argv[2] = '-ctest4.cmd'
	# argv[3] = '-ltest4.out'
1%rm\s-rf\sjunk*\n
	# execute rm -rf junk*
\n	# Hit <RETURN> to continue
	# ELAPSED TIME
1cd	# before ""
\tjunk\n
	# after  "junk"
1cd	# before ""
\tjunk/first\n
	# after  "junk/first"
1cd	# before ""
\tjunk/second\n
	# after  "junk/second"
1%touch\s#/file\n
	# execute touch junk/second/file
\n	# Hit <RETURN> to continue
	# ELAPSED TIME
1j
0.	# execute touch junk/first/file
	# ELAPSED TIME
1j
1.	# execute touch junk/file
	# ELAPSED TIME
1k
1k
1cf	# before ""
\tjunk/second/file2\n
	# after  "junk/second/file2"
1cL	# before "junk/second/file2"
\R\R\R\R\R\R\R\R\R\R\R\R\R\L\L\t\b\b\b\b\b\bfirst\n
	# after  "junk/first/file2"
1cL	# before "junk/first/file2"
\F\L\L\L\L\L\F\t\b\b\b\b\b\b\b\b\b\b\bfile3\n
	# after  "junk/file3"
1p444p	# chmod 444 junk/file3
1si
1#	# "junk/file3"
	# "junk/first/file2"
	# "junk/second/file2"
1pp
1_
1R
1D	# path: CM_TOOLS/test
1\n	# path: CM_TOOLS/test/junk
1E	# chdir CM_TOOLS/test/junk
1L
1E	# chdir CM_TOOLS/test/junk/first
1B	# chdir CM_TOOLS/test/junk
1k
1E	# chdir CM_TOOLS/test/junk/second
1D	# path: CM_TOOLS/test/junk/second
1N
\s	# (waitmsg) No previous regular expression
	# path: CM_TOOLS/test/junk/second
1B	# path: CM_TOOLS/test/junk/first
1B	# path: CM_TOOLS/test/junk
1B	# path: CM_TOOLS/test
1B	# path: CM_TOOLS/test/junk/second
1B	# path: CM_TOOLS/test/junk/first
1=\t\Bthe_\n
	# rename "CM_TOOLS/test/junk/the_first" (name=CM_TOOLS/test/junk/first)
	# RING-chdir CM_TOOLS/test/junk/the_first =>0
	# path: CM_TOOLS/test/junk/the_first
1F	# path: CM_TOOLS/test
1F	# path: CM_TOOLS/test/junk
1F	# path: CM_TOOLS/test/junk/second
1F	# path: CM_TOOLS/test/junk/the_first
1F	# path: CM_TOOLS/test
1B	# path: CM_TOOLS/test/junk/the_first
1B	# path: CM_TOOLS/test/junk/second
1Q	# path: CM_TOOLS/test/junk
1Q	# path: CM_TOOLS/test
1D	# chdir CM_TOOLS/test
1%rm\s-rf\sjunk\n
	# execute rm -rf junk
\n	# Hit <RETURN> to continue
	# ELAPSED TIME
1qy\n
	# PROCESS ENDED
