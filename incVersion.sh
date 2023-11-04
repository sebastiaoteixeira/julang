day=$(date +%d)
month=$(date +%m)
Year=$(date +%Y)
year=$(date +%y)


# Read version.h file ...
if [ $# -eq 0 ]; then
	file="version.h"
else
	file=$1
fi

# Change static const char DATE[] = "XX"; to static const char DATE[] = "$day";
sed -i "s/const char DATE\[\] = \"..\";/const char DATE\[\] = \"$day\";/g" $file
# Change static const char MONTH[] = "XX"; to static const char MONTH[] = "$month";
sed -i "s/const char MONTH\[\] = \"..\";/const char MONTH\[\] = \"$month\";/g" $file
# Change static const char YEAR[] = "XXXX"; to static const char YEAR[] = "$Year";
sed -i "s/const char YEAR\[\] = \"....\";/const char YEAR\[\] = \"$Year\";/g" $file
# Change static const char UBUNTU_VERSION_STYLE[] = "XX.XX"; to static const char UBUNTU_VERSION_STYLE[] = "$month.$year";
sed -i "s/const char UBUNTU_VERSION_STYLE\[\] =  \".....\";/const char UBUNTU_VERSION_STYLE\[\] =  \"$month.$year\";/g" $file

# Read REVISION from version.h file ...
revision=$(grep 'const long REVISION' $file | awk '{gsub(/;/, "", $NF); print $NF}')
# Increase REVISION by a random number between 1 and 10
new_revision=$(($revision+$(($RANDOM%10))+1))
# Change static const long REVISION = XXXXX; to static const long REVISION = $revision;
sed -i "s/static const long REVISION  = $revision;/static const long REVISION  = $new_revision;/g" $file
# Increase BUILD by 1
build=$(grep 'const long BUILD ' $file | awk '{gsub(/;/, "", $NF); print $NF}')
new_build=$(($build+1))
# if new_build % 100 == 0 then increase minor by 1
minor=$(grep 'const long MINOR' $file | awk '{gsub(/;/, "", $NF); print $NF}')
if [ $(($new_build%100)) -eq 0 ]; then
	new_minor=$(($minor+1))
else
	new_minor=$minor
fi
# Change static const long BUILD = XXXXX; to static const long BUILD = $new_build;
sed -i "s/static const long BUILD  = $build;/static const long BUILD  = $new_build;/g" $file
# Change static const long MINOR = XXXXX; to static const long MINOR = $new_minor;
sed -i "s/static const long MINOR  = $minor;/static const long MINOR  = $new_minor;/g" $file

# Read MAJOR
major=$(grep 'const long MAJOR' $file | awk '{gsub(/;/, "", $NF); print $NF}')
# Change RC_FILEVERSION $major,$minor,$build,$revision to RC_FILEVERSION $major,$new_minor,$new_build,$new_revision
sed -i "s/RC_FILEVERSION $major,$minor,$build,$revision/RC_FILEVERSION $major,$new_minor,$new_build,$new_revision/g" $file
# Change RC_FILEVERSION_STRING "major.minor.build.revision" to RC_FILEVERSION_STRING "major.new_minor.new_build.new_revision"
sed -i "s/RC_FILEVERSION_STRING \"$major, $minor, $build, $revision/RC_FILEVERSION_STRING \"$major, $new_minor, $new_build, $new_revision/g" $file
# Change static const char FULLVERSION_STRING [] = "$major.$minor.$build.$revision" to static const char FULLVERSION_STRING [] = "$major.$new_minor.$new_build.$new_revision"
sed -i "s/static const char FULLVERSION_STRING \[\] = \"$major.$minor.$build.$revision\";/static const char FULLVERSION_STRING \[\] = \"$major.$new_minor.$new_build.$new_revision\";/g" $file

