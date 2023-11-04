if [ $# -eq 0 ]; then
	file="version.h"
else
	file=$1
fi

# Read version.h file ...
# Read BUILDS_COUNT
buildscount=$(grep 'const long BUILDS_COUNT  = ' $file | awk '{gsub(/;/, "", $NF); print $NF}')
# Increase BUILDS_COUNT by 1
new_buildscount=$(($buildscount+1))
# Change static const long BUILDS_COUNT  = $buildscount; to static const long BUILDS_COUNT  = $new_buildscount;
sed -i "s/static const long BUILDS_COUNT  = $buildscount;/static const long BUILDS_COUNT  = $new_buildscount;/g" $file


