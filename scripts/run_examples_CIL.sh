#!/bin/bash

clear

CANALYZER_HOME=`pwd`

OUTPUT=$CANALYZER_HOME/output/cil

echo "example test_break_in_nested_loop_inner.c"
$CANALYZER_HOME/scripts/myscript.sh testdata/test_break_in_nested_loop_inner.c
echo ""

echo "example itest_break_in_nested_loop_inner_then.c"
$CANALYZER_HOME/scripts/myscript.sh testdata/test_break_in_nested_loop_inner_then.c
echo ""

echo "example test_break_in_nested_loop_inner_else.c"
$CANALYZER_HOME/scripts/myscript.sh testdata/test_break_in_nested_loop_inner_else.c
echo ""

echo "example test_nested_loops_three_level.c"
$CANALYZER_HOME/scripts/myscript.sh testdata/test_nested_loops_three_level.c
echo ""

echo "example test_three_loops_break_in_then_3.c"
$CANALYZER_HOME/scripts/myscript.sh testdata/test_three_loops_break_in_then_3.c
echo ""

#echo "example test_three_loops_break_in_then_else.c"
#$CANALYZER_HOME/scripts/myscript.sh testdata/test_three_loops_break_in_then_else.c
#echo ""

#echo "example test_break_while.c"
#$CANALYZER_HOME/scripts/myscript.sh testdata/test_break_while.c
#echo ""

echo "example test_break_in_while.c"
$CANALYZER_HOME/scripts/myscript.sh testdata/test_break_in_while.c 
echo ""

echo "example test_case_statement.c"
$CANALYZER_HOME/scripts/myscript.sh testdata/test_case_statement.c 
echo ""

echo "example test_conditional_op.c"
$CANALYZER_HOME/scripts/myscript.sh testdata/test_conditional_op.c
echo ""

echo "example test_incr_new.c"
$CANALYZER_HOME/scripts/myscript.sh testdata/test_incr_new.c
echo ""

echo "example test_uo_logicalnot.c"
$CANALYZER_HOME/scripts/myscript.sh testdata/test_uo_logicalnot.c
echo ""

rm a.out

echo -e "\n[Done]\n"

echo -e "\nErrors/Warnings:\n"
grep -i "error" -r $OUTPUT/; grep -i "warning" -r $OUTPUT/; grep -i "stack" -r $OUTPUT/; grep -i "segmentation" -r $OUTPUT/; grep -i "Aborted" -r $OUTPUT/

