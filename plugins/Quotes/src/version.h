#define __MAJOR_VERSION				0
#define __MINOR_VERSION				0
#define __RELEASE_NUM				24
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)

#define __PLUGIN_NAME				"Quotes"
#define __INTERNAL_NAME				"Quotes"
#define __FILENAME					"Quotes.dll"
#define __DESCRIPTION 				"Shows currency rates and economic quotes."
#define __AUTHOR					"Dioksin"
#define __AUTHOREMAIL				"dioksin@ua.fm"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"Don't worry!"
#define SPECIAL_BUILD_STRING		"5388"