#/bin/sh

python zippey.py e < zippey_test.FCStd > zippey_test.plain
python zippey.py d < zippey_test.plain > zippey_test_new.FCStd

[ -e "tmp" ] && rm -rf tmp
mkdir -p tmp/zippey_test
mkdir -p tmp/zippey_test_new

unzip -q zippey_test.FCStd -d tmp/zippey_test
unzip -q zippey_test_new.FCStd -d tmp/zippey_test_new

rm -rf zippey_test.plain
rm -rf zippey_test_new.FCStd

diff -rq tmp/zippey_test tmp/zippey_test_new


