#!/bin/sh

. ../_functions.sh


announce "config.c:config_ensure_directory() - not a directory"
touch test.file
if ./stub config_ensure_directory test.file 2> test.stderr; then
	fail
fi
rm -f test.file
pass


announce "config.c:config_ensure_directory() - create"
rm -rf test.dir
if ! ./stub config_ensure_directory test.dir; then
	fail "process failed"
fi
if [ ! -d test.dir ]; then
	fail "directory does not exist"
fi
rmdir test.dir
pass


# can't test, no way to do that without being root
announce "config.c:config_ensure_directory() - bad ownership"
skip


announce "config.c:config_ensure_directory() - permission 777"
mkdir -p test.dir
chmod 777 test.dir
if ./stub config_ensure_directory test.dir 2> test.stderr; then
	fail
fi
rmdir test.dir
pass


announce "config.c:config_ensure_directory() - permission 755"
mkdir -p test.dir
chmod 755 test.dir
if ./stub config_ensure_directory test.dir 2> test.stderr; then
	fail
fi
rmdir test.dir
pass


announce "config.c:config_ensure_directory() - permission 700"
mkdir -p test.dir
chmod 700 test.dir
if ! ./stub config_ensure_directory test.dir; then
	fail
fi
rmdir test.dir
pass

exit 0
