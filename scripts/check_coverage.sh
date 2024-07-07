#!/bin/env sh

COVERAGE_THRESHOLD=$3

COVERAGE=$($1 --summary $2 | grep "\lines\.*" | awk '{print $2}' | tr -d '%')

COVERAGE_INT=${COVERAGE%.*}
COVERAGE_THRESHOLD_INT=${COVERAGE_THRESHOLD%.*}

if [ "$COVERAGE_INT" -lt "$COVERAGE_THRESHOLD_INT" ]; then
  echo "Coverage ($COVERAGE%) is below the threshold ($COVERAGE_THRESHOLD%)"
  exit 1
else
  echo "Coverage ($COVERAGE%) meets the threshold ($COVERAGE_THRESHOLD%)"
  exit 0
fi
