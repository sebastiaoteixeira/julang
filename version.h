#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "11";
	static const char YEAR[] = "2023";
	static const char UBUNTU_VERSION_STYLE[] =  "11.23";

	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";

	//Standard Version Type
	static const long MAJOR  = 0;
	static const long MINOR  = 4;
	static const long BUILD  = 460;
	static const long REVISION  = 2524;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 1005;
	#define RC_FILEVERSION 0,4,460,2524
	#define RC_FILEVERSION_STRING "0, 4, 460, 2524\0"
	static const char FULLVERSION_STRING [] = "0.4.460.2524";

	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 35;

#endif //VERSION_H
