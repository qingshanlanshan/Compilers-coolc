gmake cgen
gmake dotest

echo
echo "Running the produced code"
echo

/root/Compilers-coolc/bin/spim -file example.s