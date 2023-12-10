#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "12";
	static const char YEAR[] = "2023";
	static const char UBUNTU_VERSION_STYLE[] =  "12.23";

	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";

	//Standard Version Type
	static const long MAJOR  = 0;
	static const long MINOR  = 5;
	static const long BUILD  = 519;
	static const long REVISION  = 2842;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 1113;
	#define RC_FILEVERSION 0,5,519,2842
	#define RC_FILEVERSION_STRING "0, 5, 519, 2842\0"
	static const char FULLVERSION_STRING [] = "0.5.519.2842";

	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 35;

#endif //VERSION_H
