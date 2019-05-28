echo "Running tests"

pass=0
fail=0

./irec -basm exit.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile exit.ire
    echo Failed
    exit
fi

./irec -basm arrays.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile arrays.ire
    echo Failed
    exit
fi

./irec -basm hello.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile hello.ire
    echo Failed
    exit
fi


./exit
if [ "$?" -eq 33 ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi

./arrays
if [ "$?" -eq 10 ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi

OUTPUT=$(./hello)
if [ "$OUTPUT" = $'Hello, World!' ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi



# 
# llvm backend
#

./irec -bllvm exit.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile exit.ire
    echo Failed
    exit
fi

./irec -bllvm arrays.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile arrays.ire
    echo Failed
    exit
fi

./irec -bllvm hello.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile hello.ire
    echo Failed
    exit
fi

./exit
if [ "$?" -eq 33 ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi

./arrays
if [ "$?" -eq 10 ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi

OUTPUT=$(./hello)
if [ "$OUTPUT" = $'Hello, World!' ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi


echo ""
# end part


if [ "$fail" -eq 0 ]
then
echo All tests passed.
else
echo $fail tests failed.
fi
