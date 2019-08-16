#!/bin/sh


echo "Running tests"

pass=0
fail=0

# runs a test on a specific backend.
# params: llvm, filename, exitstatus
runtest(){
    if [ $1 -ne 0 ]
    then
        ../irec -bllvm "$2.ire" > /dev/null
    else
        ../irec -basm "$2.ire" > /dev/null
    fi
    # check if compilation succeeded
    if [ "$?" -ne 0 ]
    then
        echo Could not compile $2
        echo Failed
        exit
    fi
    "./$2"
    if [ "$?" -eq "$3" ]
    then
        pass=$((pass+1))
        echo -n "."
    else
        fail=$((fail+1))
        echo -n "E"
    fi
}

# runs a test with both backends
runtestboth(){
    # run with asm backend
    runtest 0 $1 $2
    # run with llvm backend
    runtest 1 $1 $2
}

# same as runtest except with output as param 4
runtestoutput(){
    if [ $1 -ne 0 ]
    then
        ../irec -bllvm "$2.ire" > /dev/null
    else
        ../irec -basm "$2.ire" > /dev/null
    fi
    # check if compilation succeeded
    if [ "$?" -ne 0 ]
    then
        echo Could not compile $2
        echo Failed
        exit
    fi
    OUTPUT=$("./$2")
    if [ "$?" -eq "$3" ] && [ "$OUTPUT" = "$4" ]
    then
        pass=$((pass+1))
        echo -n "."
    else
        fail=$((fail+1))
        echo -n "E"
    fi
}

# see runtestboth
runtestoutputboth(){
    runtestoutput 0 $1 $2 "$3"
    runtestoutput 1 $1 $2 "$3"
}

runtestboth exit 33
runtestboth arrays 10
runtestboth arith 3
runtestoutputboth hello 0 "Hello, World!"
runtestoutputboth types 5 "North America"
runtestoutputboth primes 0 "prime prime prime prime prime "
runtestboth factorial 120
runtestboth fibonacci 201

echo ""
# end part

if [ "$fail" -eq 0 ]
then
echo All tests passed.
else
echo $fail tests failed.
fi

exit
