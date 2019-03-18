
pass=0
fail=0


./irec exit.ire
./exit
if [ "$?" -eq 33 ]
then
pass=$((pass+1))
else
fail=$((fail+1))
fi


if [ "$fail" -eq 0 ]
then
echo All tests passed.
else
echo $fail tests failed.
fi
