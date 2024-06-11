#!/bin/bash

clear
rm -rf /tmp/llvm*

CANALYZER_HOME=`pwd`

filename=$1
ciloutput="output/cil"

cilfile=`echo $filename | cut -d"." -f1 | cut -d"/" -f2`
echo $cilfile
cildir="$ciloutput/$cilfile"
echo $cildir

mkdir -p $CANALYZER_HOME/output
mkdir -p $CANALYZER_HOME/output/cil
mkdir -p $CANALYZER_HOME/$cildir

cilly --domakeCFG --noInsertImplicitCasts --save-temps=$CANALYZER_HOME/$cildir --noPrintLn $CANALYZER_HOME/$filename

newfile="$cildir"/"$cilfile".cil.i
echo $newfile 

cat $CANALYZER_HOME/$newfile > $CANALYZER_HOME/$cildir/$cilfile.txt

#clang -cc1 -analyze -analyzer-checker=debug.ViewCFG $CANALYZER_HOME/$newfile
#dot -Tps /tmp/llvm*/CFG.dot -o $CANALYZER_HOME/$cildir/$cilfile.ps
#evince $cildir/$cilfile.ps &

$CANALYZER_HOME/bin/CAnalyzer $CANALYZER_HOME/$newfile $CANALYZER_HOME/$cildir/$cilfile.txt


