#!/bin/bash

CANALYZER_HOME=`pwd`

TESTDATA=$CANALYZER_HOME/testdata
OUTPUT=$CANALYZER_HOME/dry_run

mkdir -p $OUTPUT

echo -e "\nRunning example test cases ... \n\n"

echo "example testdata/prg1.c - assignment & operators"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/prg1.c $OUTPUT/prg1.txt
echo ""

echo "example testdata/prg2.c - if-else"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/prg2.c $OUTPUT/prg2.txt
echo ""

echo "example testdata/prg3.c - if, while"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/prg3.c $OUTPUT/prg3.txt
echo ""

echo "example testdata/continue.c - assignment, if-else, while"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/continue.c $OUTPUT/continue.txt
echo ""

#echo "example testdata/sample.c - assignment, if-else, while, functions"
#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/sample.c $OUTPUT/sample.txt
#echo ""

echo "example test-if_only_if_no_else.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-if_only_if_no_else.c $OUTPUT/test-if_only_if_no_else.txt
echo ""

echo "example test-if_only_if_and_else.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-if_only_if_and_else.c $OUTPUT/test-if_only_if_and_else.txt
echo ""

echo "example test-if_if_else_ladder.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-if_if_else_ladder.c $OUTPUT/test-if_if_else_ladder.txt
echo ""

echo "example test-if_inside_if.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-if_inside_if.c $OUTPUT/test-if_inside_if.txt
echo ""

echo "example test-if_after_if.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-if_after_if.c $OUTPUT/test-if_after_if.txt
echo ""

echo "example testdata/test_while.c - while loop"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_while.c $OUTPUT/test_while.txt
echo ""

echo "example testdata/test_for.c - for loop"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_for.c $OUTPUT/test_for.txt
echo ""

echo "example test_for_simple.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_for_simple.c $OUTPUT/test_for_simple.txt
echo ""

echo "example test_while_simple.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_while_simple.c $OUTPUT/test_while_simple.txt
echo ""

echo "example test_do_while.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_do_while.c $OUTPUT/test_do_while.txt
echo ""

echo "example test_do_while_simple.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_do_while_simple.c $OUTPUT/test_do_while_simple.txt
echo ""

echo "example test_nested_while.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_nested_while.c $OUTPUT/test_nested_while.txt
echo ""

echo "example test_if_in_do_while.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_if_in_do_while.c $OUTPUT/test_if_in_do_while.txt
echo ""

echo "example test_if_in_while_in_if.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_if_in_while_in_if.c $OUTPUT/test_if_in_while_in_if.txt
echo ""

echo "example test_while_in_for.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_while_in_for.c $OUTPUT/test_while_in_for.txt
echo ""

echo "example nomain.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/nomain.c $OUTPUT/nomain.txt
echo ""

echo "example test_only_arithmatic_binary_ops.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_only_arithmatic_binary_ops.c $OUTPUT/test_only_arithmatic_binary_ops.txt
echo ""

echo "example p1.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/p1.c $OUTPUT/p1.txt
echo ""

echo "example p2.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/p2.c $OUTPUT/p2.txt
echo ""

echo "example test_only_asg.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_only_asg.c $OUTPUT/test_only_asg.txt
echo ""

echo "example globalvar.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/globalvar.c $OUTPUT/globalvar.txt
echo ""

echo "example test-if_relational_op.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-if_relational_op.c $OUTPUT/test-if_relational_op.txt
echo ""

echo "example test-if_true_condition.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-if_true_condition.c $OUTPUT/test-if_true_condition.txt
echo ""

echo "example test_cond_in_loop.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_cond_in_loop.c $OUTPUT/test_cond_in_loop.txt
echo ""

echo "example test_while1.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_while1.c $OUTPUT/test_while1.txt
echo ""

echo "example test_const.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_const.c $OUTPUT/test_const.txt
echo ""

#echo "example test_incompatiable_types.c"
#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_incompatiable_types.c $OUTPUT/test_incompatiable_types.txt
#echo ""

echo "example test_uo_logicalnot.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_uo_logicalnot.c $OUTPUT/test_uo_logicalnot.txt
echo ""

#echo "example test_x_eq_zero.c"
#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_x_eq_zero.c $OUTPUT/test_x_eq_zero.txt
#echo ""

echo "example test_unary.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_unary.c $OUTPUT/test_unary.txt
echo ""

echo "example test-addExpr.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-addExpr.c $OUTPUT/test-addExpr.txt
echo ""

