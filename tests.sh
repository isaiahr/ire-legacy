
pass=0
fail=0


./irec exit.ire

if [ "$?" -ne 0 ]
then
    echo Could not compile exit.ire
    echo Failed
    exit
fi

./irec arrays.ire

if [ "$?" -ne 0 ]
then
    echo Could not compile arrays.ire
    echo Failed
    exit
fi

./exit
if [ "$?" -eq 33 ]
then
pass=$((pass+1))
else
fail=$((fail+1))
fi

./arrays
if [ "$?" -eq 10 ]
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