#!/bin/bash
#
# usage: cnfuzzrep.sh [-j parallelJobs|-n] [-c count] [-t timeout] [-s seed] outputDir
#

TERMINATE=1
FAIL=2

function terminate() {
  isTerminate=${TERMINATE}
}

function runMetric() {
  metric="$1"
  timeout "${timeout}" cdclsolve --input "${outputDir}/${seed}/formula.cnf" --output "${outputDir}/${seed}/cdclsolve_${metric}.cnf" --decision "${metric}"
  if [ -f "${outputDir}/${seed}/cdclsolve_${metric}.cnf" ]
  then
    if grep --quiet "${expected}" "${outputDir}/${seed}/cdclsolve_${metric}.cnf"
    then
      echo "${metric} success" >> "${outputDir}/${seed}/report.txt"
    else
      echo "${metric} fail" >> "${outputDir}/${seed}/report.txt"
      printf "." >&4
    fi
  else
    echo "${metric} timeout" >> "${outputDir}/${seed}/report.txt"
  fi
}

function runFormula() {
  mkdir --parents "${outputDir}/${seed}"
  cnfuzz ${seed} > "${outputDir}/${seed}/formula.cnf"
  local expected="^s cnf -1"
  rm --force "${outputDir}/${seed}/report.txt"
  timeout "${timeout}" lingeling -q "${outputDir}/${seed}/formula.cnf" > "${outputDir}/${seed}/lingeling.cnf"
  if [ -f "${outputDir}/${seed}/lingeling.cnf" ]
  then
    if grep --quiet '^s SATISFIABLE' "${outputDir}/${seed}/lingeling.cnf"
    then
      expected="^s cnf 1"
    fi
  else
    echo "basic reftimeout" >> "${outputDir}/${seed}/report.txt"
    echo "basic reftimeout" >> "${outputDir}/${seed}/report.txt"
    echo "basic reftimeout" >> "${outputDir}/${seed}/report.txt"
    echo "basic reftimeout" >> "${outputDir}/${seed}/report.txt"
    exit
  fi
  runMetric "basic"
  runMetric "jeroslovwang"
  runMetric "dlis"
  runMetric "vsids"
}

trap terminate SIGTERM

parallelJobs=1
timeout=0
seed=${RANDOM}
while getopts "j:nc:t:s:" option
do
  case "${option}" in
    j) parallelJobs=$OPTARG;;
    n) parallelJobs=$(nproc);;
    t) timeout=$OPTARG;;
    c) count=$OPTARG;;
    s) seed=$OPTARG;;
  esac
done
shift $((${OPTIND} - 1))
outputDir=$1
shift 2

mkfifo fifo-$$
exec 3<>fifo-$$
rm fifo-$$
for ((i=0; i<${parallelJobs}; ++i))
do
  printf "." >&3
done
mkfifo failfifo-$$
exec 4<>failfifo-$$
rm failfifo-$$

rm --recursive --force "${outputDir}"
RANDOM=${seed}
i=0
while [ -z "${isTerminate}" ] && ( [ -z "${count}" ] || ((i < count)) )
do
  read -n 1 -u 3 x
  (
    runFormula
    printf "." >&3
  )&
  i=$((++i))
  seed=${RANDOM}
  if read -n 1 -t 0 -u 4 x
  then
    isTerminate=${FAIL}
  fi
done

wait

if read -n 1 -t 0 -u 4 x
then
  isTerminate=${FAIL}
fi

if [ -n "${isTerminate}" ]
then
  exit "${isTerminate}"
fi
