#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "17";
	static const char MONTH[] = "07";
	static const char YEAR[] = "2024";
	static const char UBUNTU_VERSION_STYLE[] =  "07.24";

	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";

	//Standard Version Type
	static const long MAJOR  = 0;
	static const long MINOR  = 5;
	static const long BUILD  = 565;
	static const long REVISION  = 3083;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 1163;
	#define RC_FILEVERSION 0,5,565,3083
	#define RC_FILEVERSION_STRING "0, 5, 565, 3083\0"
	static const char FULLVERSION_STRING [] = "0.5.565.3083";

	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 35;

#endif //VERSION_H
