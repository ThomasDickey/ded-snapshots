	# PROCESS BEGUN
	# argv[0] = 'ded'
	# argv[1] = '-t/tmp'
	# argv[2] = '-ctest4.cmd'
	# argv[3] = '-ltest4.out'
1%rm -rf junk*
	# execute rm -rf junk*
\r	# Hit <RETURN> to continue
	# ELAPSED TIME
1cd	# before ""
\tjunk\r
	# after  "junk"
1cd	# before ""
\tjunk/first\r
	# after  "junk/first"
1cd	# before ""
\tjunk/second\r
	# after  "junk/second"
1%touch #/file
	# execute touch junk/second/file
\r	# Hit <RETURN> to continue
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
\tjunk/second/file2\r
	# after  "junk/second/file2"
1cL	# before "junk/second/file2"
\R\R\R\R\R\R\R\R\R\R\R\R\R\L\L\t\b\b\b\b\b\bfirst\r
	# after  "junk/first/file2"
1cL	# before "junk/first/file2"
\006\b\b\b\b\b\006\t\b\b\b\b\b\b\b\b\b\b\bfile3\r
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
1\r	# path: CM_TOOLS/test/junk
1E	# chdir CM_TOOLS/test/junk
1L
1E	# chdir CM_TOOLS/test/junk/first
1B	# chdir CM_TOOLS/test/junk
1k
1E	# chdir CM_TOOLS/test/junk/second
1D	# path: CM_TOOLS/test/junk/second
1N	# path: CM_TOOLS/test/junk/second
1B	# path: CM_TOOLS/test/junk/first
1B	# path: CM_TOOLS/test/junk
1B	# path: CM_TOOLS/test
1B	# path: CM_TOOLS/test/junk/second
1B	# path: CM_TOOLS/test/junk/first
1=the_first
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
1%rm -rf junk
	# execute rm -rf junk
\r	# Hit <RETURN> to continue
	# ELAPSED TIME
1qy
	# PROCESS ENDED
