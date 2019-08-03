# TODO: restructure this file to group these test cases into functs


echo "Running tests"

pass=0
fail=0


../irec -basm exit.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile exit.ire
    echo Failed
    exit
fi

../irec -basm arrays.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile arrays.ire
    echo Failed
    exit
fi

../irec -basm hello.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile hello.ire
    echo Failed
    exit
fi

../irec -basm arith.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile arith.ire
    echo Failed
    exit
fi

../irec -basm types.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile types.ire
    echo Failed
    exit
fi

../irec -basm factorial.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile factorial.ire
    echo Failed
    exit
fi

../irec -basm fibonacci.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile fibonacci.ire
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

./arith
if [ "$?" -eq 3 ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi

OUTPUT=$(./types)
if [ "$?" -eq 5 ] && [ "$OUTPUT" = $'North America' ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi

./factorial
if [ "$?" -eq 120 ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi

./fibonacci
if [ "$?" -eq 201 ]
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

../irec -bllvm exit.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile exit.ire
    echo Failed
    exit
fi

../irec -bllvm arrays.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile arrays.ire
    echo Failed
    exit
fi

../irec -bllvm hello.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile hello.ire
    echo Failed
    exit
fi

../irec -bllvm arith.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile arith.ire
    echo Failed
    exit
fi

../irec -bllvm types.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile types.ire
    echo Failed
    exit
fi

../irec -bllvm factorial.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile factorial.ire
    echo Failed
    exit
fi

../irec -bllvm fibonacci.ire > /dev/null

if [ "$?" -ne 0 ]
then
    echo Could not compile fibonacci.ire
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

./arith
if [ "$?" -eq 3 ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi

OUTPUT=$(./types)
if [ "$?" -eq 5 ] && [ "$OUTPUT" = $'North America' ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi

./factorial
if [ "$?" -eq 120 ]
then
pass=$((pass+1))
echo -n "."
else
fail=$((fail+1))
echo -n "E"
fi

./fibonacci
if [ "$?" -eq 201 ]
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