echo "example test_initial_values.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_initial_values.c $OUTPUT/test_initial_values.txt
echo ""

echo "example test_incrdecr.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_incrdecr.c $OUTPUT/test_incrdecr.txt
echo ""

echo "example test_incr_new.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_incr_new.c $OUTPUT/test_incr_new.txt
echo ""

echo "example test_incdec.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_incdec.c $OUTPUT/test_incdec.txt
echo ""

echo "example test_incr_condition.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_incr_condition.c $OUTPUT/test_incr_condition.txt
echo ""

echo "example test-ICE.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-ICE.c $OUTPUT/test-ICE.txt
echo ""

echo "example test-CompAssign.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-CompAssign.c $OUTPUT/test-CompAssign.txt
echo ""

echo "example test_decl_with_expr_assignment.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_decl_with_expr_assignment.c $OUTPUT/test_decl_with_expr_assignment.txt
echo ""

echo "example test-paren-for-decl.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-paren-for-decl.c $OUTPUT/test-paren-for-decl.txt
echo ""

echo "example test-paren-for-assignment.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test-paren-for-assignment.c $OUTPUT/test-paren-for-assignment.txt
echo ""

echo "example test_fun_param.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_fun_param.c $OUTPUT/test_fun_param.txt
echo ""

echo "example test_uint.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_uint.c $OUTPUT/test_uint.txt
echo ""

echo "example testdata/test_only_cascaded_asgn.c - cascaded assignment"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_only_cascaded_asgn.c $OUTPUT/test_only_cascaded_asgn.txt
echo ""

echo "example testdata/test_only_opeq.c - compound assignment"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_only_opeq.c $OUTPUT/test_only_opeq.txt
echo ""

echo "example testdata/test_only_opincr.c - pre/post increment/decrement"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_only_opincr.c $OUTPUT/test_only_opincr.txt
echo ""

echo "example test_no_if.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_no_if.c $OUTPUT/test_no_if.txt
echo ""

echo "example test_only_while_comp_stmt.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_only_while_comp_stmt.c $OUTPUT/test_only_while_comp_stmt.txt
echo ""

echo "example test_while_2relop_cond.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_while_2relop_cond.c $OUTPUT/test_while_2relop_cond.txt
echo ""

echo "example testdata/dowhile.c - do-while loop"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/dowhile.c $OUTPUT/dowhile.txt
echo ""

echo "example testdata/empty_if.c - if - empty then clause and else clause"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/empty_if.c $OUTPUT/empty_if.txt
echo ""

#echo "example testdata/test_conditional_op.c - conditional operator"
#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_conditional_op.c $OUTPUT/test_conditional_op.txt
#echo ""

echo "example arr1.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/arr1.c $OUTPUT/arr1.txt
echo ""

echo "example struct1.c"
$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/struct1.c $OUTPUT/struct1.txt
echo ""

#echo "example testdata/logicalAndOr.c - logical && and ||"
#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/logicalAndOr.c $OUTPUT/logicalAndOr.txt
#echo ""

#echo "example testdata/shift_op.c - shift operator <<, >>"
#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/shift_op.c $OUTPUT/shift_op.txt
#echo ""

#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_bitwise_shift_binop.c $OUTPUT/test_bitwise_shift_binop.txt

#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/test_bitwise_logical_binop.c $OUTPUT/test_bitwise_logical_binop.txt

#echo "example testdata/shifteq_op.c - shift assign operator <<=, >>="
#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/shifteq_op.c $OUTPUT/shifteq_op.txt
#echo ""

#echo "example float-error.c"
#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/float-error.c $OUTPUT/float-error.txt
#echo ""

#echo "example double-error.c"
#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/double-error.c $OUTPUT/double-error.txt
#echo ""

#echo "example ignorePointers.c"
#$CANALYZER_HOME/bin/CAnalyzer $TESTDATA/ignorePointers.c $OUTPUT/ignorePointers.txt
#echo ""

echo -e "\n[Done]\n"

echo -e "\nErrors/Warnings:\n"
#grep -i "error" output/*; grep -i "warning" output/*; grep -i "stack" output/*; grep -i "segmentation" output/*; grep -i "Aborted" output/*
grep -i "error" $OUTPUT/*; grep -i "warning" $OUTPUT/*; grep -i "stack" $OUTPUT/*; grep -i "segmentation" $OUTPUT/*; grep -i "Aborted" $OUTPUT/*

