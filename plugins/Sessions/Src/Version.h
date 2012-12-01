#define __MAJOR_VERSION				0
#define __MINOR_VERSION				1
#define __RELEASE_NUM				3
#define __BUILD_NUM					0

#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_DOTS			__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)
#define __VERSION_STRING			__STRINGIFY(__FILEVERSION_DOTS)


#define __PLUGIN_NAME				"Sessions"
#define __INTERNAL_NAME				"Sessions"
#define __FILENAME					"Sessions.dll"
#define __DESCRIPTION 				"Sessions Manager for Miranda NG."
#define __AUTHOR					"MaD_CLuSTeR"
#define __AUTHOREMAIL				"daniok@yandex.ru"
#define __AUTHORWEB					"http://miranda-ng.org/"
#define __COPYRIGHT					"� 2007-2008 Danil Mozhar"