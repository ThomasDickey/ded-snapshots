	# PROCESS BEGUN
	# argv[0] = 'ded'
	# argv[1] = '-t/tmp'
	# argv[2] = '-ctest5.cmd'
	# argv[3] = '-ltest5.out'
1cd	# before ""
\tjunk\n
	# after  "junk"
1&
1E	# chdir CM_TOOLS/test/junk
1cf	# before ""
\ttest1\n
	# after  "test1"
1'f	# before ""
2\n
	# after  "test12"
1'f	# before ""
3\n
	# after  "test123"
1"	# before ""
	# after  "test123"
	# (dedmsg) File exists
	# before ""
4\n
	# after  "test1234"
1+	# "test1234"
1+	# "test123"
1+	# "test12"
1!rm\s-f\s#\n
	# execute rm -f test1234 test123 test12 test1
	# ELAPSED TIME
1_
1W
1"	# before ""
	# after  "test1234"
1'f	# before ""
\U\U\U\n
	# after  "test1"
1'f	# before ""
\U\U\U\n
	# after  "test12"
1'f	# before ""
\U\U\U\n
	# after  "test123"
1sn
1R
1k
1k
1sp
1#	# "test1"
	# "test12"
	# "test123"
	# "test1234"
1.	# execute rm -f test1 test12 test123 test1234
	# ELAPSED TIME
1R
1'f	# before ""
\n
	# after  "test123"
1'f	# before ""
\U\n
	# after  "test12"
1'f	# before ""
\U\U\n
	# after  "test1"
1'f	# before ""
\U\U\U\n
	# after  "test1234"
1sn
1#	# "test1234"
1sp
1#	# "test1234"
	# "test1"
	# "test12"
	# "test123"
	# "test1234"
1=	# before "test1234"
\F\t.bak\n
	# after  "test1234.bak"
	# rename "test1234.bak" (name=test1234)
	# before "test1"
	# after  "test1.bak"
	# rename "test1.bak" (name=test1)
	# before "test12"
	# after  "test12.bak"
	# rename "test12.bak" (name=test12)
	# before "test123"
	# after  "test123.bak"
	# rename "test123.bak" (name=test123)
1=	# before "test1234.bak"
\F\t\b\b\b\b\n
	# after  "test1234"
	# rename "test1234" (name=test1234.bak)
	# before "test1.bak"
	# after  "test1"
	# rename "test1" (name=test1.bak)
	# before "test12.bak"
	# after  "test12"
	# rename "test12" (name=test12.bak)
	# before "test123.bak"
	# after  "test123"
	# rename "test123" (name=test123.bak)
1_
1k
1k
1k
1'=	# before "test1"
\U\n
	# after  "test1.bak"
	# rename "test1.bak" (name=test1)
1j
1"	# before "test12"
	# after  "test12.bak"
	# rename "test12.bak" (name=test12)
1j
1"	# before "test123"
	# after  "test123.bak"
	# rename "test123.bak" (name=test123)
1j
1"	# before "test1234"
	# after  "test1234.bak"
	# rename "test1234.bak" (name=test1234)
1#	# "test1234.bak"
	# "test1.bak"
	# "test12.bak"
	# "test123.bak"
	# "test1234.bak"
1p444p	# chmod 444 test1.bak
	# chmod 444 test12.bak
	# chmod 444 test123.bak
	# chmod 444 test1234.bak
1p6p	# chmod 644 test1.bak
	# chmod 644 test12.bak
	# chmod 644 test123.bak
	# chmod 644 test1234.bak
1p7p	# chmod 744 test1.bak
	# chmod 744 test12.bak
	# chmod 744 test123.bak
	# chmod 744 test1234.bak
1p\s77p	# chmod 777 test1.bak
	# chmod 777 test12.bak
	# chmod 777 test123.bak
	# chmod 777 test1234.bak
1'p\U\U\U\n	# chmod 444 test1.bak
	# chmod 444 test12.bak
	# chmod 444 test123.bak
	# chmod 444 test1234.bak
1'p\U\U\n	# chmod 744 test1.bak
	# chmod 744 test12.bak
	# chmod 744 test123.bak
	# chmod 744 test1234.bak
1.	# execute rm -f test1.bak test12.bak test123.bak test1234.bak
	# ELAPSED TIME
1qn\n
1.	# execute rm -f test1.bak test12.bak test123.bak test1234.bak
	# ELAPSED TIME
1B	# chdir CM_TOOLS/test
1!rmdir\s#\n
	# execute rmdir junk
	# ELAPSED TIME
1qy\n
	# PROCESS ENDED
